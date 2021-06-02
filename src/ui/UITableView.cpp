#include "PCH.h"

#include "ui/UITableView.h"

namespace cs
{
	UITableView::UITableView(const std::string& name, UITableViewInitFunc& func) :
		UIElement(name),
		initFunc(func)
	{
		this->setVertexColor(ColorB::Clear);
	}

	void UITableView::init()
	{
		this->children.clear();
		if (this->initFunc)
			this->initFunc(this);

		float32 height = this->layoutChildren();
		
	}

	float32 UITableView::layoutChildren()
	{
		float32 startY = 0.0f;
		float32 height = 0.0f;

		this->tableViewRect = RectF();
		std::cout << "Layout Table View: " << this->getName() << std::endl;
		for (auto it : this->children)
		{

			cs::RectF relative_dimm = it->getMaxRelativeRect();
			
			std::cout << it->getName() << std::endl;
			std::cout << relative_dimm << std::endl;
			
			it->setYPosition(startY, cs::VAlignTop);

			height += relative_dimm.size.h;
			startY -= relative_dimm.size.h;

			this->tableViewRect.combine(it->getMaxRelativeRect());
		}
		std::cout << "----------------------------" << std::endl;
		std::cout << this->tableViewRect << std::endl;
		return height;
	}


	void UITableView::updateLayoutImpl()
	{
		this->layoutChildren();
	}
}