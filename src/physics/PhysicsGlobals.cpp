#include "PCH.h"

#include "PhysicsGlobals.h"

namespace cs
{
	const float32 PhysicsConst::kBox2DScale = 10.0f;
	const float32 PhysicsConst::kInvBox2DScale = 1.0f / PhysicsConst::kBox2DScale;

	const float32 PhysicsConst::kBox2DPaddingValue = 0.01f;
	const float32 PhysicsConst::kHalfScale = 0.5f;
}