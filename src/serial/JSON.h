#pragma once

#include "global/Values.h"
#include "gason.h"

#include <ostream>

namespace cs
{
	namespace json
	{
		extern const char* kJsonTypes[];

		void debug(JsonValue o, std::ostream& oss, uint32 depth = 0);
	}
}