#include "PCH.h"

#include "global/Stats.h"

namespace cs
{
	volatile uint32 gStatKeeper[StatTypeMAX];

	void EngineStats::init()
	{
		memset((void*) gStatKeeper, 0, StatTypeMAX * sizeof(uint32));
	}

	void EngineStats::incrementStat(StatType type)
	{
		++gStatKeeper[type];
	}


	void EngineStats::incrementStatBy(StatType type, int32 sz)
	{
		gStatKeeper[type] += sz;
	}

	void EngineStats::decrementStat(StatType type)
	{
		--gStatKeeper[type];
	}
	
	int32 EngineStats::getStat(StatType type)
	{
		return gStatKeeper[type];
	}

	const char* EngineStats::getTag(StatType type)
	{
		const char* kStatTag[] =
		{
			"UI Element",
			"UI Document",
			"Entity",
			"Mesh",
			"Texture",
			"Particle Heap",
			"Buffer Size",
			"Texture Size"
		};

		return kStatTag[type];
	}

	bool EngineStats::isSize(StatType type)
	{
		if (type == StatTypeBufferSize ||
			type == StatTypeTextureSize)
		{
			return true;
		}
		return false;
	}

}