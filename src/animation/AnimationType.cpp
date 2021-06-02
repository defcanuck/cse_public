#include "PCH.h"

#include "animation/AnimationType.h"
#include "scene/SceneNode.h"
#include "ecs/Entity.h"

namespace cs
{
    BEGIN_META_CLASS(AnimationPlayBase)
    END_META()

    BEGIN_META_CLASS(AnimationPlayLoop)
    END_META()

    BEGIN_META_CLASS(AnimationPlayBounce)
    END_META()

	BEGIN_META_CLASS(AnimationScaleBase)
	END_META()

	BEGIN_META_CLASS(AnimationScaleCenter)
	END_META()

	BEGIN_META_CLASS(AnimationScaleLeft)
	END_META()

    BEGIN_META_CLASS(AnimatorLerpVec3)
        ADD_MEMBER(minValue);
            SET_MEMBER_NO_SLIDER();
        ADD_MEMBER(maxValue);
            SET_MEMBER_NO_SLIDER();
        ADD_MEMBER(maxTime);
            SET_MEMBER_NO_SLIDER();
            SET_MEMBER_CALLBACK_POST(&AnimatorLerpVec3::onAnimationChanged);
    END_META()

	BEGIN_META_CLASS(AnimatorLerpVec2)
		ADD_MEMBER(minValue);
			SET_MEMBER_NO_SLIDER();
		ADD_MEMBER(maxValue);
			SET_MEMBER_NO_SLIDER();
		ADD_MEMBER(maxTime);
			SET_MEMBER_NO_SLIDER();
			SET_MEMBER_CALLBACK_POST(&AnimatorLerpVec2::onAnimationChanged);
	END_META()

    BEGIN_META_CLASS(AnimatorLerpFloat)
        ADD_MEMBER(minValue);
            SET_MEMBER_NO_SLIDER();
        ADD_MEMBER(maxValue);
            SET_MEMBER_NO_SLIDER();
        ADD_MEMBER(maxTime);
            SET_MEMBER_NO_SLIDER();
            SET_MEMBER_CALLBACK_POST(&AnimatorLerpFloat::onAnimationChanged);
    END_META()

    BEGIN_META_CLASS(AnimatorLerpColor)
        ADD_MEMBER(minValue);
            SET_MEMBER_NO_SLIDER();
        ADD_MEMBER(maxValue);
            SET_MEMBER_NO_SLIDER();
        ADD_MEMBER(maxTime);
            SET_MEMBER_NO_SLIDER();
            SET_MEMBER_CALLBACK_POST(&AnimatorLerpColor::onAnimationChanged);
    END_META()

    // Values

    DEFINE_META_VECTOR_NEW(AnimatedValueList, AnimatedValue, AnimatedValueList)

    BEGIN_META_CLASS(AnimatedValue)
        ADD_MEMBER_PTR(animPlay);
            SET_MEMBER_CALLBACK_POST(&AnimatedValue::onResetAnimation);
            ADD_COMBO_META_LABEL(AnimationPlayBase, "None");
            ADD_COMBO_META_LABEL(AnimationPlayLoop, "Loop");
            ADD_COMBO_META_LABEL(AnimationPlayBounce, "Bounce");
        ADD_MEMBER(speed);
			SET_MEMBER_MIN(0.0f);
			SET_MEMBER_MAX(2.0f);
            SET_MEMBER_CALLBACK_POST(&AnimatedValue::onSpeedChanged);
    END_META()

    // Position

    BEGIN_META_CLASS(AnimationPosition)
        ADD_MEMBER_PTR(posAnimation);
    END_META()

    BEGIN_META_CLASS(AnimationRotation)
        ADD_MEMBER_PTR(rotAnimation);
        ADD_MEMBER(axis);
    END_META()

    BEGIN_META_CLASS(AnimationColor)
        ADD_MEMBER_PTR(colorAnimation);
    END_META()


	BEGIN_META_CLASS(AnimationSize)
		ADD_MEMBER_PTR(sizeAnimation);
		ADD_MEMBER_PTR(scaleType);
			ADD_COMBO_META_LABEL(AnimationScaleCenter, "Center");
			ADD_COMBO_META_LABEL(AnimationScaleLeft, "Left");
	END_META()

	BEGIN_META_CLASS(AnimationTextureUV)
		ADD_MEMBER_PTR(szAnimation);
		ADD_MEMBER_PTR(posAnimation);
	END_META()


	void AnimatedValue::setAnimationType(AnimationType type) 
	{ 
		switch (type)
		{
			case AnimationTypeNone: this->animPlay = CREATE_CLASS(AnimationPlayBase); break;
			case AnimationTypeLoop: this->animPlay = CREATE_CLASS(AnimationPlayLoop); break;
			case AnimationTypeBounce: this->animPlay = CREATE_CLASS(AnimationPlayBounce); break;
		}
	}

    void AnimatedValue::onNew()
    {
        this->onAnimInit();
        this->onResetAnimation();
    }

    void AnimatedValue::onPostLoad(const LoadFlagMask& flags)
    {
        if (flags.test(LoadFlagsAnimation))
        {
            this->onAnimInit();
            this->onResetAnimation();
        }
    }

    void AnimatedValue::reset(SceneNode* node, bool active)
    {
        if (active)
        {
            this->onResetAnimation();
        }
        else
        {
            assert(node);
            node->resetTransform();
        }
    }

    bool AnimatedValue::process(float32 dt)
    {
        if (this->baseInstance)
        {
            return this->baseInstance->process(dt);
        }
		return false;
    }

    void AnimatedValue::onSpeedChanged()
    {
        if (this->baseInstance)
        {
            this->baseInstance->setSpeed(this->speed);
        }
    }

    void AnimatorLerpVec3::getVolumes(VolumeList& selectable_volumes)
    {
        VolumePtr line_volume = CREATE_CLASS(LineVolume, this->minValue, this->maxValue);
        selectable_volumes.push_back(line_volume);
    }

	void AnimatorLerpVec2::getVolumes(VolumeList& selectable_volumes)
	{
		VolumePtr line_volume = CREATE_CLASS(QuadVolume, this->minValue.x, this->minValue.y, this->maxValue.x, this->maxValue.y);
		selectable_volumes.push_back(line_volume);
	}

    void AnimationPosition::onAnimInit()
    {
        if (this->posAnimation)
        {
            this->posAnimation->onChanged += createCallbackArg0(&AnimationPosition::onResetAnimation, this);
        }
    }

    void AnimationPosition::getSelectableVolume(SelectableVolumeList& selectable_volumes)
    {
        if (this->posAnimation)
        {
            VolumeList volumes;
            this->posAnimation->getVolumes(volumes);
            for (auto& it : volumes)
            {
                selectable_volumes.push_back(SelectableVolume());
                SelectableVolume& selectable = selectable_volumes.back();
                selectable.volume = it;
                selectable.type = SelectableVolumeTypePositionAnimation;
                selectable.useInitialTransform = true;
            }
        }
    }

    void AnimationPosition::onResetAnimation()
    {
        if (this->posAnimation)
        {
            AnimationType type = this->animPlay->getType();
            std::shared_ptr<AnimatorTyped<vec3>> anim_ptr = std::static_pointer_cast<AnimatorTyped<vec3>>(this->posAnimation);
            this->instance = Animation<vec3>(anim_ptr, type, this->speed);
            this->baseInstance = dynamic_cast<AnimationBase*>(&this->instance);
        }
    }

    void AnimationPosition::apply(SceneNode* node)
    {
        assert(node);
        vec3 pos = this->instance.getValue();
        const Transform trans = node->getLocalInitialTransform();
        node->setCurrentPosition(trans.translate(pos));
    }

    void AnimationRotation::onAnimInit()
    {
        if (this->rotAnimation)
        {
            this->rotAnimation->onChanged += createCallbackArg0(&AnimationRotation::onResetAnimation, this);
        }
    }

    void AnimationRotation::getSelectableVolume(SelectableVolumeList& selectable_volumes)
    {
        if (this->rotAnimation)
        {
            /*
            VolumeList volumes;
            this->posAnimation->getVolumes(volumes);
            for (auto& it : volumes)
            {
                selectable_volumes.push_back(SelectableVolume());
                SelectableVolume& selectable = selectable_volumes.back();
                selectable.volume = it;
                selectable.type = SelectableVolumeTypePositionAnimation;
                selectable.useInitialTransform = true;
            }
            */
        }
    }

    void AnimationRotation::onResetAnimation()
    {
        if (this->rotAnimation)
        {
            AnimationType type = this->animPlay->getType();
            std::shared_ptr<AnimatorTyped<float32>> anim_ptr = std::static_pointer_cast<AnimatorTyped<float32>>(this->rotAnimation);
            this->instance = Animation<float32>(anim_ptr, type, this->speed);
            this->baseInstance = dynamic_cast<AnimationBase*>(&this->instance);
        }
    }

    void AnimationRotation::apply(SceneNode* node)
    {
        assert(node);
        float32 angle = degreesToRadians(this->instance.getValue());
        const Transform trans = node->getLocalInitialTransform();
        quat rot = glm::quat(this->axis * angle);
        node->setCurrentRotation(trans.rotate(rot));
    }

    void AnimationColor::onAnimInit()
    {
        if (this->colorAnimation)
        {
            this->colorAnimation->onChanged += createCallbackArg0(&AnimationColor::onResetAnimation, this);
        }
    }

    void AnimationColor::getSelectableVolume(SelectableVolumeList& selectable_volumes)
    {
        // nothing
    }

    void AnimationColor::onResetAnimation()
    {
        if (this->colorAnimation)
        {
            AnimationType type = this->animPlay->getType();
            std::shared_ptr<AnimatorTyped<ColorB>> anim_ptr = std::static_pointer_cast<AnimatorTyped<ColorB>>(this->colorAnimation);
            this->instance = Animation<ColorB>(anim_ptr, type, this->speed);
			this->baseInstance = dynamic_cast<AnimationBase*>(&this->instance);
        }
    }

    void AnimationColor::apply(SceneNode* node)
    {
        assert(node);
        ColorB color = this->instance.getValue();
        const Transform trans = node->getLocalInitialTransform();
        Entity* entity = dynamic_cast<Entity*>(node);
        if (entity)
        {
            entity->setColor(color);
        }
    }

	void AnimationSize::onAnimInit()
	{
		if (this->sizeAnimation)
		{
			this->sizeAnimation->onChanged += createCallbackArg0(&AnimationSize::onResetAnimation, this);
		}
	}

	void AnimationSize::getSelectableVolume(SelectableVolumeList& selectable_volumes)
	{
		if (this->sizeAnimation)
		{
			VolumeList volumes;
			this->sizeAnimation->getVolumes(volumes);
			for (auto& it : volumes)
			{
				selectable_volumes.push_back(SelectableVolume());
				SelectableVolume& selectable = selectable_volumes.back();
				selectable.volume = it;
				selectable.type = SelectableVolumeTypePositionAnimation;
				selectable.useInitialTransform = true;
			}
		}
	}

	void AnimationSize::onResetAnimation()
	{
		if (this->sizeAnimation)
		{
			AnimationType type = this->animPlay->getType();
			std::shared_ptr<AnimatorTyped<vec2>> anim_ptr = std::static_pointer_cast<AnimatorTyped<vec2>>(this->sizeAnimation);
			this->instance = Animation<vec2>(anim_ptr, type, this->speed);
			this->baseInstance = dynamic_cast<AnimationBase*>(&this->instance);
		}
	}

	void AnimationSize::apply(SceneNode* node)
	{
		assert(node);
		vec2 sz = this->instance.getValue();
		node->setRenderableSize(sz, this->scaleType->getType());
	}

	void AnimationSize::setAnimationScaleType(AnimationScaleType type)
	{
		switch (type)
		{
			case AnimationScaleTypeCenter:
				this->scaleType = CREATE_CLASS(AnimationScaleCenter);
				break;
			case AnimationScaleTypeLeft:
				this->scaleType = CREATE_CLASS(AnimationScaleLeft);
				break;
		}
	}

	void AnimationTextureUV::onAnimInit()
	{
		if (this->szAnimation)
		{
			this->szAnimation->onChanged += createCallbackArg0(&AnimationTextureUV::onResetAnimation, this);
		}
		if (this->posAnimation)
		{
			this->posAnimation->onChanged += createCallbackArg0(&AnimationTextureUV::onResetAnimation, this);
		}
	}

	
	void AnimationTextureUV::getSelectableVolume(SelectableVolumeList& selectable_volumes)
	{
		// do nothing
	}

	void AnimationTextureUV::onResetAnimation()
	{
		if (this->szAnimation)
		{
			AnimationType type = this->animPlay->getType();

			std::shared_ptr<AnimatorTyped<vec2>> anim_ptr = std::static_pointer_cast<AnimatorTyped<vec2>>(this->szAnimation);
			this->instance = Animation<vec2>(anim_ptr, type, this->speed);
			this->baseInstance = dynamic_cast<AnimationBase*>(&this->instance);
		}
	}

	void AnimationTextureUV::apply(SceneNode* node)
	{
		assert(node);
		std::shared_ptr<AnimatorTyped<vec2>> posPtr = std::dynamic_pointer_cast<AnimatorTyped<vec2>>(this->posAnimation);

		vec2 sz = this->instance.getValue();
		vec2 pos = this->instance.evaluate(posPtr);

		RectF animRect(pos.x, pos.y, sz.x, sz.y);
		node->setRenderableUV(animRect);
	}
    
#if defined(CS_IOS)
    template <>
    vec3 AnimatorLerpTyped<vec3>::getDefaultMin() { return kZero3; }
    template <>
    vec3 AnimatorLerpTyped<vec3>::getDefaultMax() { return kOne3; }

	template <>
	vec2 AnimatorLerpTyped<vec2>::getDefaultMin() { return kZero2; }
	template <>
	vec2 AnimatorLerpTyped<vec2>::getDefaultMax() { return kOne2; }
    
    template <>
    float32 AnimatorLerpTyped<float32>::getDefaultMin() { return 0.0f; }
    template <>
    float32 AnimatorLerpTyped<float32>::getDefaultMax() { return 1.0f; }
    
    template <>
    ColorB AnimatorLerpTyped<ColorB>::getDefaultMin() { return ColorB::Black; }
    template <>
    ColorB AnimatorLerpTyped<ColorB>::getDefaultMax() { return ColorB::White; }
#endif
}
