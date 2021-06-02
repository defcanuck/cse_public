#pragma once


#include "gfx/FrameBuffer.h"
#include "gfx/gl/OpenGL.h"

namespace cs
{
	CLASS_DEFINITION_DERIVED(FrameBuffer_OpenGL, FrameBuffer)
	public:
		FrameBuffer_OpenGL() :
			frameHandle(0) { 
			this->init();
		}

		virtual void attachColorBuffer(TextureResourcePtr& tex);
		virtual void attachDepthBuffer(DepthBufferPtr& depth);

		virtual bool check();

	protected:

        virtual void bind();
        virtual void unbind();

		void init();

		GLuint frameHandle;

	};
}
