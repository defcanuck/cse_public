#pragma once

#include "animation/Animator.h"
#include "ui/UIElement.h"
#include "ui/UITextElement.h"

namespace cs
{

	enum UIAnimationType
	{
		UIAnimationTypeWidth,
		UIAnimationTypeHeight,
		UIAnimationTypeX,
		UIAnimationTypeY,
		UIAnimationTypePosition,
		UIAnimationTypeSize,
		UIAnimationTypeVertexColor,
		UIAnimationTypeVertexAlpha,
		UIAnimationTypeElementColor,
		UIAnimationTypeElementAlpha,
		//...
		UIAnimationTypeMAX
	};

	enum UITextAnimationType
	{
		UITextElementTypeFontColor,
		UITextElementTypeFontAlpha,
		UITextElementTypeFontShadow,
		UITextElementTypeTextScale,
		//
		UITextElementTypeFontMAX
	};

	extern const char* kAnimationTypeStr[];
	extern const char* kTextAnimationTypeStr[];

	class UIElement;

	class UIAnimationCreator
	{
	public:

		template <class T>
		static Animation<T> createTextAnimation(UITextElementPtr& element, UITextAnimationType ui_anim_type, const T& new_value, const float32& max_time, const float32& smooth, AnimationType anim_type)
		{
			return Animation<T>();
		}

		template <class T>
		static Animation<T> createAnimation(UIElementPtr& element, UIAnimationType ui_anim_type, const T& new_value, const float32& max_time, const float32& smooth, AnimationType anim_type)
		{
			return Animation<T>();
		}

		template <class T>
		static Animation<T> createUniformAnimation(const T& start_value, const T& end_value, const float32& max_time, const float32& smooth, AnimationType anim_type)
		{
			return Animation<T>();
		}
		
		template<class T>
		static AnimationInstancePtr createAnimationInstance(UIElementPtr& element, Animation<T>& animation, UIAnimationType AnimationType)
		{
			log::print(LogError, "Cannot find Animation routine for type ", kAnimationTypeStr[AnimationType]);
			return nullptr;
		}

		template<class T>
		static AnimationInstancePtr createTextAnimationInstance(UITextElementPtr& element, Animation<T>& animation, UITextAnimationType animationType)
		{
			log::print(LogError, "Cannot find Text Animation routine for type ", kTextAnimationTypeStr[animationType]);
			return nullptr;
		}

		template<class T>
		static AnimationInstancePtr createAnimationUniformInstance(const std::string& uniformName, UIElementPtr& element, Animation<T>& animation)
		{
			log::print(LogError, "Cannot find Animation");
			return nullptr;
		}
	};

	template <>
	AnimationInstancePtr UIAnimationCreator::createAnimationInstance<float32>(UIElementPtr& element, Animation<float32>& animation, UIAnimationType ui_anim_type);

	template <>
	AnimationInstancePtr UIAnimationCreator::createAnimationInstance<vec2>(UIElementPtr& element, Animation<vec2>& animation, UIAnimationType ui_anim_type);

	template <>
	AnimationInstancePtr UIAnimationCreator::createAnimationInstance<vec3>(UIElementPtr& element, Animation<vec3>& animation, UIAnimationType ui_anim_type);

	template <>
	AnimationInstancePtr UIAnimationCreator::createAnimationInstance<ColorB>(UIElementPtr& element, Animation<ColorB>& animation, UIAnimationType ui_anim_type);

	template <>
	AnimationInstancePtr UIAnimationCreator::createAnimationInstance<ColorF>(UIElementPtr& element, Animation<ColorF>& animation, UIAnimationType ui_anim_type);

	template <>
	AnimationInstancePtr UIAnimationCreator::createAnimationUniformInstance<float32>(const std::string& uniformName, UIElementPtr& element, Animation<float32>& animation);


	template <>
	AnimationInstancePtr UIAnimationCreator::createTextAnimationInstance<float32>(UITextElementPtr& element, Animation<float32>& animation, UITextAnimationType ui_anim_type);

	template <>
	AnimationInstancePtr UIAnimationCreator::createTextAnimationInstance<vec2>(UITextElementPtr& element, Animation<vec2>& animation, UITextAnimationType ui_anim_type);

	template <>
	AnimationInstancePtr UIAnimationCreator::createTextAnimationInstance<vec3>(UITextElementPtr& element, Animation<vec3>& animation, UITextAnimationType ui_anim_type);

	template <>
	AnimationInstancePtr UIAnimationCreator::createTextAnimationInstance<ColorB>(UITextElementPtr& element, Animation<ColorB>& animation, UITextAnimationType ui_anim_type);

	template <>
	AnimationInstancePtr UIAnimationCreator::createTextAnimationInstance<ColorF>(UITextElementPtr& element, Animation<ColorF>& animation, UITextAnimationType ui_anim_type);


	template <>
	Animation<float32> UIAnimationCreator::createTextAnimation(UITextElementPtr& element, UITextAnimationType ui_anim_type, const float32& new_value, const float32& max_time, const float32& smooth, AnimationType anim_type);

	template <>
	Animation<ColorB> UIAnimationCreator::createTextAnimation(UITextElementPtr& element, UITextAnimationType ui_anim_type, const ColorB &new_value, const float32& max_time, const float32& smooth, AnimationType anim_type);

	template <>
	Animation<ColorF> UIAnimationCreator::createTextAnimation(UITextElementPtr& element, UITextAnimationType ui_anim_type, const ColorF& new_value, const float32& max_time, const float32& smooth, AnimationType anim_type);

	template <>
	Animation<vec2> UIAnimationCreator::createTextAnimation(UITextElementPtr& element, UITextAnimationType ui_anim_type, const vec2 &new_value, const float32& max_time, const float32& smooth, AnimationType anim_type);

	template <>
	Animation<vec3> UIAnimationCreator::createTextAnimation(UITextElementPtr& element, UITextAnimationType ui_anim_type, const vec3& new_value, const float32& max_time, const float32& smooth, AnimationType anim_type);



	template <>
	Animation<float32> UIAnimationCreator::createAnimation(UIElementPtr& element, UIAnimationType ui_anim_type, const float32& new_value, const float32& max_time, const float32& smooth, AnimationType anim_type);

	template <>
	Animation<ColorB> UIAnimationCreator::createAnimation(UIElementPtr& element, UIAnimationType ui_anim_type, const ColorB &new_value, const float32& max_time, const float32& smooth, AnimationType anim_type);

	template <>
	Animation<ColorF> UIAnimationCreator::createAnimation(UIElementPtr& element, UIAnimationType ui_anim_type, const ColorF& new_value, const float32& max_time, const float32& smooth, AnimationType anim_type);

	template <>
	Animation<vec2> UIAnimationCreator::createAnimation(UIElementPtr& element, UIAnimationType ui_anim_type, const vec2 &new_value, const float32& max_time, const float32& smooth, AnimationType anim_type);

	template <>
	Animation<vec3> UIAnimationCreator::createAnimation(UIElementPtr& element, UIAnimationType ui_anim_type, const vec3& new_value, const float32& max_time, const float32& smooth, AnimationType anim_type);

	template <>
	Animation<float32> UIAnimationCreator::createUniformAnimation(const float32& start_value, const float32& end_value, const float32& max_time, const float32& smooth, AnimationType anim_type);

	template <>
    void AnimationSharedCallbackHandler<UIElement>::onBegin(std::shared_ptr<UIElement>& object);

	template <>
    void AnimationSharedCallbackHandler<UIElement>::onEnd(std::shared_ptr<UIElement>& object);
}	
