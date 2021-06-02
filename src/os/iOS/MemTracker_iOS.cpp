#include "os/MemTracker.h"

#include "Platform_iOS.h"


namespace cs
{
    void MemTracker::getMemoryStats(MemStats& stats)
    {
       stats.usedMem = GetSizeInMbOS();
    }
}
