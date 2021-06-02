#include "PCH.h"

#include "scripting/LuaBindGlobal.h"
#include "global/ResourceFactory.h"

namespace cs
{
	using namespace luabind;

	BEGIN_DEFINE_LUA_ENUM(ClickInput)
		.enum_("constants")
		[
			value("Primary", ClickPrimary),
			value("Secondary", ClickSecondary),
			value("Zoom", ClickZoom)
		]
	END_DEFINE_LUA_ENUM()

	BEGIN_DEFINE_LUA_ENUM(TouchState)
		.enum_("constants")
		[
			value("Up", TouchUp),
			value("Down", TouchDown),
			value("Move", TouchMove),
			value("Pressed", TouchPressed),
			value("Released", TouchReleased),
			value("Wheel", TouchWheel)
		]
	END_DEFINE_LUA_ENUM()

	BEGIN_DEFINE_LUA_ENUM(LoadFlags)
		.enum_("constants")
		[ 
			value("Render", LoadFlagsRenderable),
			value("Physics", LoadFlagsPhysics),
			value("Script", LoadFlagsScript)
		]
	END_DEFINE_LUA_ENUM()

	ADD_META_BITSET(LoadFlagMask, LoadFlags, LoadFlagsMAX)
		
	BEGIN_DEFINE_LUA_CLASS_SHARED_RENAMED(LuaCallback, Callback)
		.def(constructor<const luabind::object&>())
		.def(constructor<const luabind::object&, const luabind::object&>())
		.def(constructor<const luabind::object&, const luabind::object&, const luabind::object&>())
		.def(constructor<const luabind::object&, const luabind::object&, const luabind::object&, const luabind::object&>())
		.def(constructor<const luabind::object&, const luabind::object&, const luabind::object&, const luabind::object&, const luabind::object&>())
		.def(constructor<const luabind::object&, const luabind::object&, const luabind::object&, const luabind::object&, const luabind::object&, const luabind::object&>())
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS(Event)
		.def(constructor<>())
		.def("addCallback", (void(Event::*)(LuaCallbackPtr)) &Event::addLuaCallback)
		.def("addCallback", (void(Event::*)(const luabind::object&)) &Event::addCallback)
		.def("addCallback", (void(Event::*)(const luabind::object&, const luabind::object&)) &Event::addCallback)
		.def("addCallback", (void(Event::*)(const luabind::object&, const luabind::object&, const luabind::object&)) &Event::addCallback)
		.def("addCallback", (void(Event::*)(const luabind::object&, const luabind::object&, const luabind::object&, const luabind::object&)) &Event::addCallback)
		.def("invoke", &Event::invoke)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(PropertySetHandle)
		.def(constructor<>())
		.def(constructor<PropertySetResourcePtr&>())
		.def(constructor<const std::string&>())
		.def("getPropertySet", &PropertySetHandle::getPropertySet)
		.def("getPropertySetResource", &PropertySetHandle::getPropertySetResource)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(PropertySetResource)
		.def(constructor<>())
		.def(constructor<const std::string&, const std::string&>())
		.def("save", &PropertySetResource::save)
		.def("update", &PropertySetResource::update)
		.def("getPropertySet", &PropertySetResource::getPropertySet)
	END_DEFINE_LUA_CLASS()

    BEGIN_DEFINE_LUA_CLASS_SHARED(ScriptNotification)
        .def("addListener", &ScriptNotification::addListener)
        .def("removeListener", &ScriptNotification::removeListener)
        .scope
        [
            def("getInstance", &ScriptNotification::getInstance)
        ]
    END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(InputManager)
		.def("setEnabled", &InputManager::setEnabled)
		.scope
		[
			def("getInstance", &InputManager::getInstance)
		]

	END_DEFINE_LUA_CLASS()
		
}
