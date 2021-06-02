#pragma once

#include "ClassDef.h"
#include "global/Values.h"
#include "gfx/TextureResource.h"

#include <string>

namespace cs
{
	CLASS_DEFINITION_DERIVED(TextureResource_Metal, TextureResource)
	
	public:
		TextureResource_Metal(const std::string& filePath);
		TextureResource_Metal(const Dimensions& dimm, TextureChannels c, uchar* bytes = nullptr, TextureUsage us = TextureUsageShaderRead);
		
		virtual ~TextureResource_Metal();

		virtual void bind(uint32 stage, bool wrapU = false, bool wrapV = false);
        void* getMetalTexture() { return this->mtlTexture; }
    
	private:
    
        void* mtlTexture;
        int32 mtlSize;

		virtual void init();
		virtual void loadFromFile(const std::string& fileName);
		virtual void loadData();

	};
}
