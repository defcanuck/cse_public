#pragma once

#include "ClassDef.h"

#include "global/Resource.h"

namespace cs
{
	class LuaState;
	
	CLASS_DEFINITION_DERIVED_REFLECT(LuaScript, Resource)
	public:
		LuaScript(const std::string& fname);
		virtual bool load(LuaState* state, bool execute = false) { return false; }

	private:

	};
	
	CLASS_DEFINITION_DERIVED_REFLECT(LuaScriptFile, LuaScript)
	public:
		LuaScriptFile(const std::string& fname);

		virtual bool load(LuaState* state, bool execute = false);

	private:

	};
}