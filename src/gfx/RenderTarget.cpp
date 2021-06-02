#include "PCH.h"

#include "gfx/RenderTarget.h"
#include "gfx/RenderInterface.h"

#include "gfx/gl/OpenGL.h"

namespace cs
{
	const RenderTargetType RenderTargetManager::kUseBackBuffer = RenderTargetTypeBackBuffer;

	void RenderTargetManager::init(const Dimensions& dimm)
	{
        
        //if (!gltest::testFrameBuffers())
        {
           // log::error("Could not create a valid color+depth frame buffer!");
        }
        
        
		float32 cs = 1.0f / RenderInterface::getInstance()->getContentScale();

		Dimensions fullDimm = dimm;
		fullDimm.x = int32(dimm.x * cs);
		fullDimm.y = int32(dimm.y * cs);

		Dimensions halfDimm;
		halfDimm.x = int32(dimm.x / 2.0f * cs);
		halfDimm.y = int32(dimm.y / 2.0f * cs);

		Dimensions quarterDimm;
		quarterDimm.x = int32(dimm.x / 4 * cs);
		quarterDimm.y = int32(dimm.y / 4 * cs);
        
        TextureChannels channelsToUse = TextureRGB;
		this->renderTargets[RenderTargetTypeBackBuffer] = CREATE_CLASS(RenderTexture, "BackBuffer", fullDimm, channelsToUse, RenderInterface::kDefaultDepthComponent);
		
		this->renderTargets[RenderTargetTypeCopyBuffer] = CREATE_CLASS(RenderTexture, "CopyBuffer", fullDimm, channelsToUse, RenderInterface::kDefaultDepthComponent);
		this->renderTargets[RenderTargetTypeCopyBuffer2] = CREATE_CLASS(RenderTexture, "CopyBuffer2", fullDimm, TextureRGBA, RenderInterface::kDefaultDepthComponent);
		
		this->renderTargets[RenderTargetTypeBackBufferFloat] = CREATE_CLASS(RenderTexture, "BackBufferFloat", fullDimm, TextureRGBFloat, RenderInterface::kDefaultDepthComponent);

		this->renderTargets[RenderTargetTypeCopyBufferHalf] =
			CREATE_CLASS(RenderTexture, "CopyBufferHalf", halfDimm, channelsToUse, RenderInterface::kDefaultDepthComponent);

		this->renderTargets[RenderTargetTypeCopyBufferQuarter] =
			CREATE_CLASS(RenderTexture, "CopyBufferHalf", quarterDimm, channelsToUse, RenderInterface::kDefaultDepthComponent);
        
#if !defined(CS_METAL)
        RenderInterface::getInstance()->setDefaultFrameBuffer();
#endif
        
		SharedUniform::getInstance().getUniform("copy_percent")->setValue(this->renderTargets[RenderTargetTypeCopyBuffer]->getTextureFit());
		SharedUniform::getInstance().getUniform("copy_half_percent")->setValue(this->renderTargets[RenderTargetTypeCopyBufferHalf]->getTextureFit());
		SharedUniform::getInstance().getUniform("copy_quarter_percent")->setValue(this->renderTargets[RenderTargetTypeCopyBufferQuarter]->getTextureFit());
    }

	vec2 RenderTargetManager::getTextureFit(RenderTargetType type)
	{
		RenderTextureMap::iterator it = this->renderTargets.find(type);
		if (it != this->renderTargets.end())
		{
			return it->second->getTextureFit();
		}
		return kOne2;
	}

	RectI RenderTargetManager::getViewport(RenderTargetType type)
	{
		RenderTextureMap::iterator it = this->renderTargets.find(type);
		if (it == this->renderTargets.end())
		{
			return RectI();
		}

		Dimensions dim = it->second->getDimensions();
		return RectI(0, 0, dim.x, dim.y);
	}

	RenderTexturePtr RenderTargetManager::populateSharedTarget(const std::string& name, Dimensions d, TextureChannels c, DepthComponent depth)
	{

        std::map<std::string, SharedRenderTarget>::iterator it = this->sharedTargets.find(name);
        if (it != this->sharedTargets.end())
        {
            log::info("Warning - duplicate targets added");
            return it->second.target;
        }
        
		SharedRenderTarget target;
		target.channels = c;
		target.dimm = d;
		target.depthComponent = depth;
		target.target = CREATE_CLASS(RenderTexture, name, d, c, depth);
		target.mapped = false;
        this->sharedTargets[name] = target;

		return target.target;
	}


	RenderTexturePtr RenderTargetManager::populateSharedTarget(const std::string& name, int32 w, int32 h, TextureChannels c, DepthComponent depth)
	{
		return this->populateSharedTarget(name, Dimensions(w, h), c, depth);
	}

	RenderTexturePtr RenderTargetManager::mapSharedTarget(const std::string& name, Dimensions d, TextureChannels c, DepthComponent depth, RenderTextureSharedData& data)
	{
        
        for (auto& it : this->sharedTargets)
        {
            SharedRenderTarget& target = it.second;
            if (target.mapped)
            {
                continue;
            }

            bool channelsMatch = target.channels == c;
            bool depthMatch = target.depthComponent == depth;
            bool dimmMatch = target.dimm == d;

            if (channelsMatch && depthMatch && dimmMatch)
            {
                assert(target.target.get());
                data.target = target.target;
                data.shared = true;
                target.mapped = true;
				return data.target;
            }
        }
        
        log::info("Warning - creating a Render Target on demand");
		data.target = CREATE_CLASS(RenderTexture, name, d, c, depth);
        data.shared = false;
		return data.target;

	}

	void RenderTargetManager::unmapSharedTarget(RenderTextureSharedData& data)
	{
		for (auto& it : this->sharedTargets)
		{
            SharedRenderTarget& target = it.second;
			if (data.target.get() == target.target.get())
			{
				target.mapped = false;
			}
		}
	}
}
