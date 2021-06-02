#pragma once

#include "gfx/BufferObject.h"

namespace cs
{

	CLASS_DEFINITION_DERIVED(BufferObject_Metal, BufferObject)
	public:

		BufferObject_Metal(BufferType t)
			: BufferObject(t)
			, buffer(0)
		{
			this->init();
		}

		virtual ~BufferObject_Metal()
		{ 
			this->free();
		}

		virtual void alloc(size_t sz, const void* data, BufferStorage st = BufferStorageDynamic);
        virtual void dealloc() { this->free(); }
    
		virtual void* lock(BufferAccess access = BufferAccessRead);
		virtual void unlock();
		virtual bool stateEqual(const BufferObjectPtr& rhs);
		virtual void resize(size_t sz);
		
        void* getBufferObject() { return this->buffer; }
    
	protected:

		virtual void bindImpl();

		void init();
		void free();

        void* buffer;
    
    };
}
