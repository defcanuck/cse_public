#pragma once

#include "ClassDef.h"
#include "global/Values.h"
#include "math/Rect.h"
#include "global/Resource.h"
#include "gfx/Types.h"

namespace cs
{
	CLASS_DEFINITION(TextureResource)
	
	public:

		TextureResource();
		TextureResource(const Dimensions& dimm, TextureChannels c, uchar* data = nullptr, TextureUsage = TextureUsageShaderRead);

		virtual ~TextureResource();

		virtual void bind(uint32 stage, bool wrapU = false, bool wrapV = false) = 0;
        virtual uint32 getTextureId() const { return 0; }
        virtual uint32* getTextureIdPtr() { return nullptr; }

        TextureChannels getChannels() const { return this->channels; }
    
		uint32 getRawWidth() const { return this->width; }
		uint32 getRawHeight() const { return this->height; }

		uint32 getRealWidth() const { return static_cast<uint32>(this->width * this->fitWidth); }
		uint32 getRealHeight() const { return static_cast<uint32>(this->height * this->fitHeight); }

		void adjustRect(RectF& rect);

	protected:
    
        void fitPow2();

		uint32 width;
		uint32 height;
		TextureChannels channels;
		uchar* bytes;
        TextureUsage usage;
		
		float32 fitWidth;
		float32 fitHeight;

	};
}
