#include "PCH.h"

#include "ui/UISpan.h"
#include "gfx/RenderInterface.h"

namespace cs
{

	float32 UIDimension::getContentScale()
	{
		return RenderInterface::getInstance()->getContentScale();
	}
}