#include "PCH.h"

#include "gfx/metal/FrameBuffer_Metal.h"
#include "gfx/metal/TextureResource_Metal.h"
#include "gfx/RenderInterface.h"

namespace cs
{
	void FrameBuffer_Metal::attachColorBuffer(TextureResourcePtr& tex)
	{
		this->colorBuffer = tex;
	}

	void FrameBuffer_Metal::attachDepthBuffer(DepthBufferPtr& depth)
	{
		this->depthBuffer = depth;
	}

	void FrameBuffer_Metal::bind()
	{
		
	}

	void FrameBuffer_Metal::unbind()
	{
		
	}

}
