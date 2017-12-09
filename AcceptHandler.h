#pragma once

#include <ace/Event_Handler.h>
#include <ace/Reactor.h>
#include <ace/SOCK_Acceptor.h>
#include "ClientHandler.h"

struct ConfigInfo
{
	enum
	{
		DEFAULT_PG_SQL_PORT = 5432,
		DEFAULT_LOCAL_PORT = 4712,
		READ_BUF_SIZE_DEFAULT_MB = 10
	};
	size_t maxReadBufSize = READ_BUF_SIZE_DEFAULT_MB;
	std::string remoteHost = "localhost";
	uint16_t remotePort = ConfigInfo::DEFAULT_PG_SQL_PORT;
	uint16_t localPort = ConfigInfo::DEFAULT_LOCAL_PORT;
	std::string logFile = "out.log";
};


class AcceptHandler : public ACE_Event_Handler {

public:

	AcceptHandler(ACE_Reactor *reactor, const ConfigInfo& configInfo);

	virtual ~AcceptHandler();

	int Open(void);

	virtual ACE_HANDLE get_handle() const;

	virtual int handle_input(ACE_HANDLE = ACE_INVALID_HANDLE);
	virtual int handle_close(ACE_HANDLE, ACE_Reactor_Mask);

private:
	ACE_Reactor *reactor;
	ACE_SOCK_Acceptor acceptor;
	ConfigInfo configInfo;

};


