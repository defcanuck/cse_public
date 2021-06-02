#include "PCH.h"

#include "math/BoundingSphere.h"
#include "math/Ray.h"
#include "math/Intersect.h"

namespace cs
{
	bool BoundingSphere::intersect(const Ray& ray, const Transform& transform, HitParams& params)
	{

		vec3 offset = transform.getPosition();
		return raySphereIntersect(
			ray.getDirection(),
			ray.getOrigin(),
			offset,
			this->radius,
			params.hitPos,
			params.hitDistance,
			params.hitNormal);
	}

}