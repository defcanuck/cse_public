#include "PCH.h"

#include "gfx/gl/DepthBuffer_OpenGL.h"
#include "gfx/gl/TextureResource_OpenGL.h"
#include "gfx/RenderInterface.h"

namespace cs
{
  
	DepthBuffer_OpenGL::~DepthBuffer_OpenGL()
	{
        if (this->handle > 0)
            GL_CHECK(glDeleteRenderbuffers(1, &this->handle));
        
        log::info("Destroying depth buffer?");
	}

	void DepthBuffer_OpenGL::init(bool isRenderTexture)
	{
		
        if (isRenderTexture)
        {
            RenderInterface* ri = RenderInterface::getInstance();
            Dimensions dimm(this->depthWidth, this->depthHeight);
            TextureChannels channels;
            switch (this->depthComponent)
            {
#if defined(CS_WINDOWS)
                case DepthComponent16: channels = TextureDepth16; break;
                case DepthComponent24: channels = TextureDepth24; break;
                case DepthComponent32: channels = TextureDepth32; break;
#else
                default:
                    channels = TextureDepth16;
                    break;
#endif
            }
            this->depthTexture = ri->loadTexture(dimm, channels);
        }
        else
        {
            GL_CHECK(glGenRenderbuffers(1, &this->handle));
            GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, this->handle));
            uint32 depth_convert = kDepthComponentConvert[this->depthComponent];
            GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, depth_convert, this->depthWidth, this->depthHeight));
        }
	}

	void DepthBuffer_OpenGL::bind(uint32 stage)
	{
		if (this->depthTexture)
		{
			this->depthTexture->bind(stage);
		}
	}

	GLuint DepthBuffer_OpenGL::getDepthTextureHandle() const
	{
		TextureResource_OpenGLPtr tex_ptr = std::static_pointer_cast<TextureResource_OpenGL>(this->depthTexture);
		return tex_ptr->getTextureHandle();
	}
}
