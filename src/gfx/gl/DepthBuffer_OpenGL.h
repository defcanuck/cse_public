#pragma once

#include "gfx/DepthBuffer.h"
#include "gfx/gl/OpenGL.h"

namespace cs
{
	CLASS_DEFINITION_DERIVED(DepthBuffer_OpenGL, DepthBuffer)
	public:
		DepthBuffer_OpenGL(bool isRenderTexture = true)
            : DepthBuffer()
		{ 
			this->init(isRenderTexture);
		}

		DepthBuffer_OpenGL(const Dimensions& dimm, DepthComponent comp, bool isRenderTexture = true) :
			DepthBuffer(dimm, comp) 
		{
			this->init(isRenderTexture);
		}
		virtual ~DepthBuffer_OpenGL();

		GLuint getDepthHandle() const { return this->handle; }
		GLuint getDepthTextureHandle() const;
			
		virtual void bind(uint32 stage);

	private:

		void init(bool isRenderTexture);
		GLuint handle;

	};
}
