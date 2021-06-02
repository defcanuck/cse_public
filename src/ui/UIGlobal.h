#pragma once

#include "gfx/TypesGlobal.h"
#include "gfx/BatchDraw.h"
#include "global/Callback.h"
#include "math/Rect.h"

namespace cs
{
	void initUIGlobals();

	extern ShaderHandlePtr defaultShaderResource;
	extern ShaderHandlePtr defaultFontShaderResource;
	extern ShaderHandlePtr defaultMaskResource;

	extern float gResolutionWidthScale;
	extern float gResolutionHeightScale;

}
