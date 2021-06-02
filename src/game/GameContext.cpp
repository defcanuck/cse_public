#include "PCH.h"

#include "game/GameContext.h"
#include "os/FileManager.h"

namespace cs
{
	bool GameContext::loadScene(const std::string& fileName)
	{
		std::string fullPath;
		if (!FileManager::getInstance()->getPathToFile(fileName, fullPath))
		{
			log_error("Cannot find file: ", fileName);
			return false;
		}

		SerializableHandle<Scene> handle(this->mainScene);
		if (handle.exists())
			handle->clearBehaviorStates();

		if (handle.load(fullPath))
		{
			handle->setup(this->luaState);
			handle->clearBehaviorStates();
			return true;
		}
		
		return false;
	}
}