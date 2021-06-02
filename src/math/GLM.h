#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include <ostream>
#include <vector>

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::quat;

namespace glm
{
	std::ostream& operator<<(std::ostream& os, const vec2& vec);
	std::ostream& operator<<(std::ostream& os, const vec3& vec);
	std::ostream& operator<<(std::ostream& os, const quat& rot);
}

extern const vec3 kZero3;
extern const vec3 kOne3;

extern const vec3 kDefalutXAxis;
extern const vec3 kDefalutYAxis;
extern const vec3 kDefalutZAxis;

extern const vec2 kZero2;
extern const vec2 kOne2;

extern const vec4 kZero4;
extern const vec4 kOne4;

extern const mat4 kIdentity;

namespace glm_util
{
	inline vec3 transform(const vec3& pos, const mat4& rot, bool translation)
	{
		vec4 tmpPos(pos, translation ? 1.0f : 0.0f);
		tmpPos = rot * tmpPos;
		if (translation)
			return vec3(tmpPos.x / tmpPos.w, tmpPos.y / tmpPos.w, tmpPos.z / tmpPos.w);
		
		return vec3(tmpPos.x, tmpPos.y, tmpPos.z);
	}

	inline void bounds(std::vector<vec2>& positions, vec2& min_pos, vec2& max_pos)
	{
		float minx = std::numeric_limits<float>::infinity(), maxx = -std::numeric_limits<float>::infinity();
		float miny = std::numeric_limits<float>::infinity(), maxy = -std::numeric_limits<float>::infinity();
		for (auto& it : positions)
		{
			minx = (it.x < minx) ? it.x : minx;
			miny = (it.y < miny) ? it.y : miny;

			maxx = (it.x > maxx) ? it.x : maxx;
			maxy = (it.y > maxy) ? it.y : maxy;
		}
		min_pos = vec2(minx, miny);
		max_pos = vec2(maxx, maxy);
	}
	
	inline void bounds(std::vector<vec3>& positions, vec3& min_pos, vec3& max_pos)
	{
		float minx = std::numeric_limits<float>::infinity(), maxx = -std::numeric_limits<float>::infinity();
		float miny = std::numeric_limits<float>::infinity(), maxy = -std::numeric_limits<float>::infinity();
		float minz = std::numeric_limits<float>::infinity(), maxz = -std::numeric_limits<float>::infinity();
		
		for (auto& it : positions)
		{
			minx = (it.x < minx) ? it.x : minx;
			miny = (it.y < miny) ? it.y : miny;
			minz = (it.z < minz) ? it.z : minz;

			maxx = (it.x > maxx) ? it.x : maxx;
			maxy = (it.y > maxy) ? it.y : maxy;
			maxz = (it.z > maxz) ? it.z : maxz;
		}
		min_pos = vec3(minx, miny, minz);
		max_pos = vec3(maxx, maxy, maxz);
	}

}
