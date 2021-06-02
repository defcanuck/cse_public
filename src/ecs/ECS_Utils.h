#pragma once


#include "scene/Scene.h"
#include "scene/Camera.h"

namespace cs
{
	struct SystemUpdateParams
	{
		float32 animationDt;
		float32 updateDt;
		CameraPtr camera;
		bool active;
	};
}