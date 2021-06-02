#pragma once

#include "ecs/system/BaseSystem.h"
#include "ecs/comp/AnimationComponent.h"

namespace cs
{

	class AnimationSystem : public ECSContextSystemBase<AnimationSystem, ECSAnimation>, public BaseSystem
	{

	public:
		AnimationSystem(ECSContext* cxt);
		virtual ~AnimationSystem();

		virtual void processImpl(SystemUpdateParams* params);
		
	private:

	};
}