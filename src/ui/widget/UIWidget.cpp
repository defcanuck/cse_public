#include "PCH.h"

#include "ui/widget/UIWidget.h"

namespace cs
{
	void UIWidget::registerInitCallback(uint32 index, UIElementInitFunc& func)
	{
		assert(this->callbacks.count(index) == 0);
		this->callbacks[index] = func;
	}

	void UIWidget::invokeInitCallback(uint32 index, UIElement* ptr)
	{
		UIElementInitFuncCallbacks::iterator it;
		if ((it = this->callbacks.find(index)) != this->callbacks.end())
		{
			it->second(ptr);
		}
	}

	void UIWidget::addChildImpl(UIElementPtr& ptr)
	{
		BASECLASS::addChildImpl(ptr);
		UIWidgetPtr widgetPtr = std::dynamic_pointer_cast<UIWidget>(ptr);
		if (widgetPtr)
		{
			widgetPtr->addOnChangedCallback(this->onChanged);
		}
	}

	void UIWidget::removeChildImpl(UIElementPtr& ptr)
	{
		BASECLASS::removeChildImpl(ptr);
	}
}