#include "PCH.h"

#include "ui/UIDecorator.h"

namespace cs
{

	UIDecorator::UIDecorator()
		: decoratorColor(ColorB::White)
		, inheritColor(false)
	{ }

	UIDecorator::UIDecorator(const ColorB& color)
		: decoratorColor(color)
		, inheritColor(false)
	{ }

	void UIDecorator::batch(
		uintptr_t ptr,
		BatchDrawList& display_list,
		uint32& numVertices,
		uint16& numIndices,
		float32 depth,
		const RectF& parentRect,
		const ColorB& parentColor,
		const ColorB& vertexColor,
		ShaderHandlePtr& elementShader)
	{

		DrawDataMap::iterator it = this->drawData.find(ptr);
		if (it == this->drawData.end())
		{
			it = this->drawData.emplace(std::make_pair((uintptr_t)ptr, DecoratorDrawData())).first;
		}

		ColorB useColor = (this->inheritColor) ? vertexColor : this->decoratorColor;
		DecoratorDrawData& data = it->second;
		if (data.cachedRect != parentRect ||
			data.depth != depth ||
			data.parentColor != useColor ||
			data.drawData.empty() ||
			data.shader.get() != elementShader.get())
		{
			this->updateGeometry(data, parentRect, depth, useColor, elementShader);
		}

		for (auto& it : data.drawData)
		{
			display_list.push_back(BatchDrawParams(it, numVertices));
			BatchDrawParams& params = display_list.back();
			PointF center(0.0f, 0.0f); // = this->cachedRect.getCenter();

			params.transform = glm::translate(vec3(center.x, center.y, depth));
			params.tint = parentColor;

			numVertices += static_cast<uint32>(it->positions.size());
			numIndices += static_cast<uint16>(it->indices.size());

		}
	}
}