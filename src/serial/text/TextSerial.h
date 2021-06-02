#pragma once

#include <iostream>

#include "serial/RefVariant.h"
#include "math/GLM.h"
#include "math/Rect.h"
#include "gfx/Color.h"
#include "global/Utils.h"
#include "gfx/Types.h"

#define TEXT_SERIALIZE(type) \
	template <> \
	void serializePrim<type>(std::ostream& oss, RefVariant prim)	

#define TEXT_SERIALIZE_BITFIELD_IMPL(type) \
	template <> \
	void serializePrim<type>(std::ostream& oss, RefVariant prim) \
	{ \
		type& uival = *reinterpret_cast<type*>(prim.getData()); \
		oss << uival; \
	}

#define TEXT_DESERIALIZE(type) \
	template <> \
	void deserializePrim<type>(JsonValue value, RefVariant prim) \

#define TEXT_DESERIALIZE_BITFIELD_IMPL(type) \
	template <> \
	void deserializePrim<type>(JsonValue value, RefVariant prim) \
	{ \
		type& dst = *reinterpret_cast<type*>(prim.getData()); \
		dst = type(size_t(value.toNumber())); \
	}

namespace cs
{
	namespace text
	{
		extern unsigned level;
		extern const std::string kClassTypeStr;

		extern const std::string kTrue;
		extern const std::string kFalse;

	}
}