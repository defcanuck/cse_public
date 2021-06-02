#pragma once 

#include "ui/UIBehavior.h"

namespace cs
{

	enum HoverColorOperation
	{
		HoverColorOperationNone = -1,
		HoverColorOperationReplace,
		HoverColorOperationAdd,
		HoverColorOperationMAX
	};

	CLASS_DEFINITION_DERIVED(UIHoverBehavior, UIBehavior)
	public:
		UIHoverBehavior() :
			UIBehavior(),
			hover(false),
			hoverOp(HoverColorOperationReplace),
			hoverColor({ ColorB::White }),
			oldColor({ ColorB::White }) { }

		virtual bool onEnter(UIElement* element, UIElementClick& click);
		virtual void onExit(UIElement* element, ClickInput input);
		void setHoverColor(const ColorBList& col) { this->hoverColor = col; }
		void setOperation(HoverColorOperation op) { this->hoverOp = op; }

	private:

		bool hover;
		HoverColorOperation hoverOp;
		ColorBList hoverColor;
		ColorBList oldColor;

	};
}