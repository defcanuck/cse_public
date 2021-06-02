#pragma once

#include "math/BoundingVolume.h"
#include "math/Plane.h"

namespace cs
{
	CLASS_DEFINITION_DERIVED(BoundingPlane, BoundingVolume)
	public:
		BoundingPlane() : plane() { }
		BoundingPlane(const vec3& p, const float32 d) : plane(p, d) { }
		BoundingPlane(const Plane& p) : plane(p) { }

		virtual bool intersect(const Ray& ray, const Transform& transform, HitParams& params);

	private:

		Plane plane;
	};
}