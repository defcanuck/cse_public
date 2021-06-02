#include "PCH.h"

#include "global/Timer.h"
#include "sdl/SDL_Helpers.h"

namespace cs
{
	Timer::Tick Timer::getCurrentTick()
	{
		return SDL_GetTicks();
	}

	void Timer::start()
	{
		this->started = true;
		this->paused = false;
		this->startTicks = SDL_GetTicks();
		this->pausedTicks = 0;
	}

	void Timer::stop()
	{
		this->started = false;
		this->paused = false;
		this->startTicks = 0;
		this->pausedTicks = 0;
	}

	void Timer::pause()
	{
		if (this->started && !this->paused)
		{
			this->paused = true;

			this->pausedTicks = SDL_GetTicks() - this->startTicks;
			this->startTicks = 0;
		}
	}

	void Timer::unpause()
	{
		if (this->started && this->paused)
		{
			this->paused = false;
			this->startTicks = SDL_GetTicks() - this->pausedTicks;
			this->pausedTicks = 0;
		}
	}

	uint32 Timer::getTicks() const
	{
		uint32 time = 0;
		if (this->started) {
			if (this->paused) {
				time = this->pausedTicks;
			} else {
				time = SDL_GetTicks() - this->startTicks;
			}
		}

		return time;
	}

	bool Timer::isPaused() const
	{
		return this->paused;
	}

	bool Timer::isStarted() const
	{
		return this->started;
	}

	ScopedTimer::ScopedTimer(uint32* dst)
		: updateTime(dst)
		, ticks(0)
	{
		this->ticks = SDL_GetTicks();
	}

	void ScopedTimer::stop()
	{
		*this->updateTime = SDL_GetTicks() - this->ticks;
	}

	ScopedTimer::~ScopedTimer()
	{
		this->stop();
	}

	ScopedAccumTimer::~ScopedAccumTimer()
	{
		this->stop();
	}

	void ScopedAccumTimer::stop()
	{
		*this->updateTime += SDL_GetTicks() - this->ticks;
	}
}
