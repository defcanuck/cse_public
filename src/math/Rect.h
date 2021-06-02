#pragma once

#include "global/Values.h"
#include "math/GLM.h"
#include "math/Ray.h"
#include "math/Intersect.h"
#include "global/Utils.h"

// #define USE_VARIABLE_SPLIT

namespace cs
{

	template <typename T>
	struct Point
	{

		Point() : x(), y() { }
		Point(T xx, T yy) : x(xx), y(yy) { }
		Point(const vec2& vec) : x(float32(vec.x)), y(float32(vec.y)) { }
		Point(const Point<T>& rhs) : x(rhs.x), y(rhs.y) { }

		T getProduct() const
		{
			return this->x * this->y;
		}

		void combine(const Point<T>& pt)
		{
			this->x = std::max<T>(pt.x, this->x);
			this->y = std::max<T>(pt.y, this->y);
		}

		union
		{
			T x;
			T w;
		};

		union
		{
			T y;
			T h;
		};

		bool operator==(const Point<T>& rhs)
		{
			return this->x == rhs.x && this->y == rhs.y;
		}
	};

	template <typename T>
	std::ostream& operator<<(std::ostream& os, const Point<T>& rhs)
	{
		os << rhs.x << ", " << rhs.y;
		return os;
	}

	template <typename T>
	bool operator==(const Point<T>& lhs, const Point<T>& rhs)
	{
		return rhs.x == lhs.x && rhs.y == lhs.y;
	}

	template <typename T>
	bool operator!=(const Point<T>& lhs, const Point<T>& rhs)
	{
		return rhs.x != lhs.x || rhs.y != lhs.y;
	}

	template <typename T>
	Point<T> operator-(const Point<T>& lhs, const Point<T>& rhs)
	{
		Point<T> result;
		result.x = lhs.x - rhs.x;
		result.y = lhs.y - rhs.y;
		return result;
	}

	template <typename T>
	Point<T> operator+(const Point<T>& lhs, const Point<T>& rhs)
	{
		Point<T> result;
		result.x = lhs.x + rhs.x;
		result.y = lhs.y + rhs.y;
		return result;
	}


	enum RectCorner
	{
		BottomLeft,
		TopLeft,
		TopRight,
		BottomRight, 
		CornersMAX
	};

	template <typename T>
	struct Rect
	{
		Rect() :
			pos(Point<T>()),
			size(Point<T>()) { }

		Rect(const T& x, const T& y, const T& w, const T& h) :
			pos(x, y),
			size(w, h) { }

		Rect(const vec2& p, const vec2& d) :
			pos(Point<T>(p)),
			size(Point<T>(d)) { }

		Rect(const Point<T>& p, const Point<T>& d) :
			pos(p),
			size(d) { }

		Rect(const Rect<T>& rhs)
			: pos(rhs.pos)
			, size(rhs.size)
		{ }

		void operator=(const Rect<T>& rhs)
		{
			this->pos = rhs.pos;
			this->size = rhs.size;
		}

		void zero()
		{
			this->pos.x = 0;
			this->pos.y = 0;
			this->size.x = 0;
			this->size.y = 0;
		}

		bool isZero() { return this->size.w == 0 || this->size.h == 0; }

		Point<T> getBL() const { return Point<T>(this->pos.x, this->pos.y); }
		Point<T> getTL() const { return Point<T>(this->pos.x, this->pos.y + this->size.h); }
		Point<T> getTR() const { return Point<T>(this->pos.x + this->size.w, this->pos.y + this->size.h); }
		Point<T> getBR() const { return Point<T>(this->pos.x + this->size.w, this->pos.y); }

		Point<T> getCenter() const { return Point<T>(this->pos.x + T(float32(this->size.w) * 0.5f), T(this->pos.y + float32(this->size.h) * 0.5)); }

		void flipHorizontal()
		{
			this->pos.x = this->pos.x + this->size.w;
			this->size.w = -this->size.w;
			
		}

		void flipVertical()
		{
			this->pos.y = this->pos.y + this->size.h;
			this->size.h = -this->size.h;
		}

		Point<T> pos;
		Point<T> size;

		T getTop() const { return this->pos.y + this->size.h; }
		T getBottom() const { return this->pos.y; }
		T getLeft() const { return this->pos.x; }
		T getRight() const { return this->pos.x + this->size.w; }

		void getLRTB(T& left, T& right, T& bot, T& top) const
		{
			left = this->pos.x;
			right = this->pos.x + this->size.w;
			bot = this->pos.y;
			top = this->pos.y + this->size.h;
		}

		void offset(const vec2& off)
		{
			this->pos.x += off.x;
			this->pos.y += off.y;
		}

		void offset(const Point<T>& off)
		{
			this->pos.x += off.x;
			this->pos.y += off.y;
		}

		void offset(const float32& x, const float32& y)
		{
			this->pos.x += x;
			this->pos.y += y;
		}

		void combine(const Rect<T>& rect)
		{
			T sl, sr, sb, st;
			T fl, fr, fb, ft;
			this->getLRTB(sl, sr, sb, st);
			rect.getLRTB(fl, fr, fb, ft);

			T left = std::min<T>(sl, fl);
			T right = std::max<T>(sr, fr);
			T bottom = std::min<T>(sb, fb);
			T top = std::max<T>(st, ft);

			this->pos = Point<T>(left, bottom);
			this->size = Point<T>(right - left, top - bottom);
		}

		bool canFit(Point<T>& size)
		{
			return (this->size.w > size.w) && (this->size.h > size.h);
		}

		void split(Point<T>& size, Rect<T>& src, Rect<T>& a, Rect<T>& b)
		{
			assert(this->size.w > size.w);
			assert(this->size.h > size.h);

			T lhs = this->pos.x;
			T rhs = this->pos.x + size.w;
			T btm = this->pos.y;
			T top = this->pos.y + size.h;

			src = Rect<T>(lhs, btm, size.w, size.h);
			
			Rect a0 = Rect<T>(lhs, top, size.w, this->size.h - size.h);
			Rect b0 = Rect<T>(rhs, btm, this->size.w - size.w, this->size.h);
			
#if defined(USE_VARIABLE_SPLIT)
			Rect a1 = Rect<T>(lhs, top, this->size.w, this->size.h - size.h);
			Rect b1 = Rect<T>(rhs, btm, this->size.w - size.w, size.h);
			
			if (abs(a0.size.getProduct() - b0.size.getProduct()) < 
				abs(a1.size.getProduct() - b1.size.getProduct()))
			{
				a = a0;
				b = b0;
			} else {
				a = a1;
				b = b1; 
			}
#else
			a = a0;
			b = b0;
#endif
		}

		void growY(float32 scale)
		{
			T new_height = this->size.h * scale;
			T diff = new_height - this->size.h;

			this->pos.x -= diff;
			this->size.h = new_height;
		}

		void scale(Point<T>& scaleSz)
		{
			float32 invX = 1.0f / float32(scaleSz.x);
			float32 invY = 1.0f / float32(scaleSz.y);

			this->pos.x = this->pos.x * invX;
			this->pos.y = this->pos.y * invY;

			this->size.w = this->size.w * invX;
			this->size.h = this->size.h * invY;
		}

		void scaleCenter(Point<T>& scaleSz)
		{
			this->size.w *= scaleSz.x;
			this->size.h *= scaleSz.y;

			this->setCenter();
		}

		void scaleCenterRelative(Point<T>& scaleSz, Point<T>& centerDimm)
		{
			this->size.w *= scaleSz.x;
			this->size.h *= scaleSz.y;

			T offsetX = centerDimm.x - this->size.w;
			T offsetY = centerDimm.y - this->size.h;

			this->pos.w = offsetX * 0.5f;
			this->pos.y = offsetY * 0.5f;
		}

		void setCenter()
		{
			float32 halfW = this->size.w / 2.0f;
			float32 halfH = this->size.h / 2.0f;

			this->pos.x = static_cast<T>(-halfW);
			this->pos.y = static_cast<T>(-halfH);
		}

		void scaleCenter(float32 scale)
		{
			if (scale == 1.0f) return;

			T oldCenterX = this->pos.x + T(float32(this->size.w) * 0.5f);
			T oldCenterY = this->pos.y + T(float32(this->size.h) * 0.5f);

			this->size.w = T(float32(this->size.w) * scale);
			this->size.h = T(float32(this->size.h) * scale);

			this->pos.x = oldCenterX - T(float32(this->size.w) * 0.5f);
			this->pos.y = oldCenterY - T(float32(this->size.h) * 0.5f);

		}
		
		Point<T> getByCorner(RectCorner corner) const
		{
			switch (corner)
			{
				case BottomLeft: return this->getBL();
				case TopLeft: return this->getTL();
				case TopRight: return this->getTR();
				case BottomRight: return this->getBR();
				default:
					assert(false);
					GTFO(-1);
			}
			return Point<T>();
		}

		static bool contains(const Rect<T>& a, const Rect<T>& b)
		{
			return
				a.pos.x < (b.pos.x + b.size.x) &&
				(a.pos.x + a.size.x) > b.pos.x &&
				(a.pos.y + a.size.y) > b.pos.y &&
				a.pos.y < (b.pos.y + b.size.y);
		}

		static bool excludes(const Rect<T>&a, const Rect<T>& b)
		{
			return
				a.pos.x > (b.pos.x + b.size.w) ||
				(a.pos.x + a.size.w) < b.pos.x ||
				a.pos.y > (b.pos.y + b.size.h) ||
				(a.pos.y + a.size.h) < b.pos.y;
		}

		std::string tostring();
	};

	template <typename T, typename P>
	bool rectContains(const Rect<T>& rect, const P& point)
	{
		return
			point.x > rect.pos.x &&
			point.y > rect.pos.y &&
			point.x < (rect.pos.x + rect.size.w) &&
			point.y < (rect.pos.y + rect.size.h);
	}

	template <typename T, typename P>
	vec2 rectPercent(const Rect<T>& rect, const P& point)
	{
		vec2 pct;
		assert(rect.size.w > 0);
		assert(rect.size.h > 0);
		pct.x = (point.x - rect.pos.x) / float32(rect.size.w);
		pct.y = (point.y - rect.pos.y) / float32(rect.size.h);
		return pct;
	}

	template <typename T>
	bool operator==(const Rect<T>& lhs, const Rect<T>& rhs)
	{
		return rhs.pos == lhs.pos && rhs.size == lhs.size;
	}

	template <typename T>
	bool operator!=(const Rect<T>& lhs, const Rect<T>& rhs)
	{
		return rhs.pos != lhs.pos || rhs.size != lhs.size;
	}

	template <typename T>
	std::ostream& operator<<(std::ostream& os, const Rect<T>& rhs)
	{
		os << "Pos: " << rhs.pos << ", Size: " << rhs.size;
		return os;
	}

	typedef Point<int32> PointI;
	typedef Point<float32> PointF;

	typedef Point<int32> Dimensions;
	typedef Point<float32> SizeF;
	typedef Point<int32> SizeI;

	typedef Rect<int32> RectI;
	typedef Rect<float32> RectF;

	inline RectF createRectFromExtents(FloatExtentCalculator& xExt, FloatExtentCalculator& yExt)
	{
		return RectF(xExt.minValue, yExt.minValue, xExt.span(), yExt.span());
	}

	template <typename T>
	inline vec2 toVec2(const Point<T>& p)
	{
		return vec2(float32(p.x), float32(p.y));
	}

	template <typename T>
	inline vec3 toVec3(const Point<T>& p, float32 z = 0.0f)
	{
		return vec3(float32(p.x), float32(p.y), z);
	}

	template <typename T>
	bool rectIntersectLine(const Ray& ray, const Rect<T>& rect)
	{
		
		float32 tmin = (rect.getLeft() - ray.origin.x) / ray.direction.x;
		float32 tmax = (rect.getRight() - ray.origin.x) / ray.direction.x;

		if (tmin > tmax)
		{
			std::swap(tmin, tmax);
		}

		float32 tymin = (rect.getBottom() - ray.origin.y) / ray.direction.y;
		float32 tymax = (rect.getTop() - ray.origin.y) / ray.direction.y;

		if (tymin > tymax)
		{
			std::swap(tymin, tymax);
		}

		if ((tmin > tymax) || (tymin > tmax))
			return false;

		return true;
		
	}

	template <typename T>
	bool rectIntersect(const Ray& ray, const Rect<T>& rect, vec3& hit_pos)
	{
		vec3 p0 = toVec3(rect.getBL());
		vec3 p1 = toVec3(rect.getTL());
		vec3 p2 = toVec3(rect.getTR());
		vec3 p3 = toVec3(rect.getBR());

		float32 u, v, t;
		if (rayIntersectsTriangle(ray.getDirection(), ray.getOrigin(), p0, p1, p2, u, v, t) ||
			rayIntersectsTriangle(ray.getDirection(), ray.getOrigin(), p0, p2, p3, u, v, t))
		{
			hit_pos = ray.getPointAt(t);
			return true;
		}

		return false;
	}

	template <typename T>
	std::string Rect<T>::tostring()
	{
		std::stringstream str;
		str << *this;
		return str.str();
	}

}
