#pragma once

#include "scene/behaviors/Behavior.h"
#include "scene/Actor.h"

namespace cs
{

	CLASS_DEFINITION_DERIVED(ScriptInputBehavior, Behavior)

	public:

		ScriptInputBehavior();
		virtual ~ScriptInputBehavior();

		virtual bool onInputMove(Scene* scene, const BehaviorData& data);
		virtual bool onInputPressed(Scene* scene, const BehaviorData& data);
		virtual bool onInputReleased(Scene* scene, const BehaviorData& data);
	
	private:

		Entity* pressedEntity;
	};

}