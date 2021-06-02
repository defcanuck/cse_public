#pragma once

#include "ui/UITextElement.h"

namespace cs
{
	CLASS_DEFINITION_DERIVED(UICollapseable, UITextElement)
	public:
		UICollapseable(const std::string& name, bool e = false);

		void expand();
		void collapse();
		void setState(bool e);
		void toggleExpand();

		void setExpandedVertexColor(const ColorB& vc) { this->expandedVertexColor[0] = vc; }
		void setExpandedVertexColors(const ColorBList& vc) { this->expandedVertexColor = vc; }

	protected:

		virtual void batchChildren(
			BatchDrawList& display_list,
			uint32& numVertices,
			uint16& numIndices,
			const ColorB& tint,
			const UIBatchProcessData& data,
			UIBatchProcessInfo& info);

		virtual ColorB getColorByIndex(int32 i);
		virtual void combineChildrenRect(RectF& combineRect, const RectF& parent_rect);
		virtual void combineChildrenDimm(PointF& combineDimm, const PointF& parent_dimm);

		ColorBList expandedVertexColor;
		bool expanded;

	};

};