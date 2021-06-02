#pragma once

#include "ClassDef.h"

#include "ecs/system/BaseSystem.h"
#include "scripting/LuaState.h"

namespace cs
{

	class ScriptSystem : public ECSContextSystemBase<ScriptSystem, ECSScript>, public BaseSystem
	{
	public:
		ScriptSystem(ECSContext* cxt);
		virtual ~ScriptSystem();

		bool init(LuaStatePtr& state);
		virtual void processImpl(SystemUpdateParams* params);
		
		void setLuaState(LuaStatePtr& ptr) { this->luaState = ptr; }

	private:

		LuaStatePtr luaState;
	};
}