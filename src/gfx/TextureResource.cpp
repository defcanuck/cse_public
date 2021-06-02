#include "PCH.h"

#include "gfx/TextureResource.h"
#include "global/Stats.h"

namespace cs
{
	TextureResource::TextureResource()
		: width(0)
		, height(0)
		, channels(TextureNone)
		, fitWidth(1.0f)
		, fitHeight(1.0f)
        , usage(TextureUsageShaderRead)
	{ 
		
	}

	TextureResource::TextureResource(const Dimensions& dimm, TextureChannels c, uchar* data, TextureUsage us)
		: width(dimm.w)
		, height(dimm.h)
		, channels(c)
		, bytes(data)
		, fitWidth(1.0f)
		, fitHeight(1.0f)
        , usage(us)
	{ 
		EngineStats::incrementStat(StatTypeTexture);
	}

	TextureResource::~TextureResource()
	{
		delete[] this->bytes;
		this->bytes = nullptr;

		EngineStats::decrementStat(StatTypeTexture);
	}

	void TextureResource::adjustRect(RectF& rect)
	{
		// TextureHandle::kDefaultTextureSampleRect(0.0f, 1.0f, 1.0f, -1.0f);
		rect.pos.x = 0.0f;
		rect.pos.y = fitHeight;
		rect.size.w = fitWidth;
		rect.size.h = -fitHeight;
	}
    
    void TextureResource::fitPow2()
    {
        if (!checkPow2(this->width) || !checkPow2(this->height))
        {
            uint32 newWidth = (checkPow2(this->width)) ? this->width : nextPow2(this->width);
            uint32 newHeight = (checkPow2(this->height)) ? this->height : nextPow2(this->height);
            
            if (this->bytes)
            {
                
                uint32 sz = newWidth * newHeight * getTextureSize(this->channels);
                uchar* newBytes = new uchar[sz];
                memset(newBytes, 0, sz);
                
                uchar* ptr = newBytes;
                for (uint32 row = 0; row < this->height; row++)
                {
					uint32 tex_size = getTextureSize(this->channels);
					memcpy(ptr, bytes + (this->width * row  * tex_size), this->width * tex_size);
					ptr = ptr + (newWidth * tex_size);
                }
                
                delete[] this->bytes;
                this->bytes = newBytes;
            }
            
            this->fitWidth = this->width / float32(newWidth);
            this->fitHeight = this->height / float32(newHeight);
            
            this->width = newWidth;
            this->height = newHeight;
            
        }
    }
}
