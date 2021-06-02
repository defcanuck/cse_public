#pragma once

#include "os/LogManager.h"

namespace cs
{
	class LogManager_iOS : public LogManager
	{
	public:

		virtual void print(LogType type, std::ostringstream& oss);
	
	};
}
