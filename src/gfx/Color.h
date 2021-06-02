#pragma once

#include "global/Values.h"
#include "math/GLM.h"

#include <climits>
#include <algorithm>

namespace cs
{

	template <typename T>
	struct ColorChannel
	{
		typedef T Value;
		const static T max_value() { return 1; }
	};

#if defined(CS_IOS)

	template<> 
    const uchar ColorChannel<uchar>::max_value();

	template<>
    const uint32 ColorChannel<uint32>::max_value();

	template<>
    const float32 ColorChannel<float32>::max_value();

#else

	template<>
	const uchar ColorChannel<uchar>::max_value() { return 255; }

	template<>
	const uint32 ColorChannel<uint32>::max_value() { return 255; }

	template<>
	const float32 ColorChannel<float32>::max_value() { return 1.0f; }

#endif

	template <class T>
	struct Color
	{
		
		Color() :
			r(0), g(0), b(0), a(0) { }

		Color(typename T::Value rr, typename T::Value gg, typename T::Value bb, typename T::Value aa) :
			r(rr), g(gg), b(bb), a(aa) { }

		Color(typename T::Value& val) :
			r(val), g(val), b(val), a(val) { }

		Color(const int32& val) :
			r(typename T::Value(val)), 
			g(typename T::Value(val)), 
			b(typename T::Value(val)), 
			a(typename T::Value(val)) { }

		Color(const Color<T>& rhs)
		{
			this->r = rhs.r;
			this->g = rhs.g;
			this->b = rhs.b;
			this->a = rhs.a;
		}

		typename T::Value r;
		typename T::Value g;
		typename T::Value b;
		typename T::Value a;

		Color<T>& operator=(const Color<T>& rhs)
		{
			this->r = rhs.r;
			this->g = rhs.g;
			this->b = rhs.b;
			this->a = rhs.a;
			return *this;
		}

		bool operator==(const Color<T>& rhs) const
		{
			return (this->r == rhs.r &&
					this->g == rhs.g &&
					this->b == rhs.b &&
					this->a == rhs.a);
		}

		bool operator!=(const Color<T>& rhs) const
		{
			return (this->r != rhs.r ||
					this->g != rhs.g ||
					this->b != rhs.b ||
					this->a != rhs.a);
		}

		Color<T>& operator+=(const Color<T>& rhs)
		{
            typename T::Value mx = T::max_value();

			this->r = std::min<typename T::Value>(rhs.r + this->r, mx);
			this->g = std::min<typename T::Value>(rhs.g + this->g, mx);
			this->b = std::min<typename T::Value>(rhs.b + this->b, mx);
			this->a = std::min<typename T::Value>(rhs.a + this->a, mx);
			return *this;
		}

		Color<T> mod(const Color<T>& rhs)
		{
			Color<T> newColor;
			typename T::Value rr = this->r + (T::Value)(uint32(abs(this->r - rhs.r)));
		}

		Color<T> operator*(const float32& rhs)
		{
#if !defined(CS_IPHONE)
			typename T::Value mx = (T::Value) T::max_value();
#else
            typename T::Value mx = T::max_value();
#endif
			Color<T> newColor;
			newColor.r = std::min<typename T::Value>((typename T::Value) (this->r * rhs), mx);
			newColor.g = std::min<typename T::Value>((typename T::Value) (this->g * rhs), mx);
			newColor.b = std::min<typename T::Value>((typename T::Value) (this->b * rhs), mx);
			newColor.a = std::min<typename T::Value>((typename T::Value) (this->a * rhs), mx);
			return newColor;
		}

		static const Color<T> Clear;
		static const Color<T> Black;
		static const Color<T> White;
		static const Color<T> Grey;
		static const Color<T> Red;
		static const Color<T> Green;
		static const Color<T> Blue;
	};

	template <typename T>
	const Color<T> Color<T>::Clear(0, 0, 0, 0);

	template <typename T>
	const Color<T> Color<T>::Black(0, 0, 0, T::max_value());

	template <typename T>
	const Color<T> Color<T>::White(T::max_value(), T::max_value(), T::max_value(), T::max_value());

	template <typename T>
	const Color<T> Color<T>::Grey(T::max_value() / 2, T::max_value() / 2, T::max_value() / 2, T::max_value());

	template <typename T>
	const Color<T> Color<T>::Red(T::max_value(), 0, 0, T::max_value());

	template <typename T>
	const Color<T> Color<T>::Green(0, T::max_value(), 0, T::max_value());

	template <typename T>
	const Color<T> Color<T>::Blue(0, 0, T::max_value(), T::max_value());

	template <typename T>
	std::ostream& operator<<(std::ostream& oss, const Color<T>& color)
	{
		oss << "r: " << color.r << ", "
			<< "g: " << color.g << ", "
			<< "b: " << color.b << ", "
			<< "a: " << color.a;
		return oss;
	}

	template <typename T>
	Color<T> operator*(const Color<T>& lhs, const Color<T>& rhs)
	{
		Color<T> newColor;
		uint32 mx = (uint32)T::max_value();
		float32 inv_mx = 1.0f / mx;
		newColor.r = (typename T::Value) std::min<uint32>(uint32(float32(lhs.r) * inv_mx * float32(rhs.r)), mx);
		newColor.g = (typename T::Value) std::min<uint32>(uint32(float32(lhs.g) * inv_mx * float32(rhs.g)), mx);
		newColor.b = (typename T::Value) std::min<uint32>(uint32(float32(lhs.b) * inv_mx * float32(rhs.b)), mx);
		newColor.a = (typename T::Value) std::min<uint32>(uint32(float32(lhs.a) * inv_mx * float32(rhs.a)), mx);
		return newColor;
	}

	template <typename T>
	Color<T> operator*(const Color<T>& lhs, const float32& rhs)
	{
		Color<T> newColor;
		newColor.r = (typename T::Value) (lhs.r * rhs);
		newColor.g = (typename T::Value) (lhs.g * rhs);
		newColor.b = (typename T::Value) (lhs.b * rhs);
		newColor.a = (typename T::Value) (lhs.a * rhs);
		return newColor;
	}


	template <typename T>
	Color<T> operator+(const Color<T>& lhs, const Color<T>& rhs)
	{
		Color<T> newColor;
		uint32 mx = (uint32) T::max_value();
		newColor.r = (typename T::Value) std::min<uint32>(uint32(lhs.r) + uint32(rhs.r), mx);
		newColor.g = (typename T::Value) std::min<uint32>(uint32(lhs.g) + uint32(rhs.g), mx);
		newColor.b = (typename T::Value) std::min<uint32>(uint32(lhs.b) + uint32(rhs.b), mx);
		newColor.a = (typename T::Value) std::min<uint32>(uint32(lhs.a) + uint32(rhs.a), mx);
		return newColor;
	}

	template <typename T>
	Color<ColorChannel<uchar>> operator+(const Color<ColorChannel<uchar>>& lhs, const Color<ColorChannel<uchar>>& rhs)
	{
		Color<ColorChannel<uchar>> newColor;
		typename ColorChannel<uchar>::Value mx = (ColorChannel<uchar>::Value) T::max_value();
		newColor.r = std::min<typename ColorChannel<uchar>::Value>(lhs.r + rhs.r, mx);
		newColor.g = std::min<typename ColorChannel<uchar>::Value>(lhs.g + rhs.g, mx);
		newColor.b = std::min<typename ColorChannel<uchar>::Value>(lhs.b + rhs.b, mx);
		newColor.a = std::min<typename ColorChannel<uchar>::Value>(lhs.a + rhs.a, mx);
		return newColor;
	}

	template <typename T>
	Color<ColorChannel<uchar>> operator*(const Color<ColorChannel<uchar>>& lhs, const Color<ColorChannel<uchar>>& rhs)
	{
		Color<ColorChannel<uchar>> newColor;
		typename T::Value mx = (ColorChannel<uchar>::Value) T::max_value();
		newColor.r = (typename ColorChannel<uchar>::Value) (lhs.r * rhs.r / float32(mx));
		newColor.g = (typename ColorChannel<uchar>::Value) (lhs.g * rhs.g / float32(mx));
		newColor.b = (typename ColorChannel<uchar>::Value) (lhs.b * rhs.b / float32(mx));
		newColor.a = (typename ColorChannel<uchar>::Value) (lhs.a * rhs.a / float32(mx));
		return newColor;
	}

	typedef Color<ColorChannel<uchar>> ColorB;
	typedef Color<ColorChannel<float32>> ColorF;
	typedef Color<ColorChannel<uint32>> ColorUI;

	typedef std::vector<ColorB> ColorBList;
	typedef std::vector<ColorF> ColorFList;

	inline ColorF toColorF(const ColorB& color)
	{
		ColorF newColor;
		float32 recip = 1.0f / 255.0f;
		newColor.r = color.r * recip;
		newColor.g = color.g * recip;
		newColor.b = color.b * recip;
		newColor.a = color.a * recip;
		return newColor;
	}

	inline ColorB toColorB(const ColorF& color)
	{
		ColorB newColor;
		newColor.r = uchar(color.r * 255);
		newColor.g = uchar(color.g * 255);
		newColor.b = uchar(color.b * 255);
		newColor.a = uchar(color.a * 255);
		return newColor;
	}

	inline vec4 toVec4(const ColorF& color)
	{
		return vec4(color.r, color.g, color.b, color.a);
	}

	inline vec4 toVec4(const ColorB& color)
	{
		const float32 kScale = 1.0f / 255.0f;
		return vec4(color.r * kScale, color.g * kScale, color.b * kScale, color.a * kScale);
	}


}
