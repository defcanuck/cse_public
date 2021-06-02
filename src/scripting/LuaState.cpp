#include "PCH.h"

#include "scripting/LuaState.h"
#include "scripting/LuaBindings.h"
#include "os/LogManager.h"
#include "os/FileManager.h"

namespace cs
{
	int32 gLastLuaError = 0;

	const LuaState::LuaGlobals LuaState::kLuaGlobals = { "Global.lua" };

	const char* kLuaTypeStr[] =
	{
		"nil",		// LUA_TNIL
		"bool",		// LUA_TBOOLEAN
		"userdata",	// LUA_TLIGHTUSERDATA
		"number",	// LUA_TNUMBER
		"string",	// LUA_TSTRING
		"table",	// LUA_TTABLE
		"function",	// LUA_TFUNCTION
		"userdata",	// LUA_TUSERDATA
		"thread"	// LUA_TTHREAD
	};

	bool checkLuaError(lua_State* state, int status)
	{
		if (status != 0) 
		{
			log::print(LogError, "Lua:", lua_tostring(state, -1));
			lua_pop(state, 1); // remove error message
			return false;
		}
		return true;
	}

	int setLuaPath(lua_State* L, const char* path)
	{
		lua_getglobal(L, "package");

		// get field "path" from table at top of stack (-1)
		lua_getfield(L, -1, "path"); 
		// grab path string from top of stack

		std::string cur_path = lua_tostring(L, -1); 
		cur_path.append(";"); // do your path magic here
		cur_path.append(path);

		// get rid of the string on the stack we just pushed on line 5
		lua_pop(L, 1); 

		// push the new one
		lua_pushstring(L, cur_path.c_str()); 
		lua_setfield(L, -2, "path"); 

		// set the field "path" in table at -2 with value at top of stack
		// get rid of package table from top of stack
		lua_pop(L, 1); 
		return 0;
	}

	LuaState::LuaState(const std::string& luaAbsolutePath, const LuaGlobals& globals, LuaStateParams params)
	{
		this->state = luaL_newstate();

		luaL_requiref(this->state,"io", luaopen_io, 1); // provides io.*
		luaL_requiref(this->state, "base", luaopen_base, 1);
		luaL_requiref(this->state, "table", luaopen_table, 1);
		luaL_requiref(this->state, "string", luaopen_string, 1);
		luaL_requiref(this->state, "math", luaopen_math, 1);
		luaL_requiref(this->state, "package", luaopen_package, 1);
		luaL_requiref(this->state, "debug", luaopen_debug, 1);
		luaL_requiref(this->state, "os", luaopen_os, 1);

		initBindings(this->state);
		initGlobals(this->state, &params);

		setLuaPath(this->state, luaAbsolutePath.c_str());

		for (auto it : params.initFuncs)
		{
			it.func(this->state);
		}

		bool ret = true;
		for (auto it : globals)
		{
			if (!this->loadFile(it))
			{
				log::error("Failed to load ", it, " into global Lua source");
			}
		}

		// luaopen_loadlib(this->state);
	}

	LuaState::~LuaState()
	{
		log::info("Closing Lua State");
		lua_close(this->state);
	}
	
	bool LuaState::loadScript(LuaScriptPtr& script)
	{
		LuaScriptPtr ptr;
		if (this->findScript(script->getName(), ptr))
		{
			return true;
		}

		if (script->load(this))
		{
			this->scripts[script->getName()] = script;
			return true;
		}

		return false;
	}

	bool LuaState::loadFile(const std::string& fileName, bool execute)
	{
		LuaScriptPtr ptr;
		if (this->findScript(fileName, ptr))
		{
			log::print(LogError, "Duplicate Script in State");
			return false;
		}
			
		LuaScriptFilePtr scriptFile = CREATE_CLASS(LuaScriptFile, fileName);
		if (scriptFile->load(this, execute))
		{
			this->scripts[fileName] = scriptFile;
			return true;
		}
			
		return false;
	}

	bool LuaState::findScript(const std::string& name, LuaScriptPtr& ptr)
	{
		LuaScriptMap::iterator it = this->scripts.find(name);
		if (it != scripts.end())
		{
			ptr = it->second;
			return true;
		}
		return false;
	}

	bool LuaState::run()
	{
		int32 ret = lua_pcall(this->state, 0, LUA_MULTRET, 0);
		return checkLuaError(this->state, ret);
	}

	void LuaState::checkError()
	{
		checkLuaError(this->state, -1);
	}

	bool LuaState::runString(const std::string& str)
	{
		int32 ret = luaL_dostring(this->state, str.c_str());
		return checkLuaError(this->state, ret);
	}
}
