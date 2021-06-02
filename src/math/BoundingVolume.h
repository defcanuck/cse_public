#pragma once

#include "ClassDef.h"
#include "global/Values.h"

#include "math/GLM.h"
#include "math/Ray.h"
#include "math/Transform.h"

namespace cs
{

	struct HitParams
	{
		vec3 hitPos;
		vec3 hitNormal;
		float32 hitDistance;
	};

	CLASS_DEFINITION(BoundingVolume)
	
	public:
	
		BoundingVolume() { }
		virtual ~BoundingVolume() { }

		virtual bool intersect(const Ray& ray, const Transform& trans, HitParams& params) = 0;


	};

}