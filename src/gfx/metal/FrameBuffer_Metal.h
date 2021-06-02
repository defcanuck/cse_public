#pragma once

#include "gfx/FrameBuffer.h"


namespace cs
{
	CLASS_DEFINITION_DERIVED(FrameBuffer_Metal, FrameBuffer)
	public:
        FrameBuffer_Metal() { }

		virtual void attachColorBuffer(TextureResourcePtr& tex);
		virtual void attachDepthBuffer(DepthBufferPtr& depth);

	protected:

        virtual void bind();
        virtual void unbind();

	};
}
