#include "PCH.h"
#include "gfx/Color.h"

namespace cs
{

#if defined(CS_IOS)
    template<>
    const uchar ColorChannel<uchar>::max_value() { return 255; }
    
    template<>
    const uint32 ColorChannel<uint32>::max_value() { return 255; }
    
    template<>
    const float32 ColorChannel<float32>::max_value() { return 1.0f; }

#endif

}
