
#pragma once

#include "math/GLM.h"
#include "global/Values.h"

namespace cs
{
	bool raySphereIntersect(
		const vec3& raydir,
		const vec3& rayorg,
		const vec3& pos,
		const float& rad,
		vec3& hitpoint,
		float& distance,
		vec3& normal);

	bool rayIntersectsTriangle(
		const vec3& raydir,
		const vec3& rayorg,
		vec3 &v0,
		vec3 &v1,
		vec3 &v2,
		float32 &u,
		float32 &v,
		float32 &t);
}