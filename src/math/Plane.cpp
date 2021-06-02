#include "PCH.h"
 
#include "math/Plane.h"

namespace cs
{

	Plane::Plane(void)
	{
	}

	Plane::Plane(float32 a, float32 b, float32 c, float32 d) : n(a, b, c), d(d)
	{
	}

	Plane::Plane(const vec3 &pt, const vec3 &normal)
	{
		fromPointNormal(pt, normal);
	}

	Plane::~Plane(void)
	{
	}

	Plane::Plane(const vec3 &pt1, const vec3 &pt2, const vec3 &pt3)
	{
		fromPoints(pt1, pt2, pt3);
	}

	Plane::Plane(const vec3& pn, float32 dn) :
		n(pn),
		d(dn)
	{

	}

	float32 Plane::dot(const Plane &p, const glm::vec3 &pt)
	{
		// Returns:
		//  > 0 if the point 'pt' lies in front of the plane 'p'
		//  < 0 if the point 'pt' lies behind the plane 'p'
		//    0 if the point 'pt' lies on the plane 'p'
		//
		// The signed distance from the point 'pt' to the plane 'p' is returned.

		return glm::dot(p.n, pt) + p.d;
	}

	void Plane::fromPointNormal(const vec3 &pt, const vec3 &normal)
	{
		set(normal.x, normal.y, normal.z, -glm::dot(normal, pt));
		normalize();
	}

	void Plane::fromPoints(const vec3 &pt1, const vec3 &pt2, const vec3 &pt3)
	{
		n = glm::cross(pt2 - pt1, pt3 - pt1);
		d = -glm::dot(n, pt1);
		normalize();
	}

	const vec3 &Plane::normal() const
	{
		return n;
	}

	vec3 &Plane::normal()
	{
		return n;
	}

	void Plane::normalize()
	{
		float32 length = 1.0f / glm::length(n);
		n *= length;
		d *= length;
	}

	void Plane::set(float32 a, float32 b, float32 c, float32 d)
	{
		n.x = a;
		n.y = b;
		n.z = c;
		this->d = d;
	}


	bool Plane::intersects(const Ray& ray, float32& t) const
	{
		return this->intersects(ray.getOrigin(), ray.getDirection(), t);
	}

	bool Plane::intersects(const Ray& ray, vec3& pos) const
	{
		float32 t = 0;
		if (this->intersects(ray.getOrigin(), ray.getDirection(), t))
		{
			pos = ray.getPointAt(t);
			return true;
		}

		return false;
	}

	bool Plane::intersects(const vec3& ray_origin, const vec3& ray_direction, float32& t) const
	{
		float32 denom = glm::dot(n, ray_direction);
		if (abs(denom) > 0.0001f) // your favorite epsilon
		{ 
			t = -(glm::dot(ray_origin, n) + d) / denom;
			if (t >= 0) 
				return true; 
		}
		return false;
	}
}