#include "PCH.h"

#include "os/iOS/InputField_iOS.h"

#include "SDL.h"

namespace cs
{

	void InputField_iOS::beginInputImpl()
	{
		SDL_StartTextInput();
	}

	void InputField_iOS::endInputImpl()
	{
		SDL_StopTextInput();
	}
}
