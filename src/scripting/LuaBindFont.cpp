#include "PCH.h"

#include "scripting/LuaBindFont.h"
#include "scripting/LuaMacro.h"
#include "scripting/LuaConverter.h"

#include <string>

#include <luabind/iterator_policy.hpp>

namespace cs
{

	using namespace luabind;

	BEGIN_DEFINE_LUA_CLASS_SHARED(Font)
		.def("getCharacterHandle", (TextureHandlePtr(Font::*)(const std::string&, int32)) &Font::getCharacterHandle)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(FontManager)
		.def("getDiagnostics", &FontManager::getDiagnostics)
		.def("preload", &FontManager::preload)
		.def("setDoubleSpace", &FontManager::setDoubleSpace)
		.def("getFont", &FontManager::getFont)
		.scope
		[
			def("getInstance", &FontManager::getInstance)
		]
	END_DEFINE_LUA_CLASS()
}