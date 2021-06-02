#pragma once

struct lua_State;

namespace cs
{
	struct LuaStateParams;

	void initBindings(lua_State* state);
	void initGlobals(lua_State* state, LuaStateParams* params);
}