#pragma once

#include "global/Resource.h"
#include "gfx/TextureResource.h"
#include "gfx/FrameBuffer.h"
#include "gfx/Geometry.h"
#include "gfx/Color.h"
#include "gfx/TextureHandle.h"

namespace cs
{
	CLASS_DEFINITION_DERIVED(RenderTexture, Resource)
	public:
		RenderTexture(
			const std::string& name, 
			Dimensions dimm, 
			TextureChannels channels, 
			DepthComponent depthComponent = DepthComponentNone);

		void bind(bool force = false, const std::vector<ClearMode>* clearParams = nullptr);
		void unbind();

		TextureResourcePtr getTextureResource() const;
		TexturePtr getTexture() const;
		TexturePtr getDepthTexture() const;
        TextureHandlePtr getTextureHandle();
		
		const DepthBufferPtr& getDepthBuffer() const;
		const FrameBufferPtr& getFrameBuffer() const;
		const Dimensions& getDimensions() const { return this->rttDimm; }
		vec2 getTextureFit() { return this->textureFit; }

	protected:

		Dimensions rttDimm;
		vec2 textureFit;
		TextureResourcePtr textureResource;
		DepthBufferPtr depthBuffer;
		FrameBufferPtr frameBuffer;
	};
}
