#include "AcceptHandler.h"
#include "ClientHandler.h"

#include <ace/Auto_Ptr.h>
#include <ace/INET_Addr.h>
#include <ace/Log_Msg.h>


AcceptHandler:: AcceptHandler(ACE_Reactor *reactor, const ConfigInfo& ci) :
        ACE_Event_Handler(),
		configInfo(ci),
        reactor(reactor == 0 ? ACE_Reactor::instance() : reactor),
        acceptor() 
{
    ACE_TRACE("AcceptHandler:: AcceptHandler(ACE_Reactor *)");
}

AcceptHandler::~AcceptHandler() 
{
    ACE_TRACE("AcceptHandler::~AcceptHandler()");
}

int AcceptHandler::Open()
{
	ACE_TRACE("AcceptHandler::open(void)");

	ACE_INET_Addr addr(configInfo.localPort);

	if (acceptor.open(addr, 1) == -1)
	{
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%N:%l: Failed to open ") ACE_TEXT("listening socket. (errno = %i: %m)\n"), ACE_ERRNO_GET), -1);
	}

	if (reactor->register_handler(this, ACE_Event_Handler::ACCEPT_MASK) == -1)
	{
		ACE_ERROR((LM_ERROR, ACE_TEXT("%N:%l: Failed to register accept ") ACE_TEXT("handler. (errno = %i: %m)\n"), ACE_ERRNO_GET));
		if (acceptor.close() == -1)
		{
			ACE_ERROR((LM_ERROR, ACE_TEXT("%N:%l: Failed to close the socket ") ACE_TEXT("after previous error. (errno = %i: %m)\n"), ACE_ERRNO_GET));
		}
		return -1;
	}
	return 0;
}

ACE_HANDLE AcceptHandler::get_handle() const 
{
    ACE_TRACE("AcceptHandler::get_handle(void)");
    return acceptor.get_handle();
}



int AcceptHandler::handle_input(ACE_HANDLE)
{
    ACE_TRACE("AcceptHandler::handle_input(ACE_HANDLE)");

    ACE_INET_Addr clientAddr;

    ClientHandler *clientHandler = 0;
    ACE_NEW_NORETURN (clientHandler, ClientHandler());
	ClientHandler *connectHandler = 0;
	ACE_NEW_NORETURN(connectHandler, ClientHandler());
	if (clientHandler == 0 || connectHandler == 0)
	{
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%N:%l: Failed to allocate ") ACE_TEXT("reader. (errno = %i: %m)\n"), ACE_ERRNO_GET), -1);
	}

    auto_ptr<ClientHandler> clientDeleter(clientHandler);
	auto_ptr<ClientHandler> connectDeleter(clientHandler);

	if (acceptor.accept(clientHandler->peer(), &clientAddr) == -1)
	{
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%N:%l: Failed to accept ") ACE_TEXT("client connection. (errno = %i: %m)\n"), ACE_ERRNO_GET), -1);
	}

	ACE_INET_Addr remoteAddr(configInfo.remotePort, configInfo.remoteHost.c_str());
	connectHandler->Init(clientHandler, remoteAddr, configInfo.maxReadBufSize);
	clientHandler->Init(connectHandler, clientAddr, configInfo.maxReadBufSize);
	connectHandler->Connect();

	if (reactor->register_handler(clientHandler, ACE_Event_Handler::READ_MASK) == -1)
	{
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%N:%l: Failed to register ") ACE_TEXT("read handler. (errno = %i: %m)\n"), ACE_ERRNO_GET), -1);
	}
	if (reactor->register_handler(connectHandler, ACE_Event_Handler::READ_MASK) == -1)
	{
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%N:%l: Failed to register ") ACE_TEXT("read handler. (errno = %i: %m)\n"), ACE_ERRNO_GET), -1);
	}

    clientDeleter.release();
	connectDeleter.release();

    return 0;
}

int AcceptHandler::handle_close(ACE_HANDLE, ACE_Reactor_Mask) 
{
    ACE_TRACE("AcceptHandler::handle_close(ACE_HANDLE, ACE_Reactor_Mask)");

	if (acceptor.close() == -1)
	{
		ACE_ERROR((LM_ERROR, ACE_TEXT("%N:%l: Failed to close the ") ACE_TEXT("socket. (errno = %i: %m)\n"), ACE_ERRNO_GET));
	}
	return 0;
}

