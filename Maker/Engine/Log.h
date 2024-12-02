#pragma once
#include <vector>
#include <string>

class Log
{
protected:
	Log() = default;
public:
	static Log& getInstance()
	{
		static Log instance;
		return instance;
	}
	std::vector<std::string> logMessages;
	void logMessage(const std::string& message)
	{
		logMessages.push_back(message);
	}
	Log(const Log&) = delete;
	Log(Log&&) = delete;
	Log& operator=(const Log&) = delete;
	Log& operator=(Log&&) = delete;

};