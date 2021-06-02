#pragma once

#include "ClassDef.h"

#include "ui/behavior/UIClickBehavior.h"
#include "ui/UIElement.h"
#include "global/Timer.h"

#include "animation/Animator.h"
#include "ui/UIAnimation.h"

namespace cs
{

	CLASS_DEFINITION_DERIVED(UIClickAnimationBehavior, UIClickBehavior)
	public:

		UIClickAnimationBehavior()
			: animState(AnimationStateNone)
		{ }

		UIClickAnimationBehavior(Event& e)
			: UIClickBehavior(e)
			, animState(AnimationStateNone)
		{ }

		UIClickAnimationBehavior(std::vector<CallbackPtr>& fcs)
			: UIClickBehavior(fcs)
			, animState(AnimationStateNone)
		{ }

		UIClickAnimationBehavior(CallbackArg0<void>::CallbackFunc& fn)
			: UIClickBehavior(fn)
			, animState(AnimationStateNone)
		{ }

		UIClickAnimationBehavior(CallbackArg0<void>::callbackFunc fn)
			: UIClickBehavior(fn)
			, animState(AnimationStateNone)
		{ }

		UIClickAnimationBehavior(CallbackPtr callback)
			: UIClickBehavior(callback)
			, animState(AnimationStateNone)
		{ }

		virtual bool onEnter(UIElement* element, UIElementClick& click);
		virtual void process(UIElement* element, float32 dt);

		template<class T>
		void addAnimation(UIElementPtr& element, Animation<T>& animation, UIAnimationType AnimationType)
		{
			UIElement* elem_ptr = element.get();
			assert(elem_ptr);

			AnimationInstancePtr anim_instance_ptr = UIAnimationCreator::createAnimationInstance(element, animation, AnimationType);
			if (!anim_instance_ptr.get())
			{
				log::error("No animation created - invalid parameters");
				return;
			}

			this->animations.instances.push_back(anim_instance_ptr);
		}

		template<class T>
		void addUniformAnimation(const std::string& uniformName, UIElementPtr& element, Animation<T>& animation)
		{
			UIElement* elem_ptr = element.get();
			assert(elem_ptr);

			AnimationInstancePtr anim_instance_ptr = UIAnimationCreator::createAnimationUniformInstance(uniformName, element, animation);
			if (!anim_instance_ptr.get())
			{
				log::error("No animation created - invalid parameters");
				return;
			}

			this->animations.instances.push_back(anim_instance_ptr);
		}

		virtual void onFirstFrame();
		
		virtual bool canProcessDisabled() const { return true; }

		Event onBeginAnimation;
		Event onEndAnimation;

	private:

		AnimationState animState;
		AnimationInstanceList animations;
		Timer spamTimer;
	};
}