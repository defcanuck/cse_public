#pragma once

#include "ui/UIElement.h"

namespace cs
{
	class UITableView;
	typedef std::function<void(UITableView*)> UITableViewInitFunc;

	CLASS_DEFINITION_DERIVED(UITableView, UIElement)
	public:
		UITableView(const std::string& name, UITableViewInitFunc& func);

		void init();
		
		virtual bool shouldDraw() const { return true; }
		float32 layoutChildren();

		void setInitFunc(UITableViewInitFunc& ifunc) { this->initFunc = ifunc; }

	protected:

		virtual void updateLayoutImpl();
		
		UITableViewInitFunc initFunc;
		RectF tableViewRect;

	};
}