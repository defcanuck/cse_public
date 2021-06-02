#pragma once

#include "scene/behaviors/Behavior.h"
#include "scene/Actor.h"

namespace cs
{

	CLASS_DEFINITION_DERIVED(GameInputBehavior, Behavior)

	public:

		GameInputBehavior();
		virtual ~GameInputBehavior();

		virtual bool onInputMove(Scene* scene, const BehaviorData& data);
		virtual bool onInputPressed(Scene* scene, const BehaviorData& data);

		virtual bool onInputDown(Scene* scene, const BehaviorData& data) { return this->pressedEntity != nullptr; }
		virtual bool onInputUp(Scene* scene, const BehaviorData& data) { return this->onInputUpImpl(scene, data); }
		virtual bool onInputReleased(Scene* scene, const BehaviorData& data)  { return this->onInputUpImpl(scene, data); }

	private:

		bool onInputUpImpl(Scene* scene, const BehaviorData& data);

		Entity* pressedEntity;
	
	};

}