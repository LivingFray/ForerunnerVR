#include "log.h"
#include <string>

void LogManager::AddConsumer(ConsumerFunc Consumer)
{
	Consumers.push_back(Consumer);
}

std::string LogLevelToString(LogLevel Level)
{
	switch (Level)
	{
		case LogLevel::Log:
			return "LOG";
		case LogLevel::Warn:
			return "WARN";
		case LogLevel::Error:
			return "ERR";
		default:
			return "UNK";
	}
}
