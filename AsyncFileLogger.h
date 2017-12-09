#pragma once

#include <thread>
#include <mutex>
#include <deque>


class AsyncFileLogger
{
	using Queue = std::deque<std::string>;
	using Lock = std::lock_guard<std::mutex>;
public:
	AsyncFileLogger();
	~AsyncFileLogger();
	void Log(const std::string& str);

	static void SetFileName(const std::string& fn);
	static AsyncFileLogger& Instance();

private:
	Queue queue;
	std::mutex mut;
	std::thread thr;
	bool needEnd = false;
	static std::string fileName;

	static void ThreadFunction(AsyncFileLogger &logger);
};


