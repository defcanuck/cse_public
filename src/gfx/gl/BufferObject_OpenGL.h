#pragma once

#include "gfx/BufferObject.h"
#include "gfx/gl/OpenGL.h"

namespace cs
{

	CLASS_DEFINITION_DERIVED(BufferObject_OpenGL, BufferObject)
	public:

		BufferObject_OpenGL(BufferType t)
			: BufferObject(t)
			, handle(0)
		{
			this->lockData[0] = nullptr;
			this->lockData[1] = nullptr;
			this->lockIndex = 0;
			this->init();
		}

		virtual ~BufferObject_OpenGL()
		{ 
			this->free();
		}

		virtual void alloc(size_t sz, const void* data, BufferStorage st = BufferStorageDynamic);
        virtual void dealloc() { this->free(); }
    
		static void clear(BufferType type);

		virtual void* lock(BufferAccess access = BufferAccessRead);
		virtual void unlock();
		virtual bool stateEqual(const BufferObjectPtr& rhs);
		virtual void resize(size_t sz);

		uint32 getHandle() const { return this->handle; }
		
	protected:

		virtual void bindImpl();

		void init();
		void free();

		uint32 handle;
		char* lockData[2];
		uint8 lockIndex;
	};
}
