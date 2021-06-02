#pragma once

#include "gfx/Color.h"
#include "gfx/Geometry.h"
#include "gfx/RenderTarget.h"
#include "gfx/PostProcess.h"

namespace cs
{
	struct RenderTargetCapture
	{
		RenderTexturePtr rtt;
		TextureHandlePtr atlasTexture;
		PostProcessList postProcess;
		PostProcessPtr resolveToFront;
		RectF fillRect;
	};

	namespace renderer
	{
		void draw(const mat4& mvp, const ColorB& global_color, GeometryPtr& geom);
        DrawCallPtr createDraw(GeometryPtr& geom, TextureHandlePtr& texture, ShaderHandlePtr& shader);

		void pushRenderTarget(const std::string& name, RenderTargetCapture& capture, const RectI& viewport, RenderTargetType rtt_type = RenderTargetTypeNone);
		void popRenderTarget(RenderTargetCapture& capture, const RectI& viewport);
	}
}
