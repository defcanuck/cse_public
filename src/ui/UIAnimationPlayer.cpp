#include "PCH.h"

#include "ui/UIAnimationPlayer.h"

namespace cs
{
	void UIAnimationPlayer::process(float32 dt)
	{

		std::vector<uintptr_t> keysToDelete;
		for (auto& it : this->animList)
		{
			AnimElementList& cur_anim = it.second;
			switch (cur_anim.animState)
			{
				case AnimationStatePlaying:
				{
					// First do the persistent
					bool isActive = true;
					bool isTransitionActive = cur_anim.animations[UIAnimationPlayerTrackTransition].process(dt);
					if (!isTransitionActive && cur_anim.animations[UIAnimationPlayerTrackTransition].instances.size() > 0)
					{
						cur_anim.animations[UIAnimationPlayerTrackTransition].onEnd();

						Event toCallEvent = cur_anim.onEnd;
						cur_anim.onEnd.clear();
						toCallEvent.invoke();

						// The above callback is free to add additional animations
						if (cur_anim.animations[UIAnimationPlayerTrackTransition].instances.size() > 0)
						{
							isTransitionActive = true;
						}
					}
					bool isPersistentActive = cur_anim.animations[UIAnimationPlayerTrackPersistent].process(dt);
					if (!isPersistentActive && cur_anim.animations[UIAnimationPlayerTrackPersistent].instances.size() > 0)
					{
						cur_anim.animations[UIAnimationPlayerTrackPersistent].onEnd();
					}

					isActive = isPersistentActive || isTransitionActive;
					if (!isActive)
					{
						cur_anim.animState = AnimationStateFinished;
					}
				} break;
				
				case AnimationStateFinished:
					keysToDelete.emplace_back(it.first);
					break;

				case AnimationStatePaused:
				default:
					break;
			}
		}

		// nothing to do, stop tracks
		for (auto&& key : keysToDelete)
		{
			this->animList.erase(key);
		}
	}

	void UIAnimationPlayer::start(UIElementPtr& element)
	{
		AnimList::iterator it = this->animList.find((uintptr_t)element.get());
		if (it != this->animList.end())
		{
			it->second.animState = AnimationStatePlaying;

			for (size_t i = 0; i < UIAnimationPlayerTracksMAX; ++i)
			{
				it->second.animations[i].onBegin();
			}
		}
	}

	void UIAnimationPlayer::addOnEndCallback(UIElementPtr& element, LuaCallbackPtr& callback)
	{
		AnimList::iterator it = this->animList.find((uintptr_t)element.get());
		if (it != this->animList.end())
		{
			AnimElementList& elemList = it->second;
			elemList.onEnd.addLuaCallback(callback);
		}
	}

	void UIAnimationPlayer::startAll()
	{
		
		
		for (auto& it : this->animList)
		{
			AnimElementList& cur_anim = it.second;
			if (cur_anim.animations[UIAnimationPlayerTrackPersistent].instances.size() == 0 &&
				cur_anim.animations[UIAnimationPlayerTrackTransition].instances.size() == 0)
			{
				// Ignore all special anim handling if we have 0 animations present
				// maybe log an info here?  This isn't bad per se but it's kind of retarded
				log::info("No animations to play - this is dumb");
				cur_anim.animState = AnimationStateFinished;
			}
			else if (cur_anim.animState == AnimationStateNone)
			{
				cur_anim.animState = AnimationStatePlaying;
				for (size_t i = 0; i < UIAnimationPlayerTracksMAX; ++i)
				{
					cur_anim.animations[i].onBegin();
				}

				// If we're starting a transition on this element, invoke the callback
				if (cur_anim.animations[UIAnimationPlayerTrackTransition].instances.size() > 0)
				{
					cur_anim.onBegin.invoke();
				}
			}
		}
	}

	void UIAnimationPlayer::clear()
	{
		this->animList.clear();
	}
}