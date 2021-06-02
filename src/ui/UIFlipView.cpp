#include "PCH.h"

#include "ui/UIFlipView.h"
#include "ui/behavior/UIClickBehavior.h"

namespace cs
{
	UIFlipView::UIFlipView(const std::string& name)
		: UIElement(name)
		, index(0)
		, transitionAnimation()
		, animationTime(0.5f)
		, swipeEnabled(true)
		, canSwipeLeft(true)
		, canSwipeRight(true)
		, isSwipeAnimating(false)
	{

	}

	void UIFlipView::populate()
	{
		this->removeAllChildren();
		if (this->populateCallback.get())
		{
			(*this->populateCallback)(this, int32(this->index));
		}

		this->swipeLeft = CREATE_CLASS(UIElement, "swipe_left");
		this->swipeRight = CREATE_CLASS(UIElement, "swipe_right");

		CallbackArg0<void>::CallbackFunc onClickLeft = std::bind(&UIFlipView::doSwipeLeft, this);
		CallbackArg0<void>::CallbackFunc onClickRight = std::bind(&UIFlipView::doSwipeRight, this);
		
		UIClickBehaviorPtr clickLeft = CREATE_CLASS(cs::UIClickBehavior, onClickLeft);
		UIClickBehaviorPtr clickRight = CREATE_CLASS(cs::UIClickBehavior, onClickRight);
		
		this->swipeLeft->addBehavior(clickRight);
		this->swipeRight->addBehavior(clickLeft);


		this->addChild(this->swipeLeft);
		this->addChild(this->swipeRight);
	}


	void UIFlipView::addSwipeElement(UIElementPtr& element)
	{
		this->swipeElements.push_back(element);
		BASECLASS::addChild(element);
	}

	UIElementPtr UIFlipView::getSwipeElementAtIndex(int32 index)
	{
		if (index < 0 || index >= this->swipeElements.size())
		{
			return UIElementPtr();
		}
		return this->swipeElements[index];
	}

	void UIFlipView::setSwipeEnabled(bool val)
	{
		this->swipeEnabled = val;
		this->setSwipeEnabledImpl();
	}

	void UIFlipView::setSwipeEnabledImpl()
	{
		if (this->swipeLeft.get())
		{
			bool isOn = !this->isSwipeAnimating && this->swipeEnabled && this->canSwipeLeft;
			this->swipeLeft->setVisible(isOn);
			this->swipeLeft->setEnabled(isOn);
		}
		if (this->swipeRight.get())
		{
			bool isOn = !this->isSwipeAnimating && this->swipeEnabled && this->canSwipeRight;
			this->swipeRight->setVisible(isOn);
			this->swipeRight->setEnabled(isOn);
		}
	}

	void UIFlipView::setSwipeSound(const std::string& fileName)
	{
		this->onSwipeSound = SoundManager::getInstance()->getSound(fileName);
	}

	void UIFlipView::doSwipeLeft()
	{
		this->doSwipe(TouchSwipeLeft);
	}

	void UIFlipView::doSwipeRight()
	{
		this->doSwipe(TouchSwipeRight);
	}

	void UIFlipView::doSwipe(TouchState swipe)
	{
		if (this->onSwipeSound.get())
		{
			this->onSwipeSound->play();
		}

		int32 offset = 0;
		switch (swipe)
		{
			case TouchSwipeLeft: offset = 1; break;
			case TouchSwipeRight: offset = -1; break;
		}

		this->index = clamp(size_t(0), this->swipeElements.size() - 1, index + offset);
		this->transitionAnimation = DummyAnimator<float32>::createAnimation(this->animationTime, 1.0f);

		for (int32 swipe_index = 0; swipe_index < int32(this->swipeElements.size()); ++swipe_index)
		{
			UIElementPtr elem = this->swipeElements[swipe_index];
			int32 indexOffset = swipe_index - int32(this->index);

			(*this->animateCallback)(swipe_index, indexOffset, offset);
		}

		this->canSwipeLeft = this->index > 0;
		this->canSwipeRight = this->index < (this->swipeElements.size() - 1);

		if (this->swipeLeft)
		{
			this->swipeLeft->setVisible(this->canSwipeLeft);
			this->swipeLeft->setEnabled(this->canSwipeLeft);
		}

		if (this->swipeRight)
		{
			this->swipeRight->setVisible(this->canSwipeRight);
			this->swipeRight->setEnabled(this->canSwipeRight);
		}

		if (this->updateCallback.get())
			(*this->updateCallback)();
	}

	bool UIFlipView::onCursor(ClickInput input, ClickParams& params, UIClickResults& results, uint32 depth)
	{

		if (this->transitionAnimation.hasAnim() && !this->transitionAnimation.isAnimDone())
			return false;

		if (this->getSwipeEnabled() && params.state > TouchSwipeFirst)
		{

			switch (params.state)
			{
				case TouchSwipeLeft:
				{
					if (this->index >= this->swipeElements.size() - 1)
					{
						return BASECLASS::onCursor(input, params, results, depth);
					}
					break;
				}
				case TouchSwipeRight:
				{
					if (this->index <= 0)
					{
						return BASECLASS::onCursor(input, params, results, depth);
					}
					break;
				}
			}
			this->doSwipe(params.state);

			return true;
		}
		
		// Only trigger a pressed state if we have a touch and release
		// prioritizes swipes over pushes
		if (params.state == TouchReleased)
		{
			params.state = TouchPressed;
			return BASECLASS::onCursor(input, params, results, depth);
		}

		return false;
	}

	void UIFlipView::setElementAlpha(float32 alpha)
	{
		BASECLASS::setElementAlpha(alpha);
	}

	void UIFlipView::batch(
		BatchDrawList& display_list,
		uint32& numVertices,
		uint16& numIndices,
		const UIBatchProcessData& data,
		UIBatchProcessInfo& info)
	{

		if (this->transitionAnimation.hasAnim() && !this->transitionAnimation.isAnimDone())
		{
			this->transitionAnimation.process(data.delta);
			this->isSwipeAnimating = true;
			this->setSwipeEnabledImpl();
		}
		else
		{
			this->isSwipeAnimating = false;
			this->setSwipeEnabledImpl();
		}

		if (!data.animating || true)
		{
			BASECLASS::batch(display_list, numVertices, numIndices, data, info);
		}
		else
		{
			float32 depthInc = data.depth + kDepthIncrement;

			UIBatchProcessData child_data(data);
			child_data.depth = depthInc + this->depthBias;
			child_data.tint = data.tint * this->elementColor;
			child_data.bounds = this->screen_rect;

			for (size_t i = 0; i < this->children.size(); ++i)
			{
				if (this->index != i)
					continue;

				UIElementPtr& child = this->children[i];
				if (child->getVisible())
				{
					child->batch(display_list, numVertices, numIndices, child_data, info);
				}
			}
		}
	}

}
