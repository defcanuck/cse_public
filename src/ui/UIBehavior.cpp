#include "PCH.h"

#include "ui/UIBehavior.h"
#include "ui/UIElement.h"

namespace cs
{

	bool UIDragBehavior::onEnter(UIElement* element, UIElementClick& click)
	{
		if (click.state != TouchPressed)
			return false;

		if (!dragging)
		{
			this->dragging = true;
			vec2 absPos = element->getAbsolutePosition();
			this->offset = absPos - click.absolutePos;
		}

		return true;
	}

	bool UIDragBehavior::onUpdate(UIElement* element, vec2& screen_pos)
	{
		if (this->dragging)
		{
			element->setPosition(screen_pos + this->offset);
			return true;
		}

		return false;
	}

	void UIDragBehavior::onExit(UIElement* element, ClickInput input)
	{
		if (this->dragging)
		{
			this->dragging = false;
		}
	}
}