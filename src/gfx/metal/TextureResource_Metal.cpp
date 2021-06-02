#include "PCH.h"

#include "gfx/metal/TextureResource_Metal.h"
#include "gfx/TextureLoader.h"
#include "os/FileManager.h"

#include "MTLRenderInterface.h"

#include <cassert>
#include <cstring>

namespace cs
{

	TextureResource_Metal::TextureResource_Metal(const std::string& filePath)
        : TextureResource()
        , mtlTexture(nullptr)
        , mtlSize(0)
	{
		this->init();
		this->loadFromFile(filePath);
	}

	TextureResource_Metal::TextureResource_Metal(const Dimensions& dimm, TextureChannels c, uchar* data, TextureUsage us)
        : TextureResource(dimm, c, data, us)
        , mtlTexture(nullptr)
        , mtlSize(0)
	{
		this->init();
		this->loadData();
	}

	TextureResource_Metal::~TextureResource_Metal()
	{
        MTLFreeTexture(this->mtlTexture, this->mtlSize);
	}
	
	void TextureResource_Metal::bind(uint32 stage, bool wu, bool wv)
	{
        
	}

	void TextureResource_Metal::init()
	{
		
	}

	void TextureResource_Metal::loadFromFile(const std::string& filePath)
	{
		
		this->bytes = tex::loadImage(filePath, this->width, this->height, this->channels);
		if (!this->bytes)
			return;

        this->loadData();
	}

	void TextureResource_Metal::loadData()
	{
		assert(this->width > 0);
		assert(this->height > 0);
		assert(this->channels != TextureNone);

        this->fitPow2();
        
        this->mtlSize = this->width * this->height * getTextureSize(this->channels);
		this->mtlTexture = MTLCreateTexture(this->width, this->height, this->channels, this->bytes, this->usage);
	}
}
