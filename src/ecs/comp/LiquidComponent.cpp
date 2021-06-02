#include "PCH.h"

#include "ecs/comp/LiquidComponent.h"
#include "liquid/LiquidGroup.h"

namespace cs
{

	BEGIN_META_CLASS(LiquidComponent)
	
	END_META()


	LiquidComponent::LiquidComponent()
	{

	}

	void LiquidComponent::process(float32 dt)
	{
		LiquidGroup* group = reinterpret_cast<LiquidGroup*>(this->getParent());
		group->updateParticles(dt);
	}
}