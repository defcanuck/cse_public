#include "PCH.h"

#include "scene/behaviors/ScriptInputBehavior.h"
#include "scene/behaviors/SelectionBehavior.h"
#include "scene/SceneData.h"
#include "scene/Scene.h"
#include "ecs/comp/ScriptComponent.h"
#include "ecs/system/ScriptSystem.h"

namespace cs
{
	ScriptInputBehavior::ScriptInputBehavior() 
		: pressedEntity(nullptr)
	{ 
	
	}

	ScriptInputBehavior::~ScriptInputBehavior() 
	{ 
	
	}

	bool ScriptInputBehavior::onInputMove(Scene* scene, const BehaviorData& data)
	{
		return false;
	}

	bool ScriptInputBehavior::onInputPressed(Scene* scene, const BehaviorData& data)
	{
		vec3 hit_pos;
		BaseSystem::ComponentIdMap script_components;
		ScriptSystem::getInstance()->getEnabledComponents<ScriptComponent>(script_components);

		for (auto it : script_components)
		{
			EntityIntersection hit_data;
			ScriptComponentPtr script_component = std::static_pointer_cast<ScriptComponent>(it.second);
			Entity* entity = script_component->getParent();

			bool intersects = SceneData::intersects(entity, data.ray, hit_data);
			if (intersects)
			{
				this->pressedEntity = it.second->getParent();
				script_component->onPress(data.screenPos);
				return true;
			}
		}

		return false;

	}

	bool ScriptInputBehavior::onInputReleased(Scene* scene, const BehaviorData& data)
	{
		if (this->pressedEntity)
		{
			ScriptComponentPtr scriptComponent = this->pressedEntity->getComponent<ScriptComponent>();
			if (!scriptComponent)
				return false;

			scriptComponent->onRelease(data.screenPos);
			this->pressedEntity = nullptr;
			return true;
		}
		return false;
	}

}