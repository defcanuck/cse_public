#include "PCH.h"

#include "game/GameInterface.h"

#include "os/FileManager.h"

namespace cs
{

	GameInterface::GameInterface()
	{
		this->context = nullptr;
	}

	GameInterface::~GameInterface()
	{

	}

	void GameInterface::setContext(ContextPtr& cxt)
	{
		this->context = cxt;
	}
	
	ContextPtr& GameInterface::getContext()
	{
		return this->context;
	}
	
	bool GameInterface::init(LuaBindInitFunctions& luaFuncs)
	{
		FileManager::getInstance()->refreshFromBasePath();

		this->context->refresh();
		
		return true;
	}

	UIStackPtr& GameInterface::getUI() 
	{ 
		return this->context->getUI(); 
	}

	ScenePtr& GameInterface::getScene()
	{
		return this->context->getScene();
	}

	bool GameInterface::loadScene(const std::string& fileName)
	{
		if (!this->context)
		{
			log_error("No Context Defined!");
			return false;
		}

		bool success = this->context->loadScene(fileName);
		if (success)
		{
			this->context->getScene()->reset();
		}
		return success;
	}

	GameInterfacePtr GameInterface::singleton = nullptr;
	
}