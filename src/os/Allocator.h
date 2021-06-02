
#pragma once

#include <stdlib.h>

namespace cs
{
#if defined(CS_OSX) || defined(CS_IOS) || defined(CS_IPHONE)
    inline void* alloc_aligned(size_t size, size_t alignment)
    {
        void* voidptr = 0;
        // posix_memalign(&voidptr, size, alignment);
        voidptr = malloc(size);
        return voidptr;
    }
    
    inline void alloc_free(void* voidptr)
    {
        free(voidptr);
    }
    
#endif
    
#if defined(CS_WIN)
    
    inline void* alloc_aligned(size_t size, size_t alignment)
    {
		return _aligned_malloc(size, alignment);
    }
    
    inline void alloc_free(void* voidptr)
    {
		_aligned_free(voidptr);
    }
#endif
    
}
