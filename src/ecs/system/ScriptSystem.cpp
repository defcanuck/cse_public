#include "PCH.h"

#include "ecs/system/ScriptSystem.h"
#include "ecs/comp/ScriptComponent.h"
#include "ecs/comp/CollisionComponent.h"
#include "ecs/ECS_Utils.h"

namespace cs
{
	ScriptSystem::ScriptSystem(ECSContext* cxt)
		: BaseSystem(cxt)
		, luaState(nullptr)
	{
		this->subscribeForComponent<ScriptComponent>(this->parentContext);
		this->subscribeForComponent<CollisionComponent>(this->parentContext);
	}

	ScriptSystem::~ScriptSystem()
	{
        this->removeComponentSubscription<ScriptComponent>(this->parentContext);
		this->removeComponentSubscription<CollisionComponent>(this->parentContext);
	}

	bool ScriptSystem::init(LuaStatePtr& state)
	{
		bool ret = true;
		BaseSystem::ComponentIdMap scripts;
		this->getEnabledComponents<ScriptComponent>(scripts);
		for (const auto it : scripts)
		{
			const ScriptComponentPtr& script =
				std::static_pointer_cast<ScriptComponent>(it.second);

			ret = ret && script->load(state);
		}

		scripts.clear();
		this->getEnabledComponents<CollisionComponent>(scripts);
		for (const auto it : scripts)
		{
			const CollisionComponentPtr& script =
				std::static_pointer_cast<CollisionComponent>(it.second);

			ret = ret && script->load(state);
		}

		return ret;
	}

	void ScriptSystem::processImpl(SystemUpdateParams* params)
	{
		BaseSystem::ComponentIdMap scripts;
		this->getEnabledComponents<ScriptComponent>(scripts);
		for (const auto it : scripts)
		{
			const ScriptComponentPtr& script =
				std::static_pointer_cast<ScriptComponent>(it.second);

			script->process(params->updateDt);
		}
	}
}
