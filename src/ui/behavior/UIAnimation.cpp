#include "PCH.h"

#include "ui/UIAnimation.h"

namespace cs
{
	const char* kAnimationTypeStr[]
	{
		"Width",				// UIAnimationTypeWidth
		"height",				// UIAnimationTypeHeight
		"X",					// UIAnimationTypeX
		"Y",					// UIAnimationTypeY
		"Position",				// UIAnimationTypePosition
		"Size",					// UIAnimationTypeSize
		"VertexColor",			// UIAnimationTypeVertexColor
		"VertexAlpha"			// UIAnimationTypeVertexAlpha
		"ElementColor"			// UIAnimationTypeElementColor
		"ElementAlpha"			// UIAnimationTypeElementAlpha
	};

	const char* kTextAnimationTypeStr[] =
	{
		"FontColor",			// UITextElementTypeFontColor
		"FontAlpha",			// UITextElementTypeFontAlpha
		"FontShadow"			// UITextElementTypeFontShadow
	};

	template <class T>
	Animation<T> createAnimationInternal(const T& initial_value, const T& new_value, const float32& max_time, const float32& smooth, AnimationType anim_type)
	{
		
		return LerpAnimator<T>::createAnimation(initial_value, new_value, max_time, smooth, anim_type);		
	}

	template <>
	Animation<float32> UIAnimationCreator::createTextAnimation(UITextElementPtr& element, UITextAnimationType ui_anim_type, const float32& new_value, const float32& max_time, const float32& smooth, AnimationType anim_type)
	{
		assert(max_time > 0.0f);
		float32 initial_value = 0.0f;
		switch (ui_anim_type)
		{
			case UITextElementTypeFontAlpha: initial_value = element->getFontAlpha(); break;
			case UITextElementTypeFontShadow: initial_value = element->getShadowAnimation(); break;
			case UITextElementTypeTextScale: initial_value = element->getTextScale(); break;
		}
		return createAnimationInternal<float32>(initial_value, new_value, max_time, smooth, anim_type);
	}

	template <>
	Animation<float32> UIAnimationCreator::createUniformAnimation(const float32& start_value, const float32& end_value, const float32& max_time, const float32& smooth, AnimationType anim_type)
	{
		return createAnimationInternal<float32>(start_value, end_value, max_time, smooth, anim_type);
	}

	template <>
	Animation<ColorB> UIAnimationCreator::createTextAnimation(UITextElementPtr& element, UITextAnimationType ui_anim_type, const ColorB &new_value, const float32& max_time, const float32& smooth, AnimationType anim_type)
	{
		return Animation<ColorB>();
	}

	template <>
	Animation<ColorF> UIAnimationCreator::createTextAnimation(UITextElementPtr& element, UITextAnimationType ui_anim_type, const ColorF& new_value, const float32& max_time, const float32& smooth, AnimationType anim_type)
	{
		return Animation<ColorF>();
	}

	template <>
	Animation<vec2> UIAnimationCreator::createTextAnimation(UITextElementPtr& element, UITextAnimationType ui_anim_type, const vec2 &new_value, const float32& max_time, const float32& smooth, AnimationType anim_type)
	{
		return Animation<vec2>();
	}

	template <>
	Animation<vec3> UIAnimationCreator::createTextAnimation(UITextElementPtr& element, UITextAnimationType ui_anim_type, const vec3& new_value, const float32& max_time, const float32& smooth, AnimationType anim_type)
	{
		return Animation<vec3>();
	}

	template <>
	Animation<float32> UIAnimationCreator::createAnimation(UIElementPtr& element, UIAnimationType ui_anim_type, const float32& new_value, const float32& max_time, const float32& smooth, AnimationType anim_type)
	{
		float32 initial_value = 0.0f;
		switch (ui_anim_type)
		{
			case UIAnimationTypeVertexAlpha: initial_value = element->getVertexAlpha(); break;
			case UIAnimationTypeElementAlpha: initial_value = element->getElementAlpha(); break;
			case UIAnimationTypeWidth: initial_value = element->getRawWidth(); break;
			case UIAnimationTypeHeight: initial_value = element->getRawHeight(); break;
			case UIAnimationTypeX: initial_value = element->getRawXPosition(); break;
			case UIAnimationTypeY: initial_value = element->getRawYPosition(); break;
			default:
				log::error("Invalid type for float32 - ", kAnimationTypeStr[ui_anim_type]);
		}
		
		return createAnimationInternal<float32>(initial_value, new_value, max_time, smooth, anim_type);
	}

	template <>
	Animation<ColorB> UIAnimationCreator::createAnimation(UIElementPtr& element, UIAnimationType ui_anim_type, const ColorB &new_value, const float32& max_time, const float32& smooth, AnimationType anim_type)
	{
		ColorB initial_value = ColorB::White;
		switch (ui_anim_type)
		{
			case UIAnimationTypeVertexColor: initial_value = element->getVertexColor(); break;
			case UIAnimationTypeElementColor: initial_value = element->getElementColor(); break;
			default:
			log::error("Invalid type for ColorB - ", kAnimationTypeStr[ui_anim_type]);
		}

		return createAnimationInternal<ColorB>(initial_value, new_value, max_time, smooth, anim_type);
	}

	template <>
	Animation<ColorF> UIAnimationCreator::createAnimation(UIElementPtr& element, UIAnimationType ui_anim_type, const ColorF& new_value, const float32& max_time, const float32& smooth, AnimationType anim_type)
	{
		return Animation<ColorF>();
	}

	template <>
	Animation<vec2> UIAnimationCreator::createAnimation(UIElementPtr& element, UIAnimationType ui_anim_type, const vec2 &new_value, const float32& max_time, const float32& smooth, AnimationType anim_type)
	{
		return Animation<vec2>();
	}

	template <>
	Animation<vec3> UIAnimationCreator::createAnimation(UIElementPtr& element, UIAnimationType ui_anim_type, const vec3& new_value, const float32& max_time, const float32& smooth, AnimationType anim_type)
	{
		return Animation<vec3>();
	}

	template <>
	AnimationInstancePtr UIAnimationCreator::createAnimationUniformInstance<float32>(const std::string& uniformName, UIElementPtr& element, Animation<float32>& animation)
	{
		return createNamedAnimationCallback<float32>(uniformName, animation, &UIElement::setUniformValue<float32>, element);
	}

	template <>
	AnimationInstancePtr UIAnimationCreator::createAnimationInstance<float32>(UIElementPtr& element, Animation<float32>& animation, UIAnimationType ui_anim_type)
	{
		switch (ui_anim_type)
		{
			case UIAnimationTypeVertexAlpha:
				return createAnimationCallback<float32>(animation, &UIElement::setVertexAlpha, element);
			case UIAnimationTypeElementAlpha:
				return createAnimationCallback<float32>(animation, &UIElement::setElementAlpha, element);
			case UIAnimationTypeWidth:
				return createAnimationCallback<float32>(animation, &UIElement::setRawWidth, element);
			case UIAnimationTypeHeight:
				return createAnimationCallback<float32>(animation, &UIElement::setRawHeight, element);
			case UIAnimationTypeX:
				return createAnimationCallback<float32>(animation, &UIElement::setRawXPosition, element);
			case UIAnimationTypeY:
				return createAnimationCallback<float32>(animation, &UIElement::setRawYPosition, element);
			default:
				log::error("Invalid type for float32 - ", kAnimationTypeStr[ui_anim_type]);
		}

		return AnimationInstancePtr();
	}

	template <>
	AnimationInstancePtr UIAnimationCreator::createAnimationInstance<vec2>(UIElementPtr& element, Animation<vec2>& animation, UIAnimationType ui_anim_type)
	{
		return AnimationInstancePtr();
	}

	template <>
	AnimationInstancePtr UIAnimationCreator::createAnimationInstance<vec3>(UIElementPtr& element, Animation<vec3>& animation, UIAnimationType ui_anim_type)
	{
		return AnimationInstancePtr();
	}

	template <>
	AnimationInstancePtr UIAnimationCreator::createAnimationInstance<ColorB>(UIElementPtr& element, Animation<ColorB>& animation, UIAnimationType ui_anim_type)
	{
		switch (ui_anim_type)
		{
			case UIAnimationTypeVertexColor:
				return createAnimationCallback<ColorB>(animation, &UIElement::setVertexColor, element);
			case UIAnimationTypeElementColor:
				return createAnimationCallback<ColorB>(animation, &UIElement::setElementColor, element);
			default:
				log::error("Invalid type for float32 - ", kAnimationTypeStr[ui_anim_type]);
		}

		return AnimationInstancePtr();
	}

	template <>
	AnimationInstancePtr UIAnimationCreator::createAnimationInstance<ColorF>(UIElementPtr& element, Animation<ColorF>& animation, UIAnimationType ui_anim_type)
	{
		return AnimationInstancePtr();
	}

	template <>
	AnimationInstancePtr UIAnimationCreator::createTextAnimationInstance<float32>(UITextElementPtr& element, Animation<float32>& animation, UITextAnimationType ui_anim_type)
	{
		switch (ui_anim_type)
		{
			case UITextElementTypeFontAlpha:
				return createAnimationCallback<float32>(animation, &UITextElement::setFontAlpha, element);
			case UITextElementTypeFontShadow:
				return createAnimationCallback<float32>(animation, &UITextElement::setShadowAnimation, element);
			case UITextElementTypeTextScale:
				return createAnimationCallback<float32>(animation, &UITextElement::setTextScale, element);
			default:
				log::error("Invalid type for float32 - ", kAnimationTypeStr[ui_anim_type]);
		}

		return AnimationInstancePtr();
	}

	template <>
	AnimationInstancePtr UIAnimationCreator::createTextAnimationInstance<vec2>(UITextElementPtr& element, Animation<vec2>& animation, UITextAnimationType ui_anim_type)
	{
		return AnimationInstancePtr();
	}

	template <>
	AnimationInstancePtr UIAnimationCreator::createTextAnimationInstance<vec3>(UITextElementPtr& element, Animation<vec3>& animation, UITextAnimationType ui_anim_type)
	{
		return AnimationInstancePtr();
	}

	template <>
	AnimationInstancePtr UIAnimationCreator::createTextAnimationInstance<ColorB>(UITextElementPtr& element, Animation<ColorB>& animation, UITextAnimationType ui_anim_type)
	{
		return AnimationInstancePtr();
	}

	template <>
	AnimationInstancePtr UIAnimationCreator::createTextAnimationInstance<ColorF>(UITextElementPtr& element, Animation<ColorF>& animation, UITextAnimationType ui_anim_type)
	{
		return AnimationInstancePtr();
	}
    
    template <>
    void AnimationSharedCallbackHandler<UIElement>::onBegin(std::shared_ptr<UIElement>& object)
    {
        object->setEnabled(false);
    }
    
    template <>
    void AnimationSharedCallbackHandler<UIElement>::onEnd(std::shared_ptr<UIElement>& object)
    {
        object->setEnabled(true);
    }

}
