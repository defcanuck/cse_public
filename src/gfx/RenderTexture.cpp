#include "PCH.h"

#include "gfx/RenderTexture.h"
#include "gfx/RenderInterface.h"
namespace cs
{

	RenderTexture::RenderTexture(const std::string& n, Dimensions dimm, TextureChannels channels, DepthComponent depthComponent) :
		Resource(n),
		rttDimm(Dimensions(nextPow2(dimm.w), nextPow2(dimm.h)))
	{
		this->textureFit = vec2(dimm.x / float32(rttDimm.x), dimm.y / float32(rttDimm.y));

		RenderInterface* ri = RenderInterface::getInstance();
		this->frameBuffer = ri->createFrameBuffer();

        if (this->frameBuffer.get())
        {
            this->textureResource = ri->loadTexture(dimm, channels, nullptr, TextureUsageUsageRenderTarget);
            ri->clearTextureStage(0);
            
            bool useDepth = true;
#if defined(CS_IOS) && !defined(CS_METAL)
            useDepth = false;
#endif
            if (depthComponent != DepthComponentNone && useDepth)
            {
                this->depthBuffer = ri->createDepthBuffer(rttDimm, depthComponent);
                assert(this->depthBuffer.get());
            }

#if !defined(CS_METAL)
            RenderInterface::getInstance()->setFrameBuffer(n, this->frameBuffer, false);
#endif
            
            this->frameBuffer->attachColorBuffer(this->textureResource);
            if (this->depthBuffer)
            {
                this->frameBuffer->attachDepthBuffer(this->depthBuffer);
            }
            
#if !defined(CS_METAL)
            this->frameBuffer->check();
            RenderInterface::getInstance()->clearFrameBuffer();
#endif
        }
    }
    
    TextureHandlePtr RenderTexture::getTextureHandle()
    {
        return CREATE_CLASS(TextureHandle, this->getTexture());
    }
    
	TextureResourcePtr RenderTexture::getTextureResource() const
	{
		if (!this->frameBuffer)
			return nullptr;
		return this->frameBuffer->getColorBuffer();
	}

	TexturePtr RenderTexture::getTexture() const
	{
		if (!this->frameBuffer)
			return nullptr;

        std::stringstream texName;
        texName << this->name << "_ColorBuffer";
		TexturePtr texture = CREATE_CLASS(Texture, texName.str(), this->frameBuffer->getColorBuffer());
		return texture;
	}

	TexturePtr RenderTexture::getDepthTexture() const
	{
		if (!this->depthBuffer)
			return nullptr;

		TexturePtr depthTexture = std::static_pointer_cast<Texture>(CREATE_CLASS(Texture, "DepthTexture", this->depthBuffer->getDepthTexture()));
		return depthTexture;
	}

	const FrameBufferPtr& RenderTexture::getFrameBuffer() const
	{
		return this->frameBuffer;
	}

	const DepthBufferPtr& RenderTexture::getDepthBuffer() const
	{
		return this->depthBuffer;
	}

	void RenderTexture::bind(bool force, const std::vector<ClearMode>* clearParams)
	{
		RenderInterface* ri = RenderInterface::getInstance();
		ri->setFrameBuffer(this->getName(), this->frameBuffer, false, force, clearParams);
	}

	void RenderTexture::unbind()
	{
		RenderInterface* ri = RenderInterface::getInstance();
        ri->setDefaultFrameBuffer();
	}

}
