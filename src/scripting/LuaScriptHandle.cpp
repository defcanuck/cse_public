#include "PCH.h"

#include "scripting/LuaScriptHandle.h"

#include "global/ResourceFactory.h"

namespace cs
{
	BEGIN_META_CLASS(LuaScriptHandle)
		ADD_MEMBER_RESOURCE(reference);
			SET_MEMBER_CALLBACK_POST(&LuaScriptHandle::onReferenceChanged);
	END_META()

	LuaScriptHandle::LuaScriptHandle(const std::string& fileName)
	{
		this->reference = std::static_pointer_cast<LuaScriptFile>(
			ResourceFactory::getInstance()->loadResource<LuaScriptFile>(fileName));
	}

	void LuaScriptHandle::onReferenceChanged()
	{
		this->onChanged.invoke();
	}

	bool LuaScriptHandle::load(LuaStatePtr& state)
	{
		if (!this->reference)
		{
			log::error("No Lua Script Resource bound to Handle!");
			return false;
		}

		if (!state)
		{
			log::error("Empty state cannot load script!");
			return false;
		}

		LuaScriptPtr script = std::static_pointer_cast<LuaScript>(this->reference);
		return state->loadScript(script);
	}
}