#include "PCH.h"

#include "gfx/gl/BufferObject_OpenGL.h"
#include "gfx/RenderInterface.h"
#include "global/Stats.h"

#if defined(CS_WINDOWS)
	#define GL_MAP_BUFFER 1
#endif

namespace cs
{
	GLuint kBufferStorageGL[] =
	{
		GL_STATIC_DRAW,
		GL_DYNAMIC_DRAW,
		GL_STREAM_DRAW
	};

	GLuint kBufferTypeGL[] = 
	{
		GL_ARRAY_BUFFER,
		GL_ELEMENT_ARRAY_BUFFER
	};

#if defined(CS_IOS)
    GLuint kBufferAccessGL[] =
    {
        0,
        GL_WRITE_ONLY,
        0
    };
#else
    GLuint kBufferAccessGL[] =
    {
        GL_READ_ONLY,
        GL_WRITE_ONLY,
        GL_READ_WRITE
    };
#endif

	void BufferObject_OpenGL::init()
	{
		GL_CHECK(glGenBuffers(1, &this->handle));
	}

	void BufferObject_OpenGL::free()
	{
        GL_CHECK(glDeleteBuffers(1, &this->handle));
		EngineStats::incrementStatBy(StatTypeBufferSize, -((int32) this->size));
	}

	void BufferObject_OpenGL::bindImpl()
	{
		GLenum bufferType = kBufferTypeGL[this->type];
		GL_CHECK(glBindBuffer(bufferType, this->handle));
	}

	void BufferObject_OpenGL::clear(BufferType type)
	{
		RenderInterface::getInstance()->clearBuffer(type);
	}

	void BufferObject_OpenGL::alloc(size_t sz, const void* data, BufferStorage st)
	{
		this->size = sz;
		this->storage = st;
		GLenum bufferType = kBufferTypeGL[this->type];
		GL_CHECK(glBindBuffer(bufferType, this->handle));
		GL_CHECK(glBufferData(bufferType, sz, data, kBufferStorageGL[st]));
		EngineStats::incrementStatBy(StatTypeBufferSize, this->size);
	}

	void BufferObject_OpenGL::resize(size_t sz)
	{
		// free the old buffer
		this->free();
		for (int32 i = 0; i < 2; ++i)
		{
			if (this->lockData[i])
			{
				delete[] this->lockData[i];
				this->lockData[i] = nullptr;
			}
		}
		this->lockIndex = 0;

		// init a brand new buffer
		this->init();
		this->size = sz;
		this->alloc(sz, nullptr, this->storage);
	}

	void* BufferObject_OpenGL::lock(BufferAccess access)
	{
		
#if defined(GL_MAP_BUFFER)

        void* ptr = nullptr;
		this->bindImpl();
		GLenum bufferType = kBufferTypeGL[this->type];  
		GL_CHECK(ptr = glMapBuffer(bufferType, kBufferAccessGL[access]));
		return ptr;
#else
        if (!this->lockData[this->lockIndex])
        {
            this->lockData[this->lockIndex] = new char[this->size];
        }
        return (void*) this->lockData[this->lockIndex];
#endif
	}

	void BufferObject_OpenGL::unlock()
	{
#if defined(GL_MAP_BUFFER)
		GLenum bufferType = kBufferTypeGL[this->type];
		GL_CHECK(glUnmapBuffer(bufferType));
#else
		assert(this->lockData);
		this->alloc(this->size, this->lockData[this->lockIndex], this->storage);
		this->lockIndex = (this->lockIndex + 1) % 2;
#endif
	}

	bool BufferObject_OpenGL::stateEqual(const BufferObjectPtr& rhs)
	{
		BufferObject_OpenGLPtr rhs_ptr = std::static_pointer_cast<BufferObject_OpenGL>(rhs);
		if (this->getHandle() != rhs_ptr->getHandle())
			return false;
		
		return true;
	}


}
