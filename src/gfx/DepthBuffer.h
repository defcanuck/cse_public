#pragma once

#include "ClassDef.h"
#include "gfx/Types.h"
#include "math/Rect.h"
#include "gfx/TextureResource.h"

namespace cs
{
	CLASS_DEFINITION(DepthBuffer)
	public:

		DepthBuffer() 
			: depthWidth(0)
			, depthHeight(0)
			, depthComponent(DepthComponentNone) 
			, depthTexture(nullptr)
		{ }

		DepthBuffer(const Dimensions& dimm, DepthComponent comp)
			: depthWidth(dimm.w)
			, depthHeight(dimm.h)
			, depthComponent(comp) 
			, depthTexture(nullptr)
		{ }

		virtual void bind(uint32 stage) = 0;
		const TextureResourcePtr& getDepthTexture() const { return this->depthTexture; }

	protected:

		uint32 depthWidth;
		uint32 depthHeight;
		DepthComponent depthComponent;
		TextureResourcePtr depthTexture;
	};
}