#include "PCH.h"

#include "gfx/metal/BufferObject_Metal.h"
#include "gfx/RenderInterface.h"

#include "MTLRenderInterface.h"

namespace cs
{
	void BufferObject_Metal::init()
	{

    }

	void BufferObject_Metal::free()
	{
        MTLFreeBuffer(this->buffer);
    }

	void BufferObject_Metal::bindImpl()
	{
	
    }

	void BufferObject_Metal::alloc(size_t sz, const void* data, BufferStorage storage)
	{
        if (storage == BufferStorageDynamic)
        {
            if (!this->buffer)
            {
                this->buffer = MTLCreateBuffer((unsigned) sz, data, (unsigned) storage);
            }
            else
            {
                this->buffer = MTLResizeBuffer(this->buffer, (unsigned) sz, data, (unsigned) storage);
            }
        }
        else
        {
            this->buffer = MTLCreateBuffer((unsigned) sz, data, (unsigned) storage);
        }
        
        if (this->buffer)
        {
            this->size = sz;
            this->storage = storage;
        }
    }

	void BufferObject_Metal::resize(size_t sz)
	{
        this->alloc(sz, nullptr, this->storage);
    }

	void* BufferObject_Metal::lock(BufferAccess access)
	{
		if (this->buffer)
        {
            return MTLGetBufferContents(this->buffer);
        }
        return nullptr;
	}

	void BufferObject_Metal::unlock()
	{

    }

	bool BufferObject_Metal::stateEqual(const BufferObjectPtr& rhs)
	{
        return false;
    }


}
