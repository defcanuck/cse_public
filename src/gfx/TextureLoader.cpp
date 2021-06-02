#include "PCH.h"

#include "gfx/TextureLoader.h"
#include "os/LogManager.h"

#include <SDL.h>
#include <SDL_image.h>

namespace cs
{
	namespace tex
	{
		uchar* loadImage(const std::string& filePath, uint32& width, uint32& height, TextureChannels& channels)
		{
			SDL_Surface* surface = IMG_Load(filePath.c_str());
			if (!surface) 
			{
				log::print(LogError, "Failed to load: ", filePath);
				return nullptr;
			}

			channels = TextureRGB;
			uint32 sz = 3;
			if (surface->format->BytesPerPixel == 4) {
				channels = TextureRGBA;
				sz = 4;
			}
            
#if defined(CS_METAL)
            switch (channels)
            {
                case TextureRGB: channels = TextureBGR; break;
                case TextureRGBA: channels = TextureBGRA; break;
            }
#endif
		
			width = surface->w;
			height = surface->h;
			uint32 bufferSize = width * height * sz;
			uchar* data = new uchar[bufferSize];
			memcpy(data, surface->pixels, bufferSize);
			SDL_FreeSurface(surface);

			return data;
		}
	}
}
