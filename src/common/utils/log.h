#pragma once
#include <string>
#include <vector>
#include <functional>
#include <format>

enum class LogLevel
{
	Log,
	Warn,
	Error
};

std::string LogLevelToString(LogLevel Level);

struct LogCategory
{
	std::string Name;
};


class LogManager
{
public:
	using ConsumerFunc = std::function<void(LogLevel, const LogCategory&, const std::string&)>;

	template<typename... A>
	static void Log(LogLevel Level, const LogCategory& Category, const std::string_view Format, const A&... Args)
	{
		std::string Message;

		try
		{
			Message = std::vformat(Format, std::make_format_args(Args...));
		} 
		catch (std::exception e)
		{
			Message = "Error formating error message: ";
			Message.append(Format);
			Message.append(" | ");
			Message.append(e.what());
		}

		for (ConsumerFunc& Func : Consumers)
		{
			Func(Level, Category, Message);
		}
	}
	static void AddConsumer(ConsumerFunc Consumer);

private:
	static inline std::vector<ConsumerFunc> Consumers;
};

#define FORERUNNER_CREATE_LOG_CATEGORY(Cat) static inline LogCategory LogCategory_##Cat = {.Name = #Cat};
#define FORERUNNER_LOG(Cat, Format, ...) LogManager::Log(LogLevel::Log, LogCategory_##Cat, Format, __VA_ARGS__)
#define FORERUNNER_WARN(Cat, Format, ...) LogManager::Log(LogLevel::Warn, LogCategory_##Cat, Format, __VA_ARGS__)
#define FORERUNNER_ERROR(Cat, Format, ...) LogManager::Log(LogLevel::Error, LogCategory_##Cat, Format, __VA_ARGS__)
