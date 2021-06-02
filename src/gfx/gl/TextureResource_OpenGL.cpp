#include "PCH.h"

#include "gfx/gl/TextureResource_OpenGL.h"
#include "gfx/TextureLoader.h"
#include "os/FileManager.h"
#include "global/Stats.h"

#include <cassert>
#include <cstring>

namespace cs
{

	TextureSample TextureResource_OpenGL::gSamplerStateU[TextureStageNumMAX];
    TextureSample TextureResource_OpenGL::gSamplerStateV[TextureStageNumMAX];
    
    int32 TextureResource_OpenGL::gBoundTexture[TextureStageNumMAX];
    
	void TextureResource_OpenGL::initSamplers()
	{
		memset(gSamplerStateU, TextureSampleClamp, TextureStageNumMAX * sizeof(TextureSample));
        memset(gSamplerStateV, TextureSampleClamp, TextureStageNumMAX * sizeof(TextureSample));
        
        memset(gBoundTexture, -1, TextureStageNumMAX * sizeof(int32));
        
	}

	TextureResource_OpenGL::TextureResource_OpenGL(const std::string& filePath)
		: TextureResource()
		, textureHandle(0)
		, sizeInBytes(0)
	{
		this->init();
		this->loadFromFile(filePath);
	}

	TextureResource_OpenGL::TextureResource_OpenGL(const Dimensions& dimm, TextureChannels c, uchar* data, TextureUsage us)
		: TextureResource(dimm, c, data, us)
		, textureHandle(0)
		, sizeInBytes(0)
	{
		this->init();
		this->loadData();
	}

	TextureResource_OpenGL::~TextureResource_OpenGL()
	{
		GL_CHECK(glDeleteTextures(1, &this->textureHandle));
		this->textureHandle = 0;

		EngineStats::incrementStatBy(StatTypeTextureSize, -((int32) this->sizeInBytes));
	}
	
	void TextureResource_OpenGL::bind(uint32 stage, bool wu, bool wv)
	{
        bool forceStateChange = false;
        if (int(this->textureHandle) != gBoundTexture[stage])
        {
            GL_CHECK(glActiveTexture(GL_TEXTURE0 + stage));
            GL_CHECK(glBindTexture(GL_TEXTURE_2D, this->textureHandle));
            
            gBoundTexture[stage] = int(this->textureHandle);
            forceStateChange = true;
        }
        
		bool isSquare = checkPow2(this->width) && checkPow2(this->height);
		wu = wu && isSquare;
		wv = wv && isSquare;

		TextureSample sample_u = (wu) ? TextureSampleRepeat : TextureSampleClamp;
		TextureSample sample_v = (wv) ? TextureSampleRepeat : TextureSampleClamp;

		if (gSamplerStateU[stage] != sample_u || forceStateChange)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, kTextureSampleConvert[sample_u]);
            gSamplerStateU[stage] = sample_u;
		}

		if (gSamplerStateV[stage] != sample_v || forceStateChange)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, kTextureSampleConvert[sample_v]);
            gSamplerStateV[stage] = sample_v;
		}
	}

	void TextureResource_OpenGL::init()
	{
		GL_CHECK(glGenTextures(1, &this->textureHandle));
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, this->textureHandle));

        if (kTextureConvertDst[this->channels] != GL_DEPTH_COMPONENT)
        {
            GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        }
	}

	void TextureResource_OpenGL::loadFromFile(const std::string& filePath)
	{
		
		this->bytes = tex::loadImage(filePath, this->width, this->height, this->channels);
		if (!this->bytes)
			return;

		this->fitPow2();

		GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, kTextureConvertSrc[this->channels], this->width, this->height, 0, kTextureConvertDst[this->channels], GL_UNSIGNED_BYTE, this->bytes));
		
		this->sizeInBytes = this->width * this->height * getTextureSize(this->channels);
		EngineStats::incrementStatBy(StatTypeTextureSize, this->sizeInBytes);
	}

	void TextureResource_OpenGL::loadData()
	{
		assert(this->width > 0);
		assert(this->height > 0);
		assert(this->channels != TextureNone);

		GLuint channel_format = GL_UNSIGNED_BYTE;
		if (kTextureConvertDst[this->channels] == GL_DEPTH_COMPONENT)
		{
            switch (this->channels)
            {
                case TextureDepth16:
                    channel_format = GL_UNSIGNED_SHORT;
                    break;
                case TextureDepth24:
                    assert(false);
                    log::error("24 bit depth not supported");
                    break;
                case TextureDepth32:
                    channel_format = GL_UNSIGNED_INT;
                    break;
                default:
                    assert(false);
            }
		}

		if (this->channels == TextureRGBFloat || this->channels == TextureRGBAFloat)
		{
			channel_format = GL_FLOAT;
		}

		this->fitPow2();
        
        uint32 channel_convert = kTextureConvertDst[this->channels];
        
        if (this->bytes || true)
        {
            GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, kTextureConvertSrc[this->channels], this->width, this->height, 0, channel_convert, channel_format, this->bytes));
        }
        else
        {
            GL_CHECK(glTexStorage2D(GL_TEXTURE_2D, 0, kTextureConvertSrc[this->channels], this->width, this->height));
        }

		this->sizeInBytes = this->width * this->height * getTextureSize(this->channels);
		EngineStats::incrementStatBy(StatTypeTextureSize, this->sizeInBytes);

	}
}
