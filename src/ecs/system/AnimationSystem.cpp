#include "PCH.h"

#include "ecs/system/AnimationSystem.h"
#include "ecs/ECS_Utils.h"

namespace cs
{
	AnimationSystem::AnimationSystem(ECSContext* cxt)
		: BaseSystem(cxt)
	{
		this->subscribeForComponent<AnimationComponent>(this->parentContext);
	}

	AnimationSystem::~AnimationSystem()
	{
         this->removeComponentSubscription<AnimationComponent>(this->parentContext);
	}

	void AnimationSystem::processImpl(SystemUpdateParams* params)
	{
		BaseSystem::ComponentIdMap anims;
		this->getEnabledComponents<AnimationComponent>(anims);
		for (const auto it : anims)
		{
			const AnimationComponentPtr& anim =
				std::static_pointer_cast<AnimationComponent>(it.second);

			anim->process(params);
		}
	}
}
