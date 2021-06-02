#pragma once

#include "ClassDef.h"

#include "ui/UIGlobal.h"

#include <vector>

namespace cs
{

	CLASS_DEFINITION(UIDecorator)
	public:

		UIDecorator();
		UIDecorator(const ColorB& color);

		virtual void batch(
			uintptr_t ptr,
			BatchDrawList& display_list,
			uint32& numVertices,
			uint16& numIndices,
			float32 depth,
			const RectF& parentRect,
			const ColorB& parentColor,
			const ColorB& vertexColor,
			ShaderHandlePtr& elementShader);


		void setDecoratorColor(const ColorB& color) { this->decoratorColor = color; }
		void setInheritColor(bool inherit) { this->inheritColor = inherit; }

	protected:

		struct DecoratorDrawData
		{
			RectF cachedRect;
			std::vector<BatchDrawDataPtr> drawData;
			float32 depth;
			ColorB parentColor;
			ShaderHandlePtr shader;
		};

		virtual void updateGeometry(DecoratorDrawData& data, const RectF& parent_rect, float32 depth, const ColorB& useColor, ShaderHandlePtr& elementShader) { };

		typedef std::map<uintptr_t, DecoratorDrawData> DrawDataMap;
		DrawDataMap drawData;
		ColorB decoratorColor;
		bool inheritColor;
	};

}