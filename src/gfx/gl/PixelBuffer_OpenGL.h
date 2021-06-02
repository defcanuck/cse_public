#pragma once

#include "gfx/PixelBuffer.h"
#include "gfx/gl/OpenGL.h"

namespace cs
{
	CLASS_DEFINITION_DERIVED(PixelBuffer_OpenGL, PixelBuffer)

	public:
		PixelBuffer_OpenGL(const Dimensions& dimm, TextureChannels c) :
			PixelBuffer(dimm, c),
			pbo(0)
		{
			this->init();
		}

		virtual ~PixelBuffer_OpenGL()
		{
			this->free();
		}

		virtual void update(TextureResourcePtr& ptr);
		virtual void write(uchar* bytes);
		virtual void write(TextureUpdatePtr& ptr);

	private:

		void init();
		void free();

		GLuint pbo;
	

	};
}