#pragma once

#include "ClassDef.h"
#include "global/Values.h"
#include "gfx/TextureResource.h"
#include "gfx/gl/OpenGL.h"

#include <string>

namespace cs
{
	CLASS_DEFINITION_DERIVED(TextureResource_OpenGL, TextureResource)
	
	public:
		TextureResource_OpenGL(const std::string& filePath);
		TextureResource_OpenGL(const Dimensions& dimm, TextureChannels c, uchar* bytes = nullptr, TextureUsage us = TextureUsageShaderRead);
		
		virtual ~TextureResource_OpenGL();

		virtual void bind(uint32 stage, bool wrapU = false, bool wrapV = false);
		virtual uint32 getTextureId() const { return (uint32) textureHandle; }
		virtual uint32* getTextureIdPtr() const { return (uint32*) &textureHandle; }

		virtual GLuint getTextureHandle() const { return textureHandle; }

		static void initSamplers();

	private:
	
		virtual void init();
		virtual void loadFromFile(const std::string& fileName);
		virtual void loadData();

		GLuint textureHandle;

		static TextureSample gSamplerStateU[TextureStageNumMAX];
        static TextureSample gSamplerStateV[TextureStageNumMAX];
    
        static int32 gBoundTexture[TextureStageNumMAX];

		size_t sizeInBytes;

	};
}
