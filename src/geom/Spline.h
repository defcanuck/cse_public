#pragma once

#include "math/GLM.h"

namespace cs
{
	namespace spline
	{
		template <class T>
		T pointOnCurve(const T& p0, const T& p1, const T& p2, const T& p3, float32 t)
		{
			T ret;

			float t2 = t * t;
			float t3 = t2 * t;

			ret = 0.5f * ((2.0f * p1) +
				(-p0 + p2) * t +
				(2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
				(-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);

			return ret;
		}

		template <class T>
		T evalHermite(T A, T B, T C, T D, float t)
		{
			T a = -A / 2.0f + (3.0f*B) / 2.0f - (3.0f*C) / 2.0f + D / 2.0f;
			T b = A - (5.0f*B) / 2.0f + 2.0f*C - D / 2.0f;
			T c = -A / 2.0f + C / 2.0f;
			T d = B;

			return a*t*t*t + b*t*t + c*t + d;
		}
	}
}