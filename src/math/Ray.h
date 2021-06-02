#pragma once

#include "math/GLM.h"

namespace cs
{
	class Ray
	{
	public:

		Ray() : 
			origin(vec3(0.0f, 0.0f, 0.0f)), 
			direction(vec3(0.0f, 0.0f, 1.0f)) { }
		
		Ray(const vec3& o, const vec3& d) : 
			origin(o), 
			direction(d) { }
		
		Ray(const Ray& ray) : 
			origin(ray.origin), 
			direction(ray.direction) { }

		Ray& operator=(const Ray& rhs)
		{
			this->origin = rhs.origin;
			this->direction = rhs.direction;
			return *this;
		}

		const vec3& getOrigin() const { return origin; }
		const vec3& getDirection() const { return direction; }

		inline vec3 getPointAt(float t) const
		{
			return this->origin + (this->direction * t);
		}

		const void transform(const mat4& mat)
		{
			this->origin = glm_util::transform(this->origin, mat, true);
			this->direction = glm_util::transform(this->direction, mat, false);
		}

		vec3 origin;
		vec3 direction;
	};
}