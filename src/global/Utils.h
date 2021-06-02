#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <algorithm>
#include <memory>
#include <vector>
#include <sstream>
#include <cctype>

#include "global/Values.h"
#include "math/GLM.h"
#include "gfx/Color.h"

#define CS_PI   3.14159265358979323846	/* pi */

#define PTR_ADD( PTR, OFFSET ) \
  ((void *)(((char *)(PTR)) + (OFFSET)))

#define ARRAY_LENGTH(vptr) sizeof(vptr) / sizeof(vptr[0])

#define assert_message(cond, msg) \
	if (!cond) \
	{ \
		log::error(msg); \
		assert(false); \
	}

namespace cs
{
	inline StringList explode(const std::string& s, char delim)
	{
		StringList result;
		std::istringstream iss(s);

		for (std::string token; std::getline(iss, token, delim);)
		{
			result.push_back(std::move(token));
		}

		return result;
	}

	inline std::string implode_forward(const StringList& list, char delim)
	{
		std::string tmp;
		for (int32 i = 0; i < (int32) list.size(); i++)
		{
			if (list[i].length() == 0)
				continue;
			tmp = tmp + delim + list[i];
		}
		tmp = tmp + delim;
		
		return tmp;
	}

	inline std::string implode_backward(const StringList& list, char delim)
	{
		std::string tmp;
		for (int32 i = (int32) list.size() - 1; i >= 0; i--)
		{
			if (list[i].length() == 0)
				continue;
			tmp = tmp + delim + list[i];
		}
		tmp = tmp + delim;

		return tmp;
	}

	inline std::string to_lowercase(const std::string& str)
	{
		std::string tmp = str;
		std::transform(tmp.begin(), tmp.end(), tmp.begin(),
			[](unsigned char c)
			{
				return std::tolower(c); 
			}
		);
		return tmp;
	}

	template <typename T>
	bool fuzzyCompare(T& value, T eq, T bias)
	{
		return abs(value - eq) < bias;
	}
    
    inline float32 randomFloat()
    {
        return static_cast<float32>(rand()) / static_cast<float32>(RAND_MAX);
    }

	template<typename T>
	T randomRange(T min, T max)
	{
		float t = static_cast<int32>(std::rand()) / static_cast<float32>(RAND_MAX);
		return static_cast<T>((t * min) + ((1.0f - t) * max));
	}

	template <typename T>
	T clamp(const T& min, const T& max, const T& value)
	{
		return std::min<T>(max, std::max<T>(min, value));
	}

	template <typename T>
	T lerp(const T& a, const T& b, float32 t)
	{
		return (T) ((a * (1.0f - t)) + (b * t));
	}

	template <class T>
	void fast_add(std::vector<T>& dst, const std::vector<T>& src)
	{
		dst.reserve(src.size());
		dst.insert(dst.begin(), src.begin(), src.end());
	}

	inline void removeSubstrs(std::string& s, const std::string& p)
	{
		std::string::size_type n = p.length();
		for (std::string::size_type i = s.find(p); i != std::string::npos; i = s.find(p))
			s.erase(i, n);
	}

	inline uchar scale(float32 val)
	{
		return (uchar)(val * 255);
	}

	inline uchar scaleClamp(float32 val)
	{
		return (uchar)(fabs(val) * 255);
	}
    
    inline bool checkPow2(uint32 n)
    {
        return n > 0 && (n & (n - 1)) == 0;
    }

	inline uint32 nextPow2(uint32 n)
	{
		n--;
		n |= n >> 1;   // Divide by 2^k for consecutive doublings of k up to 32,
		n |= n >> 2;   // and then or the results.
		n |= n >> 4;
		n |= n >> 8;
		n |= n >> 16;
		n++;

		return n;
	}

	inline float32 degreesToRadians(float32 degrees)
	{
		return degrees * (float32(CS_PI) / 180.0f);
	}

	inline float32 radiansToDegrees(float32 radians)
	{
		return (radians / float32(CS_PI)) * 180.0f;
	}

	struct FloatExtentCalculator
	{
		FloatExtentCalculator()
			: minValue(FLT_MAX)
			, maxValue(-FLT_MAX)
		{ }

		inline void reset()
		{
			minValue = FLT_MAX;
			maxValue = -FLT_MAX;
		}

		inline void evaluate(const float32& value)
		{
			maxValue = (value > maxValue) ? value : maxValue;
			minValue = (value < minValue) ? value : minValue;
		}
		
		inline void evaluate(const FloatExtentCalculator& calc)
		{
			this->evaluate(calc.minValue);
			this->evaluate(calc.maxValue);
		}
		inline float32 span() { return maxValue - minValue; }

		float32 maxValue;
		float32 minValue;
	};

	template <class T>
	struct RangeValue
	{
		RangeValue() 
			: min_value(T())
			, max_value(T())
		{ }

		RangeValue(const T& minv, const T& maxv)
			: min_value(minv)
			, max_value(maxv)
		{ }

		T getLerpValue(float32 t)
		{
			return lerp<T>(this->min_value, this->max_value, t);
		}

		static void applyLerp(float32 t, RangeValue<T>* src, T* dst)
		{
			assert(dst);
			assert(src);
			*dst = src->getLerpValue(t);
		}

		T min_value;
		T max_value;
	};

	template <typename T>
	std::ostream& operator<<(std::ostream& os, const RangeValue<T>& rhs)
	{
		os << rhs.min_value << ", " << rhs.max_value;
		return os;
	}

	typedef RangeValue<float32> FloatRangeValue;
	typedef RangeValue<int32> IntRangeValue;	
	typedef RangeValue<vec2> Vec2RangeValue;
	typedef RangeValue<vec3> Vec3RangeValue;
	typedef RangeValue<ColorB> ColorBRangeValue;
}
