#include "PCH.h"

#include "os/LogManager.h"

namespace cs
{
	LogManager* LogManager::instance = nullptr;

	const char* kLogType[] =
	{
		"Lua",
		"Info",
		"Debug",
		"Warning",
		"Error"
	};

	LogManager::LogManager()
		: dirty(false)
		, logToConsole(true)
	{
	
	}

	LogManager::~LogManager()
	{

	}

	void LogManager::updateHistory(LogType type, const std::string message)
	{
		LogEntry entry;
		entry.type = type;
		entry.message = message;
		this->logHistory.push_front(entry);

		while (this->logHistory.size() > kMaxLogMessages)
		{
			this->logHistory.pop_back();
		}
		this->dirty = true;
	}

	void LogManager::flush(std::vector<LogEntry>& messages)
	{
		for (auto it : this->logHistory)
			messages.push_back(it);
		this->dirty = false;
	}
}