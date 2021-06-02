#pragma once 

#include "math/GLM.h"
#include <Box2D/Box2D.h>

namespace cs
{
	struct PhysicsConst
	{
		static const float32 kBox2DScale;
		static const float32 kInvBox2DScale;

		static inline float32 box2DToWorld(const float32& value)
		{
			return value * kBox2DScale;
		}

		static inline b2Vec2 box2DToWorld(const b2Vec2& value)
		{
			return b2Vec2(box2DToWorld(value.x), box2DToWorld(value.y));
		}

		static inline b2Vec2 box2DToWorld(const vec2& value)
		{
			return b2Vec2(box2DToWorld(value.x), box2DToWorld(value.y));
		}

		static inline b2Vec2 box2DToWorld(const float32& x, const float32& y)
		{
			return b2Vec2(box2DToWorld(x), box2DToWorld(y));
		}

		static inline float32 worldToBox2D(const float32& value)
		{
			return value * kInvBox2DScale;
		}

		static inline b2Vec2 worldToBox2D(const b2Vec2& value)
		{
			return b2Vec2(worldToBox2D(value.x), worldToBox2D(value.y));
		}

		static inline b2Vec2 worldToBox2D(const vec2& value)
		{
			return b2Vec2(worldToBox2D(value.x), worldToBox2D(value.y));
		}

		static inline b2Vec2 worldToBox2D(const float32& x, const float32& y)
		{
			return b2Vec2(worldToBox2D(x), worldToBox2D(y));
		}

		static const float32 kBox2DPaddingValue;
		static const float32 kHalfScale;

		static inline float32 dimmToBox2D(float32 val)
		{
			return worldToBox2D((val * kHalfScale) - kBox2DPaddingValue);
		}

		

	};
}