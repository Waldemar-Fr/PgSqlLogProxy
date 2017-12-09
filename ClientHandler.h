#pragma once

#include "ace/Service_Config.h"

#include "ace/Acceptor.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/SOCK_Connector.h"
#include "ace/Svc_Handler.h"
#include "ace/Connector.h"
#include "ace/Null_Condition.h"
#include "ace/Null_Mutex.h"
#include "ace/svc_export.h"
#include <queue>
#include "ProtocolParser.h"


#if defined ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION_EXPORT
template class ACE_Svc_Export ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>;
#endif /* ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION_EXPORT */


typedef uint32_t ConnectionIdType;


class ClientHandler : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_SYNCH>
{
	using Data = std::vector<uint8_t>;
	using Queue = std::deque<Data>;
public:
	int id;

	int Put(const Data& event);
	int Connect();
	int Init(ClientHandler* consumer, const ACE_INET_Addr& inetAddr, size_t readBufSizeMb);

protected:
	ConnectionIdType ConnectionId(void) const;

	void Destroy()
	{
		consumer = 0;
		destroy();
	}

	virtual int handle_input(ACE_HANDLE = 0);
	void Log(const std::string & message);
	virtual int handle_output(ACE_HANDLE);
	virtual int handle_close(ACE_HANDLE = ACE_INVALID_HANDLE, ACE_Reactor_Mask = ACE_Event_Handler::ALL_EVENTS_MASK);

private:
	void ParseInput(const void * data, size_t size);
	ssize_t Send();

	PgSqlParse::InputBuffer parseBuf;
	Queue queue;
	bool isSqlLogged = false;
	ClientHandler* consumer;
	ACE_INET_Addr inetAddr;
	std::string inetAddrStr;
	size_t readBufSize;
};



