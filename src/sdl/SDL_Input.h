#pragma once

#include <SDL.h>

#include "global/Values.h"

namespace cs
{
	struct csui
	{
		static bool processInputWindows(SDL_Event& e, int32 width, int32 height);
        static bool processInputTouch(SDL_Event& e, int32 width, int32 height);
	};
}
