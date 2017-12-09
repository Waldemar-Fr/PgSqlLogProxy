#include "AsyncFileLogger.h"
#include <fstream>
#include <string>

std::string AsyncFileLogger::fileName = "out.txt";

AsyncFileLogger::AsyncFileLogger()
	: thr(ThreadFunction, std::ref(*this))
{
}

AsyncFileLogger::~AsyncFileLogger()
{
	needEnd = true;
	thr.join();
}

void AsyncFileLogger::Log(const std::string & str)
{
	Lock lock(mut);
	queue.push_back(str);
}

void AsyncFileLogger::SetFileName(const std::string & fn)
{
	fileName = fn;
}

AsyncFileLogger & AsyncFileLogger::Instance()
{
	static AsyncFileLogger logger;
	return logger;
}

void AsyncFileLogger::ThreadFunction(AsyncFileLogger & logger)
{
	std::fstream fs;
	fs.open(logger.fileName, std::fstream::in | std::fstream::out | std::fstream::app);

	while (!logger.needEnd)
	{
		auto& q = logger.queue;
		while (!q.empty())
		{
			Lock lock(logger.mut);
			fs << q.front();
			q.pop_front();
		}
		fs.flush();
	}
}


