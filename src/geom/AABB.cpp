#include "PCH.h"

#include "geom/AABB.h"

namespace cs
{
	const uint16 kBBSolidIndices[] = 
	{
		2, 1, 0, 3, 2, 0,
		4, 5, 6, 4, 6, 7,
		1, 5, 0, 0, 5, 4,
		3, 7, 2, 2, 7, 6,
		0, 4, 3, 3, 4, 7,
		2, 6, 1, 1, 6, 5
	};
	const int32 kNumBBIndices = sizeof(kBBSolidIndices) / sizeof(kBBSolidIndices[0]);
	const int32 kNumBBTriangles = kNumBBIndices / 3;


	bool AABB::intersects(const Ray& ray, vec3& hit_pos)
	{
		int32 idxCtr = 0;
		float32 u, v, t;
		for (int i = 0; i < kNumBBTriangles; i++) 
		{
			assert((idxCtr + 2) < kNumBBIndices);
			vec3& p0 = this->positions[kBBSolidIndices[idxCtr + 0]];
			vec3& p1 = this->positions[kBBSolidIndices[idxCtr + 1]];
			vec3& p2 = this->positions[kBBSolidIndices[idxCtr + 2]];
			if (rayIntersectsTriangle(ray.getDirection(), ray.getOrigin(), p0, p1, p2, u, v, t)) 
			{
				hit_pos = ray.getPointAt(t);
				return true;
			}
			idxCtr += 3;
		}
		return false;
	}

	void AABB::initPositions()
	{
		this->positions.clear();

		this->positions.push_back(vec3(minBound.x, minBound.y, minBound.z));
		this->positions.push_back(vec3(minBound.x, minBound.y, maxBound.z));
		this->positions.push_back(vec3(maxBound.x, minBound.y, maxBound.z));
		this->positions.push_back(vec3(maxBound.x, minBound.y, minBound.z));


		this->positions.push_back(vec3(minBound.x, maxBound.y, minBound.z));
		this->positions.push_back(vec3(minBound.x, maxBound.y, maxBound.z));
		this->positions.push_back(vec3(maxBound.x, maxBound.y, maxBound.z));
		this->positions.push_back(vec3(maxBound.x, maxBound.y, minBound.z));
	}
}