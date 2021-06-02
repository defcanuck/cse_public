#include "PCH.h"

#include "gfx/gl/PixelBuffer_OpenGL.h"

namespace cs
{
	void PixelBuffer_OpenGL::init()
	{
		GL_CHECK(glGenBuffers(1, &this->pbo));
	}

	void PixelBuffer_OpenGL::free()
	{
		GL_CHECK(glDeleteBuffers(1, &this->pbo));
	}

	void PixelBuffer_OpenGL::update(TextureResourcePtr& ptr)
	{

#if !defined(CS_IOS)

        ptr->bind(0);
		GL_CHECK(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, this->pbo));

		// copy pixels from PBO to texture object
		// Use offset instead of ponter.
		GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->width, this->height,
			kTextureConvertDst[this->channels], GL_UNSIGNED_BYTE, 0));
		
		GL_CHECK(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));

#endif

    }

	void PixelBuffer_OpenGL::write(uchar* bytes)
	{
#if !defined(CS_IOS)
		GL_CHECK(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, this->pbo));
		
		size_t sz = width * height * getTextureSize(this->channels);
		GL_CHECK(glBufferData(GL_PIXEL_UNPACK_BUFFER, sz, 0, GL_STREAM_DRAW));
		
		GLubyte* ptr = nullptr;
		GL_CHECK(ptr = (GLubyte*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
		if (ptr)
		{
			// update data directly on the mapped buffer
			memcpy(ptr, bytes, sz);
			GL_CHECK(glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER)); // release the mapped buffer
		}
		GL_CHECK(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
#else
        assert(false);
#endif
	}

	void PixelBuffer_OpenGL::write(TextureUpdatePtr& func)
	{
#if !defined(CS_IOS)
		GL_CHECK(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, this->pbo));

		size_t sz = width * height * getTextureSize(this->channels);
		GL_CHECK(glBufferData(GL_PIXEL_UNPACK_BUFFER, sz, 0, GL_STREAM_DRAW));

		GLubyte* ptr = nullptr;
		GL_CHECK(ptr = (GLubyte*) glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
		if (ptr)
		{
			// update data directly on the mapped buffer
			func(ptr, Dimensions(this->width, this->height), this->channels);
			GL_CHECK(glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER)); // release the mapped buffer
		}
		GL_CHECK(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
#else
        assert(false);
#endif
	}

}
