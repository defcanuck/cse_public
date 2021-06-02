#include "PCH.h"

#include "sdl/SDL_Input.h"

#include <SDL.h>

#include "os/Input.h"
#include "os/InputManager.h"

namespace cs
{
	static float32 mouseScroll = 0.5f;

	cs::ClickInput getButton(SDL_Event& e)
	{
		switch (e.button.button)
		{
		case SDL_BUTTON_LEFT:
			return cs::ClickPrimary;
		case SDL_BUTTON_RIGHT:
			return cs::ClickSecondary;
		case SDL_BUTTON_MIDDLE:
			return cs::ClickZoom;
		}

		return cs::ClickNone;
	}

	bool csui::processInputWindows(SDL_Event& e, int32 width, int32 height)
	{
		int32 x = 0, y = 0;
		float32 sx = 0.0f, sy = 0.0f;

		switch (e.type)
		{
        	case SDL_QUIT:
				return true;

			case SDL_KEYDOWN:
			case SDL_KEYUP:
			{
				uint32 flags = 0;
				uint32 key = e.key.keysym.sym & ~SDLK_SCANCODE_MASK;

				if ((SDL_GetModState() & KMOD_SHIFT) != 0)
					flags |= InputModifierShift;

				if ((SDL_GetModState() & KMOD_CTRL) != 0)
					flags |= InputModiferControl;

				if ((SDL_GetModState() & KMOD_ALT) != 0)
					flags |= InputModifierAlt;
				
				if (e.type == SDL_KEYDOWN)
				{

					switch (e.key.keysym.sym)
					{
					case SDLK_RETURN:
						cs::InputManager::getInstance()->endEditingText();
						break;
					case SDLK_BACKSPACE:
						cs::InputManager::getInstance()->onBackspace();
						break;
					}
					cs::InputManager::getInstance()->onKeyDown(key, flags);
					break;
				}

				cs::InputManager::getInstance()->onKeyUp(key, flags);

			} break;
                
			case SDL_MOUSEMOTION:
			{

				x = e.motion.x;
				y = height - e.motion.y;
				sx = x / float32(width);
				sy = y / float32(height);

				cs::ClickInput input = getButton(e);
				cs::InputManager::getInstance()->onInputMove(input, vec2(sx, sy));
			} break;
           
			case SDL_MOUSEBUTTONUP:
			{

				cs::ClickInput input = getButton(e);
				cs::InputManager::getInstance()->onInputUp(input);
			} break;
                
            case SDL_MOUSEBUTTONDOWN:
            {
                
                x = e.motion.x;
                y = height - e.motion.y;
                sx = x / float32(width);
                sy = y / float32(height);
                
                cs::ClickInput input = getButton(e);
                cs::InputManager::getInstance()->onInputDown(input, vec2(sx, sy));
            } break;
                
			case SDL_TEXTINPUT:
			{

				cs::InputManager::getInstance()->onTextEdit(e.text.text);

			} break;

			case SDL_TEXTEDITING:

				//cs::log::print(cs::LogInfo, "SDL_TEXTEDITING");
				break;

			case SDL_MOUSEWHEEL:
			{
				mouseScroll = cs::clamp(0.0f, 1.0f, mouseScroll + (float32(e.wheel.y) * 1.0f / 30.0f));
				cs::ClickInput input = getButton(e);
				cs::InputManager::getInstance()->onInputZoom(input, mouseScroll + 0.5f);
			} break;
		}

		return false;
	}
    
    bool csui::processInputTouch(SDL_Event& e, int32 width, int32 height)
    {
        switch (e.type)
        {
            case SDL_QUIT:
                return true;
            case SDL_FINGERDOWN:
            {
                float32 x = e.tfinger.x;// * width / float32(t->dx);
                float32 y = 1.0f - e.tfinger.y;// * height / float32(t->dy);
                cs::log::info("Finger Down: ", x, ",", y);
                cs::InputManager::getInstance()->onInputDown(ClickPrimary, vec2(x, y));
            } break;
                
            case SDL_FINGERUP:
            {
                cs::log::info("Finger Up");
                cs::InputManager::getInstance()->onInputUp(ClickPrimary);
            } break;
                
            case SDL_FINGERMOTION:
            {
                float32 x = e.tfinger.x;// * width / float32(t->dx);
                float32 y = 1.0f - e.tfinger.y;// * height / float32(t->dy);
                cs::log::info("Finger Motion: ", x, ",", y);
                cs::InputManager::getInstance()->onInputMove(ClickPrimary, vec2(x, y));
            } break;
        }
        
        return false;
    }

}
