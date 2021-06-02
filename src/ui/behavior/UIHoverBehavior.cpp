#include "PCH.h"

#include "ui/behavior/UIHoverBehavior.h"
#include "ui/UIElement.h"

namespace cs
{
	bool UIHoverBehavior::onEnter(UIElement* element, UIElementClick& click)
	{
		assert(element);

		if (!this->hover)
		{
			this->oldColor = element->getVertexColors();

			ColorBList newHoverColor;
			switch (this->hoverOp)
			{
				case HoverColorOperationReplace:
					newHoverColor = this->hoverColor;
					break;
				case HoverColorOperationAdd:
					//newHoverColor = this->hoverColor + this->oldColor;
					break;
			}

			element->setVertexColors(newHoverColor);
			this->hover = true;
		}

		return false;
	}

	void UIHoverBehavior::onExit(UIElement* element, ClickInput input)
	{
		if (this->hover)
		{
			this->hover = false;
			element->setVertexColors(this->oldColor);
		}
	}
}