#include "PCH.h"

#include "scene/behaviors/GameInputBehavior.h"
#include "scene/SceneData.h"
#include "scene/Scene.h"

#include "ecs/comp/GameComponent.h"
#include "ecs/system/GameSystem.h"

namespace cs
{
	GameInputBehavior::GameInputBehavior()
	{

	}

	GameInputBehavior::~GameInputBehavior()
	{

	}

	bool GameInputBehavior::onInputMove(Scene* scene, const BehaviorData& data)
	{
		if (data.input == ClickNone || !this->pressedEntity)
		{
			return false;
		}
		
		
		log::info("Move ", this->pressedEntity->getName());
		GameComponentPtr game_component = this->pressedEntity->getComponent<GameComponent>();
		if (!game_component)
			return false;

		GameSelectableBehaviorPtr& behavior = game_component->getBehavior();
		if (behavior.get() && 
			behavior->onMove.get())
			return behavior->onMove->call<bool, vec2, ClickInput, Entity*>(data.screenPos, data.input, this->pressedEntity);

		return false;
	}

	bool GameInputBehavior::onInputPressed(Scene* scene, const BehaviorData& data)
	{
		if (data.input == ClickNone)
		{
			return false;
		}
		
		vec3 hit_pos;
		BaseSystem::ComponentIdMap game_components;
		GameSystem::getInstance()->getEnabledComponents<GameComponent>(game_components);
		for (auto it : game_components)
		{
			EntityIntersection hit_data;
			GameComponentPtr game_component = std::static_pointer_cast<GameComponent>(it.second);
			if (!game_component->isSelectable())
				continue;

			const GameSelectableBehaviorPtr& behavior = game_component->getBehavior();
			
			Entity* entity = game_component->getParent();

			Ray transRay = data.ray;
			mat4 transMat = entity->getWorldTransform().getCurrentMatrix();
			transRay.transform(glm::inverse(transMat));

			vec3 hit_pos = kZero3;
			if (game_component->getInputVolume()->intersects(transRay, hit_pos))
			{
				GameSelectableBehaviorPtr& behavior = game_component->getBehavior();
				if (behavior.get() && 
					behavior->onPressed.get() &&
					behavior->onPressed->call<bool, vec2, ClickInput, Entity*>(data.screenPos, data.input, entity))
				{
					log::info("Pressed ", entity->getName());
					this->pressedEntity = entity;
					return true;
				}
			}
		}

		return false;
	}

	bool GameInputBehavior::onInputUpImpl(Scene* scene, const BehaviorData& data)
	{
		if (data.input == ClickNone || !this->pressedEntity)
		{
			return false;
		}

		GameComponentPtr game_component = this->pressedEntity->getComponent<GameComponent>();
		if (!game_component)
			return false;

		GameSelectableBehaviorPtr& behavior = game_component->getBehavior();
			
		Entity* entity = game_component->getParent();
		if (behavior.get() && 
			behavior->onReleased.get() &&
			behavior->onReleased->call<bool, vec2, ClickInput, Entity*>(data.screenPos, data.input, entity))
		{
			log::info("Released ", this->pressedEntity->getName());
			// game_component->onRelease(data.screenPos);
			this->pressedEntity = nullptr;
			return true;
		}
		
		return false;
	}

}