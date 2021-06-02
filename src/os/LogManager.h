#pragma once

#include <iostream>
#include <sstream>
#include <deque>
#include <vector>

#include "global/Values.h"

#define logVerbose(type, ...) \
	cs::log::print(type, __FILE__, " - ", __LINE__, " : ", __VA_ARGS__)

#define log_error(...) \
	cs::log::error(__FILE__, " - ", __LINE__, " : ", __VA_ARGS__)

#define log_debug(...) \
	cs::log::debug(__FILE__, " - ", __LINE__, " : ", __VA_ARGS__)

#define log_info(...) \
	cs::log::info(__FILE__, " - ", __LINE__, " : ", __VA_ARGS__)

#define log_warning(...) \
	cs::log::warning(__FILE__, " - ", __LINE__, " : ", __VA_ARGS__)

namespace cs
{

	enum LogType
	{
		LogNone = -1,
		LogLua,
		LogInfo,
		LogDebug,
		LogWarning,
		LogError,
		LogMAX
	};

	extern const char* kLogType[];

	struct LogEntry
	{
		LogEntry()
			: type(LogNone)
			, message("") { }
		LogEntry(const LogEntry& rhs)
			: type(rhs.type)
			, message(rhs.message) { }

		void operator=(const LogEntry& rhs)
		{
			this->type = rhs.type;
			this->message = rhs.message;
		}

		LogType type;
		std::string message;
	};

	class LogManager
	{
	public:
		LogManager();
		~LogManager();

		virtual void print(LogType type, std::ostringstream& oss) = 0;

		static LogManager* getInstance();

		const int32 kMaxLogMessages = 100;

		void flush(std::vector<LogEntry>& messages);
		bool isDirty() const { return this->dirty; }

		void setLogToConsole(bool log) { this->logToConsole = log; }

	protected:

		void updateHistory(LogType type, const std::string message);
		
		bool logToConsole;

	private:

		bool dirty;
		std::deque<LogEntry> logHistory;
		static LogManager* instance;
	};

	namespace log
	{

		template <typename T>
		void RecursiveLog(std::ostringstream& o, T t)
		{
			o << t;
		}

		template<typename T, typename... Args>
		void RecursiveLog(std::ostringstream& o, T t, Args... args) // recursive variadic function
		{
			RecursiveLog(o, t);
			RecursiveLog(o, args...);
		}

		template<typename... Args>
		void print(LogType logType, Args... args)
		{
			std::ostringstream oss;
			RecursiveLog(oss, args...);
			LogManager::getInstance()->print(logType, oss);
		}

		template<typename... Args>
		void warning(Args... args)
		{
			print(LogWarning, args...);
		}

		template<typename... Args>
		void info(Args... args)
		{
			print(LogInfo, args...);
		}

		template<typename... Args>
		void error(Args... args)
		{
			print(LogError, args...);
		}

		template<typename... Args>
		void debug(Args... args)
		{
			print(LogDebug, args...);
		}
	}
}
