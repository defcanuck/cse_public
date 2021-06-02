#include "os/MemTracker.h"

#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#include <stdio.h>

namespace cs
{
	void MemTracker::getMemoryStats(MemStats& stats)
	{
		struct local
		{
			static void GetMemoryInfo(DWORD processID, MemStats& stats)
			{
				HANDLE hProcess;
				PROCESS_MEMORY_COUNTERS pmc;

				hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
					PROCESS_VM_READ,
					FALSE, processID);
				if (NULL == hProcess)
					return;

				if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
				{
					/*
					printf("\tPageFaultCount: %d\n", pmc.PageFaultCount);
					printf("\tPeakWorkingSetSize: %d\n", pmc.PeakWorkingSetSize);
					printf("\tWorkingSetSize: %d\n", pmc.WorkingSetSize);
					printf("\tQuotaPeakPagedPoolUsage: %d\n", pmc.QuotaPeakPagedPoolUsage);
					printf("\tQuotaPagedPoolUsage: %d\n", pmc.QuotaPagedPoolUsage);
					printf("\tQuotaPeakNonPagedPoolUsage: %d\n", pmc.QuotaPeakNonPagedPoolUsage);
					printf("\tQuotaNonPagedPoolUsage: %d\n", pmc.QuotaNonPagedPoolUsage);
					printf("\tPagefileUsage: %d\n", pmc.PagefileUsage);
					printf("\tPeakPagefileUsage: %d\n", pmc.PeakPagefileUsage);
					*/
					stats.usedMem = pmc.WorkingSetSize;
				}

				CloseHandle(hProcess);
			}
		};

		local::GetMemoryInfo(GetCurrentProcessId(), stats);		
	}
}