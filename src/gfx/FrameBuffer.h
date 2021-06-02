#pragma once

#include "ClassDef.h"

#include "gfx/DepthBuffer.h"
#include "gfx/TextureResource.h"
#include "gfx/DepthBuffer.h"

namespace cs
{
	CLASS_DEFINITION(FrameBuffer)
	public:
		FrameBuffer() :
			colorBuffer(nullptr),
			depthBuffer(nullptr) { }

		virtual ~FrameBuffer() { }

		virtual void attachColorBuffer(TextureResourcePtr& tex) = 0;
		virtual void attachDepthBuffer(DepthBufferPtr& depth) = 0;

		virtual bool check() { return true; }

		TextureResourcePtr getColorBuffer() const { return colorBuffer; }
		DepthBufferPtr getDepthBuffer() const { return depthBuffer; }

	protected:
    
        friend class RenderInterface;
    
        virtual void bind() = 0;
        virtual void unbind() = 0;

		TextureResourcePtr colorBuffer;
		DepthBufferPtr depthBuffer;
	};
}
