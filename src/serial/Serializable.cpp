#include "PCH.h"

#include "serial/Serializable.h"

namespace cs
{
	const LoadFlagMask kLoadFlagMaskAll = LoadFlagMask((0x1 << size_t(LoadFlagsMAX)) - 1);
	const LoadFlagMask kLoadFlagMaskEmpty = LoadFlagMask(0);
}