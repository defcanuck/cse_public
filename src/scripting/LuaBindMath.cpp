#include "PCH.h"

#include "scripting/LuaBindMath.h"

#include <string>

namespace cs
{

	struct local_vec2
	{
		static vec2 normalize(const vec2& vec) { return glm::normalize(vec); }
		static float32 dot(const vec2& a, const vec2& b) { return glm::dot(a, b); }
		static float32 distance(const vec2& a, const vec2& b) { return glm::distance(a, b); }
		static float32 magnitude(const vec2& a) { return glm::length(a); }
		static std::string asString(const vec2& a)
		{
			std::stringstream str;
			str << "x: " << a.x << " y: " << a.y;
			return str.str();
		}
	};

	struct local_vec3
	{
		static vec3 normalize(const vec3& vec) { return glm::normalize(vec); }
		static vec3 cross(const vec3& a, const vec3& b) { return glm::cross(a, b); }
		static float32 dot(const vec3& a, const vec3& b) { return glm::dot(a, b); }
		static float32 distance(const vec3& a, const vec3& b) { return glm::distance(a, b); }
		static float32 magnitude(const vec3& a) { return glm::length(a); }
		static std::string asString(const vec3& a) 
		{
			std::stringstream str;
			str << "x: " << a.x << " y: " << a.y << " z: " << a.z;
			return str.str();
		}
	};

	struct local_quat
	{
		static quat fromEuler(float32 pitch, float32 yaw, float32 roll)
		{
			return quat(vec3(pitch, yaw, roll));
		}
	};

	using namespace luabind;

	BEGIN_DEFINE_LUA_CLASS_RENAMED(vec2, Vec2)
		.def(constructor<>())
		.def(constructor<float32, float32>())
		.def_readwrite("x", &vec2::x)
		.def_readwrite("y", &vec2::y)
		.def(const_self + const_self)
		.def(const_self - const_self)
		.def(const_self * other<float32>())
		.def(tostring(self))
		.scope
		[
			def("normalize", &local_vec2::normalize),
			def("dot", &local_vec2::dot),
			def("distance", &local_vec2::distance),
			def("asString", &local_vec2::asString),
			def("magnitude", &local_vec2::magnitude)

		]
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_RENAMED(vec3, Vec3)
		.def(constructor<>())
		.def(constructor<float32, float32, float32>())
		.def_readwrite("x", &vec3::x)
		.def_readwrite("y", &vec3::y)
		.def_readwrite("z", &vec3::z)
		.def(const_self + const_self)
		.def(const_self - const_self)
		.def(const_self * other<float32>())
		.def(tostring(self))
		.scope
		[
			def("normalize", &local_vec3::normalize),
			def("dot", &local_vec3::dot),
			def("distance", &local_vec3::distance),
			def("cross", &local_vec3::cross),
			def("asString", &local_vec3::asString),
			def("magnitude", &local_vec3::magnitude)
		]
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_RENAMED(vec4, Vec4)
		.def(constructor<>())
		.def(constructor<float32, float32, float32, float32>())
		.def_readwrite("x", &vec4::x)
		.def_readwrite("y", &vec4::y)
		.def_readwrite("z", &vec4::z)
		.def_readwrite("w", &vec4::w)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_RENAMED(quat, Quaternion)
		.def(constructor<>())
		.def(constructor<float32, float32, float32, float32>())
		.scope
		[
			def("fromEuler", &local_quat::fromEuler)
		]
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS(PointF)
		.def(constructor<>())
		.def(constructor<const float32&, const float32&>())
		.def_readwrite("x", &PointF::x)
		.def_readwrite("y", &PointF::y)			
		.def_readwrite("w", &PointF::x)
		.def_readwrite("h", &PointF::y)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS(PointI)
		.def(constructor<>())
		.def(constructor<const PointI&>())
		.def(constructor<int32, int32>())
		.def_readwrite("x", &PointI::x)
		.def_readwrite("y", &PointI::y)
		.def_readwrite("w", &PointI::x)
		.def_readwrite("h", &PointI::y)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS(RectI)
		.def(constructor<>())
		.def(constructor<const RectI&>())
		.def(constructor<int32, int32, int32, int32>())
		.def_readwrite("pos", &RectI::pos)
		.def_readwrite("size", &RectI::size)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS(RectF)
		.def(constructor<>())
		.def(constructor<const float32&, const float32&, const float32&, const float32&>())
		.def(constructor<const vec2&, const vec2&>())
		.def(constructor<const Point<float32>&, const Point<float32>&>())
		.def_readwrite("pos", &RectF::pos)
		.def_readwrite("size", &RectF::size)
		.def("offset", (void(RectF::*)(const vec2&)) &Rect<float32>::offset)
		.def("offset", (void(RectF::*)(const Point<float32>&)) &Rect<float32>::offset)
		.def("offset", (void(RectF::*)(const float32&, const float32&)) &Rect<float32>::offset)
		.def("getTop", &Rect<float32>::getTop)
		.def("getBottom", &Rect<float32>::getBottom)
		.def("getLeft", &Rect<float32>::getLeft)
		.def("getRight", &Rect<float32>::getRight)
		.def("getBL", &Rect<float32>::getBL)
		.def("getTL", &Rect<float32>::getTL)
		.def("getTR", &Rect<float32>::getTR)
		.def("getBR", &Rect<float32>::getBR)
		.def("tostring", &Rect<float32>::tostring)
		.scope
		[
			def("contains", &RectF::contains)
		]
	END_DEFINE_LUA_CLASS()

	struct local_ray
	{
		static bool intersectRect(Ray& ray, RectF& rect)
		{
			return rectIntersectLine(ray, rect);
		}
	};
	BEGIN_DEFINE_LUA_CLASS(Ray)
		.def(constructor<>())
		.def(constructor<const vec3&, const vec3&>())
		.def("getOrigin", &Ray::getOrigin)
		.def("getDirection", &Ray::getDirection)
		.def("getPointAt", &Ray::getPointAt)
		.def_readwrite("origin", &Ray::origin)
		.def_readwrite("direction", &Ray::direction)
		.scope
		[
			def("intersectRect", &local_ray::intersectRect)
		]
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS(Plane)
		.def(constructor<>())
		.def(constructor<float32, float32, float32, float32>())
		.def(constructor<const vec3&, const vec3&>())
		.def(constructor<const vec3&, const vec3&, const vec3&>())
		.def(constructor<const vec3&, float32>())
		.def("normal", (vec3&(Plane::*)()) &Plane::normal)
		.def("normalize", &Plane::normalize)
		.def("set", &Plane::set)
		.def("fromPointNormal", &Plane::fromPointNormal)
		.def("fromPoints", &Plane::fromPoints)
		.def("intersects", (bool(Plane::*)(const Ray&, float32&) const) &Plane::intersects)
		.def("intersects", (bool(Plane::*)(const vec3&, const vec3&, float32&) const) &Plane::intersects)
		.def("intersects", (bool(Plane::*)(const Ray&, vec3&) const) &Plane::intersects)
		.def_readwrite("n", &Plane::n)
		.def_readwrite("d", &Plane::d)
		.scope
		[
			def("dot", &Plane::dot)
		]
	END_DEFINE_LUA_CLASS()
}
