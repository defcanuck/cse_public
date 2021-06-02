#pragma once

#include "gfx/DepthBuffer.h"

namespace cs
{
	CLASS_DEFINITION_DERIVED(DepthBuffer_Metal, DepthBuffer)
	public:
		DepthBuffer_Metal(bool isRenderTexture = true)
            : DepthBuffer()
		{ 
			this->init();
		}

		DepthBuffer_Metal(const Dimensions& dimm, DepthComponent comp, bool isRenderTexture = true) :
			DepthBuffer(dimm, comp) 
		{
			this->init();
		}
		virtual ~DepthBuffer_Metal();
			
		virtual void bind(uint32 stage);

	private:

		void init();

	};
}
