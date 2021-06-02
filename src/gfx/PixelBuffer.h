#pragma once

#include "ClassDef.h"
#include "gfx/Types.h"
#include "gfx/TextureResource.h"

namespace cs
{
	CLASS_DEFINITION(PixelBuffer)

	public:

		typedef std::function<void(uchar*, const Dimensions&, TextureChannels)> TextureUpdatePtr;

		PixelBuffer(const Dimensions& dimm, TextureChannels c) :
			width(dimm.w),
			height(dimm.h),
			channels(c)
		{ }

		virtual ~PixelBuffer()
		{ }

		virtual void update(TextureResourcePtr& ptr) = 0;
		virtual void write(uchar* bytes) = 0;
		virtual void write(TextureUpdatePtr& ptr) = 0;

	protected:

		uint32 width;
		uint32 height;
		TextureChannels channels;
	};
}