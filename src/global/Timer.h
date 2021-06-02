#pragma once

#include "global/Values.h"
#include "global/Singleton.h"

#include <unordered_map>

namespace cs
{
	class Timer
	{
	public:
		Timer() :
			startTicks(0),
			pausedTicks(0),
			paused(false),
			started(false) { }

		void start();
		void stop();
		void pause();
		void unpause();

		uint32 getTicks() const;
		bool isStarted() const;
		bool isPaused() const;

		float32 getElapsed() const { return this->getTicks() / 1000.0f; }

		typedef uint32 Tick;
		static Tick getCurrentTick();

	private:
		
		uint32 startTicks;
		uint32 pausedTicks;

		bool paused;
		bool started;
	};

	class ScopedTimer
	{
	public:

		ScopedTimer(uint32* dst);
		~ScopedTimer(); // intentionally non-virtual

	protected:

		virtual void stop();

		uint32* updateTime;
		uint32 ticks;
		
	};

	class ScopedAccumTimer : public ScopedTimer
	{
	public:
		ScopedAccumTimer(uint32* dst) : ScopedTimer(dst) { }
		~ScopedAccumTimer();

	protected:
		virtual void stop();
	};

	class HighPrecisionTimer
	{
	public:
        
		HighPrecisionTimer();
		~HighPrecisionTimer();

		void reset();
		double getElapsed() const;

	private:

		char* start;
		char* frequency;
        
	};

	class TimerCache : public Singleton<TimerCache>
	{
		typedef std::unordered_map<std::string, float32> TimeSampleMap;

	public:

		void reset()
		{
			for (auto& it : this->timeSamples)
			{
				it.second = 0.0f;
			}
		}

		void accumulate(std::string name, float32 accum)
		{
			TimeSampleMap::iterator it = this->timeSamples.find(name);
			if (it == this->timeSamples.end())
			{
				this->timeSamples[name] = accum;
				return;
			}

			it->second += accum;
		}

	private:

		TimeSampleMap timeSamples;

	};
}
