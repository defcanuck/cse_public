#include "PCH.h"

#include "global/Allocator.h"

namespace cs
{
	namespace AllocatorInternal
	{

		const size_t kDefaultAlignment = 16;

		void* allocate(size_t num_bytes, size_t align)
		{
			return ::operator new(num_bytes);
		}

		void deallocate(void* ptr, size_t num_bytes)
		{
			::operator delete(ptr);
		}
	}
}