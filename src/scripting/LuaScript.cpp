#include "PCH.h"

#include "scripting/LuaScript.h"
#include "os/FileManager.h"
#include "scripting/LuaState.h"

#include "global/ResourceFactory.h"

namespace cs
{
	BEGIN_META_RESOURCE(LuaScript)

	END_META()

	BEGIN_META_RESOURCE(LuaScriptFile)

	END_META()


	LuaScript::LuaScript(const std::string& fname)
		: Resource(fname)
	{

	}

	LuaScriptFile::LuaScriptFile(const std::string& fname)
		: LuaScript(fname)
	{
	
	}

	bool LuaScriptFile::load(LuaState* state, bool execute)
	{
		std::string path;
		FileManager::getInstance()->getPathToFile(this->getName(), path);
		int32 status;
		
		if (execute)
			status = luaL_loadfile(state->getState(), path.c_str());
		else
			status = luaL_dofile(state->getState(), path.c_str());

		return checkLuaError(state->getState(), status);
	}
}