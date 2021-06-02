#pragma once

#include "ui/UIElement.h"
#include "global/Callback.h"
#include "animation/Animator.h"
#include "audio/SoundManager.h"

namespace cs
{
	CLASS_DEFINITION_DERIVED(UIFlipView, UIElement)
	public:
		UIFlipView(const std::string& name);

		void populate();

		virtual bool shouldDraw() const { return true; }

		virtual bool onCursor(ClickInput input, ClickParams& params, UIClickResults& results, uint32 depth = 0);

		void batch(
			BatchDrawList& display_list,
			uint32& numVertices,
			uint16& numIndices,
			const UIBatchProcessData& data,
			UIBatchProcessInfo& info);
		
		void setSwipeEnabled(bool val);
		bool getSwipeEnabled() const { return this->swipeEnabled; }

		void setPopulateCallback(LuaCallbackPtr pCallback) { this->populateCallback = pCallback; }
		void setAnimateCallback(LuaCallbackPtr aCallback) { this->animateCallback = aCallback; }
        void setUpdateCallback(LuaCallbackPtr aCallback) { this->updateCallback = aCallback; }
		void setAnimationTime(float32 animTime) { this->animationTime = animTime; }

		int32 getIndex() const { return int32(this->index); }
		
		UIElementPtr& getSwipeLeft() { return this->swipeLeft; }
		UIElementPtr& getSwipeRight() { return this->swipeRight; }

		void addSwipeElement(UIElementPtr& element);
		UIElementPtr getSwipeElementAtIndex(int32 index);
        uint32 getNumSwipeElements() const { return this->swipeElements.size(); }
		UIElementPtr getCurrentSwipeElement() { return this->swipeElements[this->index]; }

		void setSwipeSound(const std::string& fileName);

		virtual void setElementAlpha(float32 alpha);

		void doSwipeLeft();
		void doSwipeRight();

	protected:

		void doSwipe(TouchState swipe);
		void setSwipeEnabledImpl();

		LuaCallbackPtr populateCallback;
		LuaCallbackPtr animateCallback;
        LuaCallbackPtr updateCallback;

		UIElementVector swipeElements;
		UIElementPtr swipeLeft;
		UIElementPtr swipeRight;
		bool canSwipeLeft;
		bool canSwipeRight;
		bool isSwipeAnimating;

		Animation<float32> transitionAnimation;
		float32 animationTime;
		size_t index;
		bool swipeEnabled;

		SoundEffectPtr onSwipeSound;
	};
}
