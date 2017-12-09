#include "ClientHandler.h"
#include <ctime>
#include "ace/os_include/os_netdb.h"
#include "AsyncFileLogger.h"


int ClientHandler::handle_input(ACE_HANDLE)
{
	Data buf(readBufSize);
	ssize_t received = this->peer().recv(buf.data(), buf.size());

	switch (received)
	{
	case -1:
		ACE_ERROR_RETURN((LM_ERROR, "(%t) Peer has failed unexpectedly for Consumer_Handler %d\n", this->ConnectionId()), -1);
	case 0:
		ACE_ERROR_RETURN((LM_ERROR, "(%t) Peer has shutdown unexpectedly for Consumer_Handler %d\n", this->ConnectionId()), -1);
	default:
	{
		buf.resize(received);
		return consumer->Put(buf);
	}
	}
	return 0;
}


void ClientHandler::Log(const std::string& message)
{
	std::time_t result = std::time(nullptr);
	auto s = "Message from: " + inetAddrStr + " , Time: " + std::ctime(&result) + " " + message + "\n";
	AsyncFileLogger::Instance().Log(s);
}

void ClientHandler::ParseInput(const void * data, size_t size)
{
	try
	{
		parseBuf.Append(data, size);
		if (parseBuf.IsNotEmpty())
		{
			auto msg = GetMsg(parseBuf);
			if (msg.code == 'Q')
			{
				Log(msg.strInfo);
			}
		}
	}
	catch (const std::runtime_error&)
	{
		parseBuf.Clear();
	}
}


ssize_t ClientHandler::Send()
{
	auto& message = queue.front();
	ACE_DEBUG((LM_DEBUG,"(%t) sending %d bytes to Consumer %d\n", message.size(), this->ConnectionId()));

	ssize_t len = message.size();
	ssize_t n = this->peer().send(message.data(), len);
	if (isSqlLogged)
	{
		ParseInput(message.data(), len);
	}

	if (n < 0)
	{
		return n;
	}
	if (n < len)
	{
		message.resize(n);
		if (ACE_Reactor::instance()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK) == -1)
		{
			ACE_ERROR_RETURN((LM_ERROR, "(%t) %p\n", "schedule_wakeup"), -1);
		}
		return 0;
	}
	queue.pop_front();
	return n;
}

int ClientHandler::handle_output(ACE_HANDLE)
{
	ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) Receiver signalled 'resume transmission' %d\n"), this->get_handle()));

	if (ACE_Reactor::instance()->cancel_wakeup(this, ACE_Event_Handler::WRITE_MASK) == -1)
	{
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%t) %p\n"), ACE_TEXT("Error in ACE_Reactor::cancel_wakeup()")), -1);
	}

	while (!queue.empty())
	{
		ssize_t err = this->Send();
		if (err < 0)
		{
			return (int)err;
		}
	}

	return 0;
}

int ClientHandler::Put(const Data& message)
{
	queue.emplace_back(std::move(message));
	if (queue.size() < 2)
	{
		this->Send();
	}
	return 0;
}



int ClientHandler::Connect()
{
	isSqlLogged = true;

	ACE_SOCK_Connector connector;
	if (connector.connect(this->peer(), inetAddr) == -1)
	{
		ACE_ERROR_RETURN((LM_ERROR, "%p\n", "connect"), -1);
	}
	
	Log("Connect!!!!");

	return 0;
}


int ClientHandler::Init(ClientHandler * consumer, const ACE_INET_Addr & inetAddr, size_t readBufSizeMb)
{
	this->consumer = consumer;
	this->inetAddr = inetAddr;
	char buf[100];
	if (inetAddr.addr_to_string(buf, sizeof(buf)))
	{
		inetAddrStr = "Error addr";
	}
	else
	{
		inetAddrStr = buf;
	}
	readBufSize = readBufSizeMb * 1024 * 1024;
	if (this->peer().enable(ACE_NONBLOCK) == -1)
	{
		ACE_ERROR_RETURN((LM_ERROR, "(%t) %p\n", "enable"), -1);
	}
	return 0;
}

ConnectionIdType ClientHandler::ConnectionId(void) const
{
	return id;
}


int ClientHandler::handle_close(ACE_HANDLE, ACE_Reactor_Mask)
{
	if(consumer) consumer->Destroy();
	destroy();
	Log("Close!!!!");

	return 0;
}



