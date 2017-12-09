#include <ace/Event_Handler.h>
#include <ace/Log_Msg.h>
#include <ace/Reactor.h>
#include <ace/Signal.h>
#include <ace/streams.h>
#include <ace/Thread_Manager.h>
#include <ace/TP_Reactor.h>
#include "ace/Get_Opt.h"

#include "AcceptHandler.h"
#include "ProtocolParser.h"
#include "AsyncFileLogger.h"

ACE_THR_FUNC_RETURN threadFunc(void *arg) 
{
    ACE_Reactor *reactor = (ACE_Reactor *) arg;
	return reactor->run_reactor_event_loop();
}




static int ParseProgramOptions(int argc, ACE_TCHAR *argv[], ConfigInfo& configInfo)
{
	ACE_Get_Opt cmd_opts(argc, argv);

	cmd_opts.long_option(ACE_TEXT("host"), 'h', ACE_Get_Opt::ARG_REQUIRED);
	cmd_opts.long_option(ACE_TEXT("remote"), 'r', ACE_Get_Opt::ARG_REQUIRED);
	cmd_opts.long_option(ACE_TEXT("local"), 'l', ACE_Get_Opt::ARG_REQUIRED);
	cmd_opts.long_option(ACE_TEXT("size"), 's', ACE_Get_Opt::ARG_REQUIRED);
	cmd_opts.long_option(ACE_TEXT("file"), 'f', ACE_Get_Opt::ARG_REQUIRED);

	int option;
	while ((option = cmd_opts()) != EOF)
	{
		switch (option)
		{
		case 'f':
			configInfo.logFile = cmd_opts.opt_arg();
			break;
		case 'h':
			configInfo.remoteHost = cmd_opts.opt_arg();
			break;
		case 'r':
			configInfo.remotePort = (uint16_t)ConvertStrToUlong(cmd_opts.opt_arg(), ConfigInfo::DEFAULT_PG_SQL_PORT);
			break;
		case 'l':
			configInfo.localPort = (uint16_t)ConvertStrToUlong(cmd_opts.opt_arg(), ConfigInfo::DEFAULT_LOCAL_PORT);
			break;
		case 's':
			configInfo.maxReadBufSize = ConvertStrToUlong(cmd_opts.opt_arg(), ConfigInfo::READ_BUF_SIZE_DEFAULT_MB);
			break;
		case ':':
			ACE_ERROR_RETURN ((LM_ERROR, ACE_TEXT("-%c requires an argument\n"), cmd_opts.opt_opt()), -1);
		default:
			ACE_ERROR_RETURN ((LM_ERROR, ACE_TEXT("Parse error.\n")), -1);
		}
	}
	return 0;
}

int ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
	ConfigInfo configInfo;

	int err = ParseProgramOptions(argc, argv, configInfo);
	if (err)
	{
		return err;
	}

	AsyncFileLogger::SetFileName(configInfo.logFile);


	ACE_TP_Reactor tpReactor;
	ACE_Reactor reactor(&tpReactor);

	AcceptHandler *acceptHandler = 0;

	ACE_NEW_NORETURN(acceptHandler, AcceptHandler(&reactor, configInfo));
	if (acceptHandler == 0)
	{
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%N:%l: Failed to allocate ") ACE_TEXT("accept handler. (errno = %i: %m)\n"), ACE_ERRNO_GET), -1);
	}

	if (acceptHandler->Open() == -1)
	{
		delete acceptHandler;
		ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%N:%l: Failed to open accept ") ACE_TEXT("handler. Exiting.\n")), -1);
	}

	ACE_Thread_Manager::instance()->spawn_n(9, threadFunc, &reactor);

	ACE_Thread_Manager::instance()->wait();

	ACE_DEBUG((LM_DEBUG, ACE_TEXT("Bye. Bye.\n")));
	return 0;
}


