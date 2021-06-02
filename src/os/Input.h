#pragma once

#include "math/GLM.h"

namespace cs
{
	enum ClickInput
	{
		ClickNone = -1,
		ClickPrimary,
		ClickSecondary,
		ClickZoom,
		ClickMAX
	};

	enum TouchState
	{
		TouchNone = -1,
		TouchUp,
		TouchDown,
		TouchMove,
		TouchPressed,
		TouchReleased,
		TouchWheel,

		// Swipes start 
		TouchSwipeFirst,
		TouchSwipeLeft,
		TouchSwipeRight,
		TouchSwipeUp,
		TouchSwipeDown,
		TouchMAX
	};

	struct ClickParams
	{
		ClickParams()
            : state(TouchNone)
            , zoom(0.0f) { }

		TouchState state;
		vec2 position;
		vec2 startPosition;
		vec2 lastPosition;
		vec2 velocity;
		vec2 screen_dimm;
		float32 zoom;
	};
}
