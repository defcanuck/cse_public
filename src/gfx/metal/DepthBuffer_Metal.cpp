#include "PCH.h"

#include "gfx/metal/DepthBuffer_Metal.h"
#include "gfx/metal/TextureResource_Metal.h"
#include "gfx/RenderInterface.h"

#include "MTLRenderInterface.h"

namespace cs
{
  
	DepthBuffer_Metal::~DepthBuffer_Metal()
	{
       
	}

	void DepthBuffer_Metal::init()
	{
        RenderInterface* ri = RenderInterface::getInstance();
        Dimensions dimm(this->depthWidth, this->depthHeight);
        TextureChannels channels;
        
        switch (this->depthComponent)
        {
            case DepthComponent16: channels = TextureDepth16; break;
            case DepthComponent24: channels = TextureDepth24; break;
            case DepthComponent32: channels = TextureDepth32; break;
        }
        this->depthTexture = ri->loadTexture(dimm, channels, nullptr, TextureUsageUsageRenderTarget);
	}

	void DepthBuffer_Metal::bind(uint32 stage)
	{
		// unused
	}
}
