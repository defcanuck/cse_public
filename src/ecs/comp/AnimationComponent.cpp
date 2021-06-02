#include "PCH.h"

#include "ecs/ECS_Utils.h"
#include "ecs/comp/AnimationComponent.h"

namespace cs
{

	BEGIN_META_CLASS(AnimationMethod)
	END_META()

	BEGIN_META_CLASS(AnimationMethodValue)
		ADD_MEMBER(valueList);
			ADD_COMBO_META_LABEL(AnimationPosition, "Position");
			ADD_COMBO_META_LABEL(AnimationRotation, "Rotation");
			ADD_COMBO_META_LABEL(AnimationColor, "Color");
			ADD_COMBO_META_LABEL(AnimationSize, "Size");
			ADD_COMBO_META_LABEL(AnimationTextureUV, "TextureUV");
	END_META()
	
	BEGIN_META_CLASS(AnimationMethodPath)

	END_META()

	BEGIN_META_CLASS(AnimationComponent)
		ADD_META_FUNCTION("Reset", &AnimationComponent::resetAnimations);
		ADD_MEMBER_PTR(method);
			ADD_COMBO_META_LABEL(AnimationMethodPath, "By Path Nodes");
			ADD_COMBO_META_LABEL(AnimationMethodValue, "By Value");
	END_META()


	void AnimationMethodValue::getSelectableVolume(SelectableVolumeList& selectable_volumes)
	{
		for (auto& it : this->valueList)
		{
			it->getSelectableVolume(selectable_volumes);
		}		
	}

	void AnimationMethodValue::reset(SceneNode* node, bool active)
	{
		for (auto& it : this->valueList)
		{
			it->reset(node, active);
		}
	}

	bool AnimationMethodValue::process(AnimationUpdateParams* params)
	{
		if (this->valueList.size() > 0)
		{
			bool ret = true;
			for (auto& it : this->valueList)
			{
				ret = it->process(params->dt) && ret;
				it->apply(params->node);
			}
			return ret;
		}
		return false;
	}

	void AnimationMethodValue::onPostLoad(const LoadFlagMask& flags)
	{
		for (auto& it : this->valueList)
		{
			it->onPostLoad(flags);
		}
	}

	void AnimationMethodValue::resetAnimations()
	{
		for (auto& it : this->valueList)
		{
			it->onResetAnimation();
		}
	}

	AnimationComponent::AnimationComponent()
		: method(CREATE_CLASS(AnimationMethodValue))
		, active(true)
	{

	}

	void AnimationComponent::process(SystemUpdateParams* params)
	{
		AnimationUpdateParams animParams;
		animParams.dt = params->animationDt;
		animParams.camera = params->camera;
		animParams.node = reinterpret_cast<SceneNode*>(this->getParent());
		if (this->method)
		{
			if (this->active)
			{
				if (this->method->process(&animParams))
				{
					// execute the callback
					if (this->onAnimationComplete.get())
					{
						(*this->onAnimationComplete)();
					}

					this->active = false;
				}
			}
		}
	}

	void AnimationComponent::getSelectableVolume(SelectableVolumeList& selectable_volumes)
	{
		if (this->method)
		{
			this->method->getSelectableVolume(selectable_volumes);
		}
	}

	void AnimationComponent::onPostLoad(const LoadFlagMask& flags)
	{
		if (this->method)
		{
			this->method->onPostLoad(flags);
		}
	}

	void AnimationComponent::reset(bool active)
	{
		if (this->method)
		{
			SceneNode* node = reinterpret_cast<SceneNode*>(this->getParent());
			if (node)
			{
				this->method->reset(node, active);
			}
		}
		this->active = true;
	}

	void AnimationComponent::resetAnimations()
	{
		if (this->method.get())
		{
			this->method->resetAnimations();
		}
	}

}