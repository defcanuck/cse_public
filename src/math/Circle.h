#pragma once

#include "global/Values.h"
#include "math/Plane.h"

namespace cs
{
	class Circle
	{
	public:
		Circle()
			: radius(1.0f)
			, position()
			, normal() { }

		Circle(const float32& rad, const vec3& pos, const vec3& nml = kDefalutZAxis)
			: radius(rad)
			, position(pos)
			, normal(nml) { }

		Circle(const Circle& rhs)
			: radius(rhs.radius)
			, position(rhs.position)
			, normal(rhs.normal)
		{ }

		void operator=(const Circle& rhs)
		{
			this->radius = rhs.radius;
			this->position = rhs.position;
			this->normal = rhs.normal;
		}

		bool intersects(const Ray& ray, vec3& hit_pos)
		{
			Plane p(this->position, this->normal);
			if (p.intersects(ray, hit_pos))
			{
				return glm::distance(hit_pos, position) <= radius;
			}
			return false;
		}

		float32 radius;
		vec3 position;
		vec3 normal;
	};
}