#include "PCH.h"

#include "os/windows/InputField_Windows.h"

#include "SDL.h"

namespace cs
{

	void InputField_Windows::beginInputImpl()
	{
		SDL_StartTextInput();
	}

	void InputField_Windows::endInputImpl()
	{
		SDL_StopTextInput();
	}
}