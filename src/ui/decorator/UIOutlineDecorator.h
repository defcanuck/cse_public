#pragma once

#include "ClassDef.h"

#include "ui/UIGlobal.h"
#include "ui/UIDecorator.h"

#include "gfx/Color.h"

namespace cs
{
	CLASS_DEFINITION_DERIVED(UIOutlineDecorator, UIDecorator)
	public:

		UIOutlineDecorator(const ColorB& oc, float32 ow) 
			: UIDecorator(oc)
			, outlineWidth(ow)
			, shader(CREATE_CLASS(ShaderHandle, defaultShaderResource)) {}

	private:

		virtual void updateGeometry(DecoratorDrawData& data, const RectF& parent_rect, float32 depth, const ColorB& useColor, ShaderHandlePtr& elementShader);

		float32 outlineWidth;
		ShaderHandlePtr shader;

	};
}