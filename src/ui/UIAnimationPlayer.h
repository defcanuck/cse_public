#pragma once


#include "animation/Animator.h"
#include "ui/UIAnimation.h"
#include "ui/UIElement.h"
#include "global/Event.h"

namespace cs
{

	CLASS_DEFINITION(UIAnimationPlayer)
	
	public:

		enum UIAnimationPlayerTracks
		{
			UIAnimationPlayerTrackNone = -1,
			UIAnimationPlayerTrackPersistent,
			UIAnimationPlayerTrackTransition,
			//...
			UIAnimationPlayerTracksMAX
		};

		struct AnimElementList
		{
			AnimElementList()
				: animState(AnimationStateNone)
				, element(nullptr)
			{ }

			AnimElementList(const AnimElementList& rhs)
				: animState(rhs.animState)
				, element(rhs.element)
				, onBegin(rhs.onBegin)
				, onEnd(rhs.onEnd)
			{ 
				for (size_t i = 0; i < UIAnimationPlayerTracksMAX; ++i)
					this->animations[i] = rhs.animations[i];
			}
			
			AnimationState animState;
			UIElementPtr element;
			AnimationInstanceList animations[UIAnimationPlayerTracksMAX];
			Event onBegin;
			Event onEnd;
		};

		typedef std::unordered_map<uintptr_t, AnimElementList> AnimList;

		template <class T>
		void addAnimElement(UIElementPtr& element, Animation<T>& animation, AnimationInstancePtr& anim_instance_ptr)
		{
			AnimList::iterator it = this->animList.find((uintptr_t)element.get());
			if (it == this->animList.end())
			{
				AnimElementList elemList;
				elemList.element = element;
				elemList.animState = AnimationStateNone;
				elemList.onBegin += createCallbackArg1<void, UIElementFlag>(&UIElement::setFlag, elemList.element.get(), UIElementFlagAnimating);
				elemList.onEnd += createCallbackArg1<void, UIElementFlag>(&UIElement::unsetFlag, elemList.element.get(), UIElementFlagAnimating);

				it = this->animList.emplace(std::make_pair((uintptr_t)element.get(), elemList)).first;
			}

			UIAnimationPlayerTracks track = UIAnimationPlayerTrackNone;
			switch (animation.getType())
			{
				case AnimationTypeNone:
					track = UIAnimationPlayerTrackTransition;
					break;
				case AnimationTypeLoop:
				case AnimationTypeBounce:
					track = UIAnimationPlayerTrackPersistent;
					break;
			}

			assert(track != UIAnimationPlayerTrackNone);
			(*it).second.animations[track].instances.push_back(anim_instance_ptr);
		}

		template<class T>
		void addTextAnimation(UITextElementPtr& element, Animation<T>& animation, UITextAnimationType animationType)
		{
			UITextElement* elem_ptr = element.get();
			assert(elem_ptr);

			AnimationInstancePtr anim_instance_ptr = UIAnimationCreator::createTextAnimationInstance(element, animation, animationType);
			if (!anim_instance_ptr.get())
			{
				log::error("No animation created - invalid parameters");
				return;
			}
            
            UIElementPtr baseElem = std::static_pointer_cast<UIElement>(element);
			this->addAnimElement<T>(baseElem, animation, anim_instance_ptr);
		}

		template<class T>
		void addAnimation(UIElementPtr& element, Animation<T>& animation, UIAnimationType animationType)
		{
			UIElement* elem_ptr = element.get();
			assert(elem_ptr);

			AnimationInstancePtr anim_instance_ptr = UIAnimationCreator::createAnimationInstance(element, animation, animationType);
			if (!anim_instance_ptr.get())
			{
				log::error("No animation created - invalid parameters");
				return;
			}

			this->addAnimElement<T>(element, animation, anim_instance_ptr);
		}

		template<class T>
		void addUniformAnimation(const std::string& name, UIElementPtr& element, Animation<T>& animation)
		{
			UIElement* elem_ptr = element.get();
			assert(elem_ptr);

			AnimationInstancePtr anim_instance_ptr = UIAnimationCreator::createAnimationUniformInstance(name, element, animation);
			if (!anim_instance_ptr.get())
			{
				log::error("No animation created - invalid parameters");
				return;
			}

			this->addAnimElement<T>(element, animation, anim_instance_ptr);
		}

		template<class T>
		void addAndStartAnimation(UIElementPtr& element, Animation<T>& animation, UIAnimationType animationType)
		{
			this->addAnimation<T>(element, animation, animationType);
			this->start(element);
		}

		template<class T>
		void addAndStartTextAnimation(UITextElementPtr& element, Animation<T>& animation, UITextAnimationType animationType)
		{
			this->addTextAnimation<T>(element, animation, animationType);
            UIElementPtr baseElement = std::static_pointer_cast<UIElement>(element);
			this->start(baseElement);
		}

		void start(UIElementPtr& element);
		void startAll();
		void process(float32 dt);
		void clear();

		void addOnEndCallback(UIElementPtr& element, LuaCallbackPtr& callback);

		AnimList animList;
	};
}
