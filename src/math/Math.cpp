#include "PCH.h"

#include "math/Intersect.h"

namespace cs
{
	bool raySphereIntersect(
		const vec3& raydir,
		const vec3& rayorg,
		const vec3& pos,
		const float& rad,
		vec3& hitpoint,
		float& distance,
		vec3& normal)
	{

		float a = glm::dot(raydir, raydir);
		float b = 2.0f * glm::dot(raydir, rayorg - pos);
		float c = glm::dot(pos, pos) + glm::dot(rayorg, rayorg) - 2.0f * glm::dot(pos, rayorg) - rad * rad;

		float test = b * b - 4.0f * a * c;

		if (test >= 0.0) {

			float32 recip = 1.0f / (2.0f * a);
			float32 test_sqrt = sqrt(test);
			float32 t0 = (-b - test_sqrt) * recip;
			float32 t1 = (-b + test_sqrt) * recip;

			distance = (t1 < t0 && t1 > 0.0f) ? t1 : t0;
			hitpoint = rayorg + (distance * raydir);
			normal = (hitpoint - pos) / rad;

			return true;
		}
		return false;
	}

	bool rayIntersectsTriangle(
		const vec3& raydir,
		const vec3& rayorg,
		vec3 &v0, // vert 0
		vec3 &v1, // vert 1
		vec3 &v2, // vert 2
		float32 &u,
		float32 &v,
		float32 &t) 
	{

		vec3 e1, e2, h, s, q;
		float a, f;

		e1 = v1 - v0;
		e2 = v2 - v0;

		//vector(e1,v1,v0);
		//vector(e2,v2,v0);

		h = glm::cross(raydir, e2);
		a = glm::dot(e1, h);

		// bail if we're too close
		if (a > -0.00001 && a < 0.00001) {
			return false;
		}

		f = 1 / a;

		s = rayorg - v0;
		u = f * (glm::dot(s, h));

		if (u < 0.0 || u > 1.0)
			return false;

		q = glm::cross(s, e1); // crossProduct(q,s,e1);
		v = f * glm::dot(raydir, q); // innerProduct(d,q);
		if (v < 0.0 || u + v > 1.0)
			return false;
		// at this stage we can compute t to find out where 
		// the intersection point is on the line

		t = f * glm::dot(e2, q); //innerProduct(e2,q);
		if (t > 0.00001) // ray intersection
			return true;

		else // this means that there is a line intersection  
				// but not a ray intersection
			return false;
	}
}