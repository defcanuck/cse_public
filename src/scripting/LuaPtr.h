#pragma once

#include <luabind/luabind.hpp>

namespace cs
{
	template <class T>
	T* get_pointer(const std::shared_ptr<T>& ptr)
	{
		return ptr.get();
	}

	template<class T>
	luabind::detail::class_base* createClass()
	{
		assert(false);
		return luabind::class_<T>("ERROR!");
	}
}