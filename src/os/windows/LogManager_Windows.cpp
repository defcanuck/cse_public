#include "PCH.h"

#include "os/windows/LogManager_Windows.h"

#include <Windows.h>

#include <algorithm>
#include <string>

#define MAX_LINE_LENGTH 1024

namespace cs
{

	void LogManager_Windows::print(LogType type, std::ostringstream& oss)
	{
		std::ostringstream str;
		str << kLogType[type] << " : " << oss.str() << std::endl;

		if (this->logToConsole)
		{		
			OutputDebugString(str.str().c_str());
			std::cout << str.str().c_str();
		}

		this->updateHistory(type, str.str());
	}

	LogManager* LogManager::getInstance()
	{
		if (!instance)
			instance = new LogManager_Windows();
		return instance;
	}

}