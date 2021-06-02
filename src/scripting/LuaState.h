#pragma once

#include "ClassDef.h"

extern "C" 
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

#include "scripting/LuaScript.h"
#include "scripting/LuaBindings.h"

#define _LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR

#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>

#include <unordered_map>

#define CALL_LUA_FUNCTION_SINGLE(function_name) \
try \
{ \
    call<void>(function_name); \
} \
catch (luabind::error& err) \
{ \
    checkLuaError(err.state(), -1); \
}

#define CALL_LUA_FUNCTION(function_name, ...) \
try \
{ \
    call<void>(function_name, __VA_ARGS__); \
} \
catch (luabind::error& err) \
{ \
	checkLuaError(err.state(), -1); \
}

#define CALL_LUA_FUNCTION_RET(function_name, return_value, ...) \
	call<return_value>(function_name, __VA_ARGS__); 

namespace cs
{
	typedef void(*luaBindFunc)(lua_State*);
	struct LuaBindInitFunc
	{
		luaBindFunc func;
	};
	typedef std::vector<LuaBindInitFunc> LuaBindInitFunctions;

	bool checkLuaError(lua_State* state, int32 status);

	extern const char* kLuaTypeStr[];

	struct LuaStateParams
	{
		LuaStateParams()
			: isEditor(false) { }

		std::string entryPoint;
		std::string path;

		bool isEditor;

		LuaBindInitFunctions initFuncs;
	};

	CLASS_DEFINITION(LuaState)

	public:
	
		typedef std::vector<std::string> LuaGlobals;
		static const LuaGlobals kLuaGlobals;

		LuaState(const std::string& luaAbsolutePath, const LuaGlobals& globals = LuaGlobals(), LuaStateParams params = LuaStateParams());
		~LuaState();

		bool loadFile(const std::string& fileName, bool execute = false);
		bool loadScript(LuaScriptPtr& script);
		bool loadString(const std::string& str);

		bool run();
		bool runString(const std::string& command);
		
		inline lua_State* getState() 
		{
			return this->state;
		}
		
		void checkError();

		template <typename... Args>
		bool callFunction(const std::string funcName, Args...args)
		{
			if (!this->state)
			{
				log::error("No lua state!");
				return false;
			}

			bool ret = true;
			try
			{
				luabind::call_function<void>(this->state, funcName.c_str(), args...);
			}
			catch (luabind::error& err)
			{
				ret = checkLuaError(err.state(), -1);
			}
			return ret;
		}

		template <typename...Args>
		luabind::object createObject(const std::string& className, Args...args)
		{
			return luabind::globals(state)[className](args...);
		}

	private:

		bool findScript(const std::string& name, LuaScriptPtr& ptr);

		friend class LuaScript;

		lua_State* state;

		typedef std::unordered_map<std::string, LuaScriptPtr> LuaScriptMap;
		LuaScriptMap scripts;

	};

	
}
