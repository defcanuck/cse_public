#include "PCH.h"

#include "gfx/BufferObject.h"

namespace cs
{

	BufferObject::BufferObject(BufferType t) :
		type(t),
		size(0),
		storage(BufferStorageNone)
	{ 
	
	}

	bool BufferObject::areBuffersEqual(const BufferObjectPtr& lhs, const BufferObjectPtr& rhs)
	{
		if ((!lhs && rhs) || (rhs && !lhs))
			return true;

		return lhs->stateEqual(rhs);
	}
}