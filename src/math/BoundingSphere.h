#pragma once

#include "math/BoundingVolume.h"

namespace cs
{
	CLASS_DEFINITION_DERIVED(BoundingSphere, BoundingVolume)
	public:
		BoundingSphere() : BoundingVolume() { }
		BoundingSphere(float32 r) : radius(r) { }

		virtual bool intersect(const Ray& ray, const Transform& transform, HitParams& params);

	private:
		float32 radius;
	};
}