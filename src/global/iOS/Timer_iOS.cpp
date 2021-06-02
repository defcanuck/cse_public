#include "PCH.h"

#include "global/Timer.h"
#include <SDL.h>

namespace cs
{
	HighPrecisionTimer::HighPrecisionTimer()
	{
        this->frequency = new char[sizeof(uint64)];
        this->start = new char[sizeof(uint64)];
        
        this->reset();
	}

	HighPrecisionTimer::~HighPrecisionTimer()
	{
        delete this->frequency;
        delete this->start;
	}

	void HighPrecisionTimer::reset()
	{
        uint64* startPtr = reinterpret_cast<uint64*>(this->start);
        uint64* frequencyPtr = reinterpret_cast<uint64*>(this->frequency);
        *startPtr = SDL_GetPerformanceCounter();
        *frequencyPtr = SDL_GetPerformanceFrequency();
	}

	double HighPrecisionTimer::getElapsed() const
	{
        uint64* startPtr = reinterpret_cast<uint64*>(this->start);
        uint64* frequencyPtr = reinterpret_cast<uint64*>(this->frequency);
        uint64 now = SDL_GetPerformanceCounter();
        uint64 diff = now - *startPtr;
        long double timeVal = diff / (long double) *frequencyPtr;
		return double(timeVal);
	}
}
