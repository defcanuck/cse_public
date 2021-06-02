#include "PCH.h"

#include "ui/behavior/UIClickAnimationBehavior.h"

namespace cs
{

	const static float32 kSpamTimer = 1.0f;

	void UIClickAnimationBehavior::onFirstFrame()
	{
		// restart animations
		if (this->animations.instances.size() > 0)
		{
			this->animState = AnimationStateNone;
			this->animations.onFirstFrame();
		}
	}

	void UIClickAnimationBehavior::process(UIElement* element, float32 dt)
	{
		
		switch (this->animState)
		{
			case AnimationStatePlaying:
			{
				bool isActive = this->animations.process(dt);
				if (!isActive)
				{
					this->animState = AnimationStateFinished;
					this->animations.onEnd(true); 
					this->onEndAnimation.invoke();
					this->spamTimer.start();
                    
                    this->invoke();
				}
			} break;
			case AnimationStateFinished:
			{
				float32 spam_ms = this->spamTimer.getElapsed();
				if (spam_ms > kSpamTimer)
				{
					this->animState = AnimationStateNone;
				}

			} break;
			case AnimationStatePaused:
			default:
				break;
		}
	}

	bool UIClickAnimationBehavior::onEnter(UIElement* element, UIElementClick& click)
	{
		// Ignore all special anim handling if we have 0 animations present
		// maybe log an info here?  This isn't bad per se but it's kind of retarded
		if (click.state == TouchPressed)
		{
			
			if (this->animations.instances.size() == 0)
			{
				this->onBeginAnimation.invoke();
				this->onEndAnimation.invoke();
				this->animState = AnimationStateFinished;
				this->spamTimer.start();
                
			} 
			else if (this->animState == AnimationStateNone)
			{
				this->animState = AnimationStatePlaying;
				this->onBeginAnimation.invoke();
				this->animations.onBegin();
                
			}
			else if (this->animState == AnimationStateFinished)
			{
				// this->animations.onEnd();
			}
			return true;
		}
		return false;
	}
}
