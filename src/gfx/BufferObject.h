#pragma once

#include "ClassDef.h"
#include "gfx/Types.h"
#include "gfx/VertexDeclaration.h"

namespace cs
{
	CLASS_DEFINITION(BufferObject)
	public:

		BufferObject(BufferType t);
		virtual ~BufferObject() { }

		virtual void alloc(size_t sz, const void* data, BufferStorage st = BufferStorageStatic) = 0;
        virtual void dealloc() = 0;
    
		BufferType getType() const { return this->type; }
		BufferStorage getStorage() const { return this->storage; }

		virtual void* lock(BufferAccess access = BufferAccessRead) = 0;
		virtual void unlock() = 0;
		virtual size_t getSize() const { return size; }
		virtual bool stateEqual(const BufferObjectPtr& rhs) = 0;

		virtual void resize(size_t sz) = 0;

		static bool areBuffersEqual(const BufferObjectPtr& lhs, const BufferObjectPtr& rhs);
		virtual void bindAttributes(VertexDeclaration& decl) { }

	protected:

		friend class RenderInterface;

		virtual void bindImpl() = 0;
		
		BufferType type;
		size_t size;
		BufferStorage storage;
	
	};
}
