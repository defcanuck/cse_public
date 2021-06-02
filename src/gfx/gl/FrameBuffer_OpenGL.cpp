#include "PCH.h"

#include "gfx/gl/FrameBuffer_OpenGL.h"
#include "gfx/gl/TextureResource_OpenGL.h"
#include "gfx/gl/DepthBuffer_OpenGL.h"
#include "gfx/RenderInterface.h"

namespace cs
{
	void FrameBuffer_OpenGL::init()
	{
		//assert(glGenFramebuffers != nullptr);
		//assert(glBindFramebuffer != nullptr);

		GL_CHECK(glGenFramebuffers(1, &this->frameHandle));
		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, this->frameHandle));
	}

	void FrameBuffer_OpenGL::attachColorBuffer(TextureResourcePtr& tex)
	{
		TextureResource_OpenGLPtr gl_ptr = std::static_pointer_cast<TextureResource_OpenGL>(tex);
		if (!gl_ptr)
			return;

		this->colorBuffer = tex;
		GLuint handle = gl_ptr->getTextureHandle();
		
        RenderInterface::getInstance()->clearTextureStage(0);
        
        GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, handle, 0));
	}

	void FrameBuffer_OpenGL::attachDepthBuffer(DepthBufferPtr& depth)
	{
		//assert(glFramebufferRenderbuffer != nullptr);
		DepthBuffer_OpenGLPtr gl_ptr = std::static_pointer_cast<DepthBuffer_OpenGL>(depth);
		if (!gl_ptr)
			return;

        RenderInterface::getInstance()->clearTextureStage(0);
        
		this->depthBuffer = depth;
		const TextureResourcePtr& depthTexture = this->depthBuffer->getDepthTexture();
        
		if (depthTexture.get())
		{
			GLuint depth_tex = gl_ptr->getDepthTextureHandle();
			GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_tex, 0));
		}
		else
		{
			GLuint handle = gl_ptr->getDepthHandle();
			GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, handle));
		}	
	}

	void FrameBuffer_OpenGL::bind()
	{
		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, this->frameHandle));
	}

	void FrameBuffer_OpenGL::unbind()
	{
		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}

	bool FrameBuffer_OpenGL::check()
	{
		//assert(glCheckFramebufferStatus != nullptr);
		GLenum status;
		GL_CHECK(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));
        switch (status)
		{
			case GL_FRAMEBUFFER_COMPLETE:
				return true;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                log::error("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                log::error("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
                break;
#if defined(CS_IOS)
            case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
                log::error("GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS");
                break;
#endif
            case GL_FRAMEBUFFER_UNSUPPORTED:
                log::error("GL_FRAMEBUFFER_UNSUPPORTED");
                break;
			default:
				return false;
		}

		return status == GL_FRAMEBUFFER_COMPLETE;
	}

}
