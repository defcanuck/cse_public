#pragma once

#include <SDL.h>
//#include <SDL_image.h>

namespace cs
{
	namespace sdl
	{
		inline void consumeEvent()
		{
			SDL_Event e;
			while (SDL_PollEvent(&e) != 0);
		}
	}
}
