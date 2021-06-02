#include "PCH.h"

#include <windows.h>
#include "global/Timer.h"

namespace cs
{
	HighPrecisionTimer::HighPrecisionTimer()
	{
		this->frequency = new char[sizeof(LARGE_INTEGER)];
		if (::QueryPerformanceFrequency((LARGE_INTEGER*) frequency) == FALSE)
			throw "timer error!";

		this->start = new char[sizeof(LARGE_INTEGER)];
		this->reset();
	}

	HighPrecisionTimer::~HighPrecisionTimer()
	{
		delete[] this->start;
		delete[] this->frequency;
	}

	void HighPrecisionTimer::reset()
	{
		if (::QueryPerformanceCounter((LARGE_INTEGER*)start) == FALSE)
			throw "timer error!";
	}

	double HighPrecisionTimer::getElapsed() const
	{
		LARGE_INTEGER end;
		if (::QueryPerformanceCounter(&end) == FALSE)
			throw "foo";

		return static_cast<double>(end.QuadPart - reinterpret_cast<LARGE_INTEGER*>(this->start)->QuadPart) / reinterpret_cast<LARGE_INTEGER*>(this->frequency)->QuadPart;
	}
}