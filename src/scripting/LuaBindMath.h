#pragma once

#include "scripting/LuaMacro.h"

#include "math/GLM.h"
#include "math/Rect.h"
#include "math/Ray.h"
#include "math/Plane.h"

namespace cs
{
	PROTO_LUA_CLASS(vec2);
	PROTO_LUA_CLASS(vec3);
	PROTO_LUA_CLASS(vec4);
	PROTO_LUA_CLASS(quat);
	PROTO_LUA_CLASS(PointF);
	PROTO_LUA_CLASS(PointI);
	PROTO_LUA_CLASS(RectF);
	PROTO_LUA_CLASS(RectI);
	PROTO_LUA_CLASS(Ray);
	PROTO_LUA_CLASS(Plane);
}