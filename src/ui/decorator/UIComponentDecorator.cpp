#include "PCH.h"

#include "ui/decorator/UIComponentDecorator.h"
#include "gfx/RenderInterface.h"
#include "geom/Shape.h"
#include "global/ResourceFactory.h"

namespace cs
{
	UIComponentDecorator::ComponentParams::ComponentParams()
		: textureHandle(RenderInterface::kDefaultTexture)
	{ }

	UIComponentDecorator::UIComponentDecorator()
		: type(UIComponentDecoratorType3Quadrant)
		, width(20.0f)
		, shader()
	{

	}

	void UIComponentDecorator::updateGeometry(DecoratorDrawData& data, const RectF& parent_rect, float32 depth, const ColorB& useColor, ShaderHandlePtr& elementShader)
	{
		switch (this->type)
		{
			case UIComponentDecoratorType3Quadrant:
				this->updateGeometry3(data, parent_rect, depth, useColor, elementShader);
				break;
			case UIComponentDecoratorType8Quadrant:
				this->updateGeometry8(data, parent_rect, depth, useColor, elementShader);
				break;
		}
	}

	void UIComponentDecorator::updateGeometry8(DecoratorDrawData& data, const RectF& parent_rect, float32 depth, const ColorB& useColor, ShaderHandlePtr& elementShader)
	{
		log::error("TODO");
	}

	void UIComponentDecorator::setShader(const std::string& shaderName)
	{
		ShaderResourcePtr shaderResource = std::static_pointer_cast<ShaderResource>(
			ResourceFactory::getInstance()->loadResource<ShaderResource>(shaderName));
		if (shaderResource.get())
		{
			this->shader = CREATE_CLASS(ShaderHandle, shaderResource);
		}
	}

	void UIComponentDecorator::updateGeometry3(DecoratorDrawData& data, const RectF& parent_rect, float32 depth, const ColorB& useColor, ShaderHandlePtr& elementShader)
	{
		data.drawData.clear();

		{
			data.drawData.push_back(CREATE_CLASS(BatchDrawData));
			BatchDrawDataPtr& cornerDrawData = data.drawData.back();

			cornerDrawData->drawType = DrawTriangles;
			cornerDrawData->shader = (shader.get()) ? shader : elementShader;

			cornerDrawData->indices.clear();
			cornerDrawData->positions.clear();
			cornerDrawData->vcolors.clear();

			struct CornerParams
			{
				vec2 offset;
				vec2 line_offset;
				UIComponentDecoratorQuadrant quadrant;
				UIComponentDecoratorQuadrant side;
				bool flipU;
				bool flipV;
			};

			CornerParams kCornerParams[] =
			{
				{
					vec2(-1.0f, -1.0f),
					vec2(-1.0f,  0.0f),
					UIComponentDecoratorQuadrantCornerBotLeft,
					UIComponentDecoratorQuadrantSideLeft,
					false,
					true
				},
				{
					vec2(-1.0f, 0.0f),
					vec2( 0.0f, 1.0f),
					UIComponentDecoratorQuadrantCornerTopLeft,
					UIComponentDecoratorQuadrantSideTop,
					false,
					false,
				},
				{
					vec2(0.0f, 0.0f),
					vec2(1.0f, 0.0f),
					UIComponentDecoratorQuadrantCornerTopRight,
					UIComponentDecoratorQuadrantSideRight,
					true,
					false,
				},
				{
					vec2(0.0f, -1.0f),
					vec2(0.0f, -1.0f),
					UIComponentDecoratorQuadrantCornerBotRight,
					UIComponentDecoratorQuadrantSideBottom,
					true,
					true
				}
			};

			RectF pctRect = parent_rect;
			pctRect.pos.x -= this->width;
			pctRect.pos.y -= this->width;
			pctRect.size.w += 2.0f * this->width;
			pctRect.size.h += 2.0f * this->width;

			for (uint32 i = 0; i < 4; i++)
			{

				vec2& cornerOffset = kCornerParams[i].offset;
				vec2 p0 = toVec2(parent_rect.getByCorner((RectCorner)i)) + vec2(cornerOffset.x * this->width, cornerOffset.y * this->width);

				RectF cornerRect(p0.x, p0.y, this->width, this->width);

				uint32 indexOffset = i * 4;
				cornerDrawData->indices.push_back(indexOffset + 0);
				cornerDrawData->indices.push_back(indexOffset + 2);
				cornerDrawData->indices.push_back(indexOffset + 1);

				cornerDrawData->indices.push_back(indexOffset + 0);
				cornerDrawData->indices.push_back(indexOffset + 3);
				cornerDrawData->indices.push_back(indexOffset + 2);

				PointF tl = cornerRect.getTL();
				PointF tr = cornerRect.getTR();
				PointF bl = cornerRect.getBL();
				PointF br = cornerRect.getBR();

				cornerDrawData->positions.push_back(vec3(bl.x, bl.y, 0.0f));
				cornerDrawData->positions.push_back(vec3(tl.x, tl.y, 0.0f));
				cornerDrawData->positions.push_back(vec3(tr.x, tr.y, 0.0f));
				cornerDrawData->positions.push_back(vec3(br.x, br.y, 0.0f));

				cornerDrawData->uvs1.push_back(rectPercent(pctRect, PointF(bl.x, bl.y)));
				cornerDrawData->uvs1.push_back(rectPercent(pctRect, PointF(tl.x, tl.y)));
				cornerDrawData->uvs1.push_back(rectPercent(pctRect, PointF(tr.x, tr.y)));
				cornerDrawData->uvs1.push_back(rectPercent(pctRect, PointF(br.x, br.y)));

				for (size_t c = 0; c < 4; c++)
				{
					cornerDrawData->vcolors.push_back(useColor);
				}

				UIComponentDecoratorQuadrant quad = (this->type == UIComponentDecoratorType3Quadrant) ? UIComponentDecoratorQuadrantCorner : kCornerParams[i].quadrant;
				ComponentParams& compParams = this->components[quad];

				cornerDrawData->texture[0] = compParams.textureHandle;
				RectF uvRect = compParams.textureHandle->getUVRect();
				if (this->type == UIComponentDecoratorType3Quadrant)
				{
					if (kCornerParams[i].flipU) uvRect.flipHorizontal();
					if (kCornerParams[i].flipV) uvRect.flipVertical();
				}
				QuadShape::generateUVImpl(uvRect, cornerDrawData->uvs0);

			}

			data.drawData.push_back(CREATE_CLASS(BatchDrawData));
			BatchDrawDataPtr& sideDrawData = data.drawData.back();

			sideDrawData->drawType = DrawTriangles;
			sideDrawData->shader = (shader.get()) ? shader : elementShader;
			sideDrawData->texture[0] = RenderInterface::kDefaultTexture;
			
			sideDrawData->indices.clear();
			sideDrawData->positions.clear();
			sideDrawData->vcolors.clear();

			for (uint32 i = 0; i < 4; i++)
			{
				uint32 idx0 = i;
				uint32 idx1 = (i + 1) % 4;

				vec2 p0 = toVec2(parent_rect.getByCorner((RectCorner)idx0));
				vec2 p1 = toVec2(parent_rect.getByCorner((RectCorner)idx1));

				const vec2& off = kCornerParams[idx0].line_offset;

				uint32 offset = i * 4;
				sideDrawData->indices.push_back(offset + 0);
				sideDrawData->indices.push_back(offset + 2);
				sideDrawData->indices.push_back(offset + 1);

				sideDrawData->indices.push_back(offset + 0);
				sideDrawData->indices.push_back(offset + 3);
				sideDrawData->indices.push_back(offset + 2);

				vec2 bl = p0;
				vec2 tl = p0 + off * this->width;
				vec2 tr = p1 + off * this->width;
				vec2 br = p1;

				sideDrawData->positions.push_back(vec3(bl, 0.0f));
				sideDrawData->positions.push_back(vec3(tl, 0.0f));
				sideDrawData->positions.push_back(vec3(tr, 0.0f));
				sideDrawData->positions.push_back(vec3(br, 0.0f));

				sideDrawData->uvs1.push_back(rectPercent(pctRect, PointF(bl.x, bl.y)));
				sideDrawData->uvs1.push_back(rectPercent(pctRect, PointF(tl.x, tl.y)));
				sideDrawData->uvs1.push_back(rectPercent(pctRect, PointF(tr.x, tr.y)));
				sideDrawData->uvs1.push_back(rectPercent(pctRect, PointF(br.x, br.y)));

				for (size_t c = 0; c < 4; c++)
				{
					sideDrawData->vcolors.push_back(useColor);
				}

				UIComponentDecoratorQuadrant quad = (this->type == UIComponentDecoratorType3Quadrant) ? UIComponentDecoratorQuadrantSide : kCornerParams[i].side;
				ComponentParams& compParams = this->components[quad];

				sideDrawData->texture[0] = compParams.textureHandle;
				RectF uvRect = compParams.textureHandle->getUVRect();

				QuadShape::generateUVImpl(uvRect, sideDrawData->uvs0);

			}
		}

		data.cachedRect = parent_rect;
		data.depth = depth;
		data.parentColor = useColor;
		data.shader = (shader.get()) ? shader : elementShader;

	}

	void UIComponentDecorator::setTexture(UIComponentDecoratorQuadrant quadrant, const std::string& textureName)
	{
		ComponentParams& params = this->components[quadrant];
		params.textureHandle = CREATE_CLASS(TextureHandle, textureName);
		params.textureHandle->setWrapHorizontal(false);
		params.textureHandle->setWrapVertical(false);
	}
}