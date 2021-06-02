#include "PCH.h"

#include "ui/decorator/UIOutlineDecorator.h"

namespace cs
{


	void UIOutlineDecorator::updateGeometry(DecoratorDrawData& data, const RectF& parent_rect, float32 depth, const ColorB& useColor, ShaderHandlePtr& elementShader)
	{

		data.drawData.clear();
		data.drawData.push_back(CREATE_CLASS(BatchDrawData));
		BatchDrawDataPtr& outlineDrawData = data.drawData.back();

		vec2 offsets[] =
		{
			vec2(-1.0f, -1.0f),
			vec2(-1.0f, 1.0f),
			vec2(1.0f, 1.0f),
			vec2(1.0f, -1.0f)
		};

		outlineDrawData->drawType = DrawTriangles;
		outlineDrawData->shader = (this->shader.get()) ? this->shader : elementShader;
		outlineDrawData->texture[0] = RenderInterface::kWhiteTexture;
		
		outlineDrawData->indices.clear();
		outlineDrawData->positions.clear();
		outlineDrawData->vcolors.clear();

		for (uint32 i = 0; i < 4; i++)
		{
			uint32 idx0 = i;
			uint32 idx1 = (i + 1) % 4;

			vec2 p0 = toVec2(parent_rect.getByCorner((RectCorner)idx0));
			vec2 p1 = toVec2(parent_rect.getByCorner((RectCorner)idx1));

			const vec2& off0 = offsets[idx0];
			const vec2& off1 = offsets[idx1];

			uint32 offset = i * 4;
			outlineDrawData->indices.push_back(offset + 1);
			outlineDrawData->indices.push_back(offset + 2);
			outlineDrawData->indices.push_back(offset + 0);

			outlineDrawData->indices.push_back(offset + 0);
			outlineDrawData->indices.push_back(offset + 2);
			outlineDrawData->indices.push_back(offset + 3);

			outlineDrawData->positions.push_back(vec3(p0, 0.0f));
			outlineDrawData->positions.push_back(vec3(p0 + off0 * this->outlineWidth, 0.0f));
			outlineDrawData->positions.push_back(vec3(p1 + off1 * this->outlineWidth, 0.0f));
			outlineDrawData->positions.push_back(vec3(p1, 0.0f));

			for (size_t c = 0; c < 4; c++)
			{
				outlineDrawData->vcolors.push_back(useColor);
			}
		}

		data.cachedRect = parent_rect;
		data.depth = depth;
		data.parentColor = useColor;
	}
}