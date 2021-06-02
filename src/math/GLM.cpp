#include "PCH.h"

#include "math/GLM.h"

const vec3 kZero3 = vec3(0.0f, 0.0f, 0.0f);
const vec3 kOne3 = vec3(1.0f, 1.0f, 1.0f);

const vec3 kDefalutXAxis = vec3(1.0f, 0.0f, 0.0f);
const vec3 kDefalutYAxis = vec3(0.0f, 1.0f, 0.0f);
const vec3 kDefalutZAxis = vec3(0.0f, 0.0f, 1.0f);

const vec2 kZero2 = vec2(0.0f, 0.0f);
const vec2 kOne2 = vec2(1.0f, 1.0f);

const vec4 kZero4 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
const vec4 kOne4 = vec4(1.0f, 1.0f, 1.0f, 1.0f);

const mat4 kIdentity;

namespace glm
{
	std::ostream& operator<<(std::ostream& os, const vec2& vec)
	{
		os << vec.x << ", " << vec.y;
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const vec3& vec)
	{
		os << vec.x << ", " << vec.y << ", " << vec.z;
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const quat& rot)
	{
		vec3 angles = eulerAngles(rot);
		os << "pitch: " << angles.x << ", "
			<< "yaw: " << angles.y << ", "
			<< "roll: " << angles.z;
		return os;
	}
}