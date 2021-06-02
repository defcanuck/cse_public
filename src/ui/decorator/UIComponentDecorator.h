#pragma once

#include "ClassDef.h"

#include "ui/UIGlobal.h"
#include "ui/UIDecorator.h"

#include "gfx/Color.h"

namespace cs
{

	enum UIComponentDecoratorType
	{
		UIComponentDecoratorTypeNone = -1,
		UIComponentDecoratorType3Quadrant,
		UIComponentDecoratorType8Quadrant,
		UIComponentDecoratorTypeMAX
	};

	enum UIComponentDecoratorQuadrant
	{
		UIComponentDecoratorQuadrantNone = -1,
		
		// Corners
		UIComponentDecoratorQuadrantCorner,
		UIComponentDecoratorQuadrantCornerTopLeft = UIComponentDecoratorQuadrantCorner,
		UIComponentDecoratorQuadrantCornerTopRight,
		UIComponentDecoratorQuadrantCornerBotLeft,
		UIComponentDecoratorQuadrantCornerBotRight,

		// Sides
		UIComponentDecoratorQuadrantSide,
		UIComponentDecoratorQuadrantSideLeft = UIComponentDecoratorQuadrantSide,
		UIComponentDecoratorQuadrantSideRight,
		UIComponentDecoratorQuadrantSideTop,
		UIComponentDecoratorQuadrantSideBottom,

		UIComponentDecoratorQuadrantMAX
	};

	CLASS_DEFINITION_DERIVED(UIComponentDecorator, UIDecorator)
	public:

		UIComponentDecorator();

		void setTexture(UIComponentDecoratorQuadrant quadrant, const std::string& textureName);
		void setWidth(float32 w) { this->width = w; }
		void setShader(const std::string& shaderName);
		ShaderHandlePtr& getShader() { return this->shader; }

	protected:

		virtual void updateGeometry(DecoratorDrawData& data, const RectF& parent_rect, float32 depth, const ColorB& useColor, ShaderHandlePtr& elementShader);

	private:

		void updateGeometry3(DecoratorDrawData& data, const RectF& parent_rect, float32 depth, const ColorB& useColor, ShaderHandlePtr& elementShader);
		void updateGeometry8(DecoratorDrawData& data, const RectF& parent_rect, float32 depth, const ColorB& useColor, ShaderHandlePtr& elementShader);

		struct ComponentParams
		{
			ComponentParams();

			TextureHandlePtr textureHandle;
		};

		UIComponentDecoratorType type;
		ComponentParams components[UIComponentDecoratorQuadrantMAX];
		float32 width;
		ShaderHandlePtr shader;

	};
}