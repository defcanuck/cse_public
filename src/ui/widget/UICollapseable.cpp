#include "PCH.h"

#include "ui/widget/UICollapseable.h"
#include "ui/behavior/UIClickBehavior.h"

namespace cs
{
	UICollapseable::UICollapseable(const std::string& name, bool e) :
		UITextElement(name),
		expanded(e)
	{
		cs::CallbackArg0<void>::CallbackFunc onExpand = std::bind(&UICollapseable::toggleExpand, this);
		cs::UIClickBehaviorPtr click = CREATE_CLASS(cs::UIClickBehavior, onExpand);
		this->addBehavior(click);

		this->expandedVertexColor.push_back(ColorB::White);
	}

	ColorB UICollapseable::getColorByIndex(int32 i)
	{
		if (this->expanded)
		{
			int32 idx = std::min(i, int32(this->expandedVertexColor.size()) - 1);
			return this->expandedVertexColor[idx];
		}
		size_t idx = std::min(i, kNumVertexColors - 1);
		return this->vertexColor[idx];
	}

	void UICollapseable::combineChildrenDimm(PointF& combineDimm, const PointF& parent_dimm)
	{
		if (!this->expanded)
			return;

		for (auto it : this->children)
		{
			combineDimm.combine(it->getMaxDimensions(parent_dimm));
		}
	}

	void UICollapseable::combineChildrenRect(RectF& combineRect, const RectF& parent_rect)
	{
		if (!this->expanded)
			return;

		for (auto it : this->children)
		{
			RectF relativeRect = it->getMaxRelativeRect(parent_rect);
			combineRect.combine(relativeRect);
		}
	}

	void UICollapseable::batchChildren(
		BatchDrawList& display_list,
		uint32& numVertices,
		uint16& numIndices,
		const ColorB& tint,
		const UIBatchProcessData& data,
		UIBatchProcessInfo& info)
	{
		if (this->expanded)
		{
			BASECLASS::batchChildren(display_list, numVertices, numIndices, tint, data, info);
		}
	}

	void UICollapseable::expand() 
	{ 
		this->expanded = true; 
		for (auto it : this->children)
			it->triggerRecursive();

		this->updateLayout();
	}

	void UICollapseable::collapse() 
	{ 
		this->expanded = false; 
		for (auto it : this->children)
			it->closeRecursive();
		this->updateLayout();
	}

	void UICollapseable::setState(bool e) 
	{ 
		this->expanded = e; 
		
		this->updateLayout();
	}

	void UICollapseable::toggleExpand() 
	{ 
		this->expanded = !this->expanded; 
		for (auto it : this->children)
		{
			it->setEnabledRecursive(this->expanded);
			it->setVisibleRecursive(this->expanded);
		}
		
		this->updateLayout();
	}
}