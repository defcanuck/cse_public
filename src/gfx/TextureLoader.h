#pragma once

#include "global/Values.h"
#include "gfx/Types.h"

#include <string>

namespace cs
{
	namespace tex
	{
		uchar* loadImage(const std::string& filePath, uint32& width, uint32& height, TextureChannels& channels);
	}
}