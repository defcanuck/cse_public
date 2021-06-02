#pragma once

#include "ClassDef.h"

#include "math/GLM.h"

#include <vector>

namespace cs
{
	CLASS_DEFINITION(AABB)
	public:
		AABB()
			: minBound(vec3())
			, maxBound(vec3())
		{

		}

		AABB(const vec3& minb, const vec3& maxb)
			: minBound(minb)
			, maxBound(maxb)
		{
			this->initPositions();
		}

		bool intersects(const Ray& ray, vec3& hit_pos);

		inline const vec3& getMin() const { return this->minBound; }
		inline const vec3& getMax() const { return this->maxBound; }

		inline float32 getSpanX() const { return this->maxBound.x - this->minBound.x; }
		inline float32 getSpanY() const { return this->maxBound.y - this->minBound.y; }
		inline float32 getSpanZ() const { return this->maxBound.z - this->minBound.z; }
			
		const std::vector<vec3>& getPositions() const { return this->positions; }

	private:

		void initPositions();

		vec3 minBound;
		vec3 maxBound;
		std::vector<vec3> positions;

	};
}