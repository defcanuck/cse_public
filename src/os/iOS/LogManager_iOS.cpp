#include "PCH.h"

#include "os/iOS/LogManager_iOS.h"

#include <algorithm>
#include <string>

#define MAX_LINE_LENGTH 1024

namespace cs
{

	void LogManager_iOS::print(LogType type, std::ostringstream& oss)
	{
        
#if defined(DEBUG) || defined(CS_EDITOR)
		std::ostringstream str;
		str << kLogType[type] << " : " << oss.str() << std::endl;

		if (this->logToConsole)
		{		
			std::cout << str.str().c_str();
		}

		this->updateHistory(type, str.str());
#endif
        
    }

	LogManager* LogManager::getInstance()
	{
		if (!instance)
			instance = new LogManager_iOS();
		return instance;
	}

}
