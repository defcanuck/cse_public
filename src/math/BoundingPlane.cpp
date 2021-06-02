#include "PCH.h"

#include "math/BoundingPlane.h"

namespace cs
{
	bool BoundingPlane::intersect(const Ray& ray, const Transform& transform, HitParams& params)
	{

		float32 t = 0;
		if (this->plane.intersects(ray.getOrigin(), ray.getDirection(), t))
		{
			params.hitDistance = t;
			params.hitNormal = this->plane.normal();
			params.hitPos = ray.getPointAt(t);
			return true;
		}

		return false;
	}
}