#pragma once

#include "scripting/LuaMacro.h"

#include "animation/Animator.h"

namespace cs
{
	PROTO_LUA_CLASS(AnimationType);
	PROTO_LUA_CLASS(AnimationBase);

	PROTO_LUA_CLASS(FloatAnimation);
	PROTO_LUA_CLASS(Vec2Animation);
	PROTO_LUA_CLASS(Vec3Animation);
	PROTO_LUA_CLASS(ColorBAnimation);
	PROTO_LUA_CLASS(ColorFAnimation);

	PROTO_LUA_CLASS(FloatLerpAnimator);
	PROTO_LUA_CLASS(Vec2LerpAnimator);
	PROTO_LUA_CLASS(Vec3LerpAnimator);
	PROTO_LUA_CLASS(ColorBLerpAnimator);
	PROTO_LUA_CLASS(ColorFLerpAnimator);

}
