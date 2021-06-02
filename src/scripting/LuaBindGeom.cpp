#include "PCH.h"

#include "scripting/LuaBindGeom.h"

#include <luabind/adopt_policy.hpp>
#include <luabind/iterator_policy.hpp>

namespace cs
{
	using namespace luabind;

	DEFINE_DERIVED_VALUES_1(Volume, QuadVolume)
	BEGIN_DEFINE_LUA_CLASS_SHARED(Volume)
		.def("getRect", &Volume::getRect)
		SET_DERIVED_VALUES_1(Volume, QuadVolume)
	END_DEFINE_LUA_CLASS()

	DEFINE_INHERITED_VALUES(QuadVolume, Volume)
	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(QuadVolume, Volume)
		
		SET_INHERITED_VALUES(QuadVolume, Volume)
	END_DEFINE_LUA_CLASS()
}