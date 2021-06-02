#pragma once

namespace cs
{
	struct MemStats
	{
		int usedMem;
	};

	namespace MemTracker
	{
		void getMemoryStats(MemStats& stats);
	}
}
