#include "PCH.h"

#include "scripting/LuaBindGame.h"

#include "game/GameInterface.h"

namespace cs
{

	using namespace luabind;

	BEGIN_DEFINE_LUA_CLASS_SHARED(GameInterface)
		.def("getTitle", &GameInterface::getTitle)
		.def("getUI", &GameInterface::getUI)
		.def("getScene", &GameInterface::getScene)
		.def("getResolution", &GameInterface::getResolution)
		.def("getContext", &GameInterface::getContext)

		.def("loadScene", &GameInterface::loadScene)
		
		.scope
		[
			def("getInstance", &GameInterface::getInstance)
		]
	END_DEFINE_LUA_CLASS()
	
	BEGIN_DEFINE_LUA_CLASS_SHARED(Context)
		.def("getScene", &Context::getScene)
		.def("clearMainScene", &Context::clearMainScene)
		.def_readwrite("onScriptUpdate", &Context::onScriptUpdate)
		.def("addScene", &Context::addScene)
		.def("removeScene", &Context::removeScene)
		.def("getAspectRatio", &Context::getAspectRatio)
		.def("getContentScale", &Context::getContentScale)
		.def("getUIView", &Context::getUIView)
		.def("getNumActiveScenes", &Context::getNumActiveScenes)
        .def("resetUI", &Context::resetUI)
		.def("setFinalResolveShader", &Context::setFinalResolveShader)
	END_DEFINE_LUA_CLASS()
}
