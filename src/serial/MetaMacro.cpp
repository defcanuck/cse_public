#include "PCH.h"

#include "serial/MetaMacro.h"
#include "os/LogManager.h"

namespace cs
{
	const static bool kMetaLogDeletion = false;
	const static bool kMetaLogCreation = false;

	void logMetaCreation(const char* msg)
	{
		if (kMetaLogCreation)
		{
			log::print(LogInfo, "Creating: ", msg);
		}
	}

	void logMetaDeletion(const char* msg)
	{
		if (kMetaLogDeletion)
		{
			log::print(LogInfo, "Deleting: ", msg);
		}
	}
	
}