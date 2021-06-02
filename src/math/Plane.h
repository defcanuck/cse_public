#pragma once

#include "math/GLM.h"
#include "math/Ray.h"
#include "global/Values.h"

namespace cs
{
	class Plane
	{
	public:
		
		Plane();
		Plane(float32 a, float32 b, float32 c, float32 d);
		Plane(const vec3& pt, const vec3& normal);
		Plane(const vec3& pt1, const vec3& pt2, const vec3& pt3);
		Plane(const vec3& pn, float32 dn);
		
		~Plane();
		
		static float32 dot(const Plane &p, const vec3 &pt);

		const vec3 &normal() const;
		vec3 &normal();
		void normalize();
		void set(float32 a, float32 b, float32 c, float32 d);
		void fromPointNormal(const vec3 &pt, const vec3 &normal);
		void fromPoints(const vec3 &pt1, const vec3 &pt2, const vec3 &pt3);

		bool intersects(const Ray& ray, float32& t) const;
		bool intersects(const vec3& ray_origin, const vec3& ray_direction, float32& t) const;
		bool intersects(const Ray& ray, vec3& pos) const;

		vec3 n;
		float32 d;
	};
}