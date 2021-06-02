#pragma once

#include "ClassDef.h"

#include "gfx/TypesGlobal.h"

namespace cs
{
	struct EngineStats
	{
		static void init();
		static void incrementStat(StatType type);
		static void decrementStat(StatType type);
		static void incrementStatBy(StatType type, int32 sz);
		static int32 getStat(StatType type);
		static const char* getTag(StatType type);
		static bool isSize(StatType type);
	};
}
