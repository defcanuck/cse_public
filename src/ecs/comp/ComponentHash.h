#pragma once

#include <typeindex>
#include "global/Values.h"

namespace cs
{
	extern std::unordered_map<size_t, uint32> gComponentHashMap;

	void initComponentHash();
	uint32 getComponentHash(const std::type_index& index);
	uint32 getComponentHash(size_t hash_index);

}