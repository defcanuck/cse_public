#include "PCH.h"

#include "scripting/LuaBindScene.h"

namespace cs
{
	using namespace luabind;
    
    
    BEGIN_DEFINE_LUA_CLASS_RENAMED(HighPrecisionTimer, Timer)
        .def(constructor<>())
        .def("reset", &HighPrecisionTimer::reset)
        .def("getElapsed", &HighPrecisionTimer::getElapsed)
    END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(Behavior)
		.def("setEnabled", &Behavior::setEnabled)
		.def("getEnabled", &Behavior::getEnabled)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(CameraMoveBehavior, Behavior)
		.def(constructor<>())
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(ScriptInputBehavior, Behavior)
		.def(constructor<>())
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(GameInputBehavior, Behavior)
		.def(constructor<>())
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_ENUM(SceneCreateComponent)
	.enum_("constants")
	[
		value("Drawable", SceneCreateComponentDrawable),
		value("Physics", SceneCreateComponentPhysics),
		value("Animation", SceneCreateComponentAnimation)
	]
	END_DEFINE_LUA_ENUM()

	ADD_META_BITSET(SceneCreateComponentMask, SceneCreateComponent, SceneCreateComponentMAX)

	BEGIN_DEFINE_LUA_CLASS_SHARED(SceneCreator)
		.def(constructor<>())
		.def(constructor<SceneDataPtr&>())
		.def(constructor<SceneDataPtr&, EntityPtr&>())
		.def_readwrite("name", &SceneCreator::name)
		.def_readwrite("resource_name", &SceneCreator::resource_name)
		.def_readwrite("position", &SceneCreator::position)
		.def_readwrite("loadFlags", &SceneCreator::loadFlags)
		.def_readwrite("createMask", &SceneCreator::createMask)
		.def_readwrite("lightIndex", &SceneCreator::lightIndex)
		.def("setSceneData", &SceneCreator::setSceneData)
		.def("setParent", &SceneCreator::setParent)
		.scope
		[
			def("createReferenceNode", &SceneCreator::createReferenceNode),
			def("createSprite", &SceneCreator::createSprite),
			def("createLight", &SceneCreator::createLight)
		]
	END_DEFINE_LUA_CLASS()
	
	BEGIN_DEFINE_LUA_ENUM(SceneRender)
	.enum_("constants")
	[
		value("Debug", SceneRenderDebug),
		value("Depth", SceneRenderDepth),
		value("Physics", SceneRenderPhysics)
	]
	END_DEFINE_LUA_ENUM()

	ADD_META_BITSET(SceneRenderMask, SceneRender, SceneRenderMAX)

	BEGIN_DEFINE_LUA_CLASS_SHARED(SceneParams)
		.def(constructor<>())
		.def_readwrite("name", &SceneParams::name)
		.def_readwrite("renderOrder", &SceneParams::renderOrder)
		.def_readwrite("clearColor", &SceneParams::clearColor)
		.def_readwrite("viewport", &SceneParams::viewport)
		.def_readwrite("rttName", &SceneParams::rttName)
		.def_readwrite("rttChannels", &SceneParams::rttChannels)
		.def_readwrite("rttDimm", &SceneParams::rttDimm)
		.def_readwrite("rttDepth", &SceneParams::rttDepth)
		.def_readwrite("forceCreate", &SceneParams::forceCreate)
        .def_readwrite("systemsEnabled", &SceneParams::systemsEnabled)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(SceneTimer)
		.def(constructor<const std::string&, float32, LuaCallbackPtr&>())
		.def("getPercent", &SceneTimer::getPercent)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(Scene)
		.def("setClearColor", &Scene::setClearColor)
		.def("getCamera", &Scene::getCamera)
		.def("getIsActive", &Scene::getIsActive)
		.def("setIsActive", &Scene::setIsActive)
        .def("getIsAnimating", &Scene::getIsAnimating)
        .def("setIsAnimating", &Scene::setIsAnimating)
		.def("addBehavior", &Scene::addBehavior<CameraMoveBehavior>)
		.def("addBehavior", &Scene::addBehavior<SelectionBehavior>)
		.def("addBehavior", &Scene::addBehavior<ScriptInputBehavior>)
		.def("addBehavior", &Scene::addBehavior<GameInputBehavior>)
		.def("setScriptUpdate", &Scene::setScriptUpdate)
		.def("setScriptDraw", &Scene::setScriptDraw)
		.def("clearScriptUpdate", &Scene::clearScriptUpdate)
		.def("clearScriptDraw", &Scene::clearScriptDraw)
		.def("getSceneData", &Scene::getSceneData)
		.def("setSceneSpeed", &Scene::setSceneSpeed)
		.def("getSceneSpeed", &Scene::getSceneSpeed)
		.def("setAnimSpeed", &Scene::setAnimSpeed)
		.def("getAnimSpeed", &Scene::getAnimSpeed)
		.def("animateSceneSpeed", &Scene::animateSceneSpeed)
		.def("addSceneTimer", &Scene::addSceneTimer)
		.def("getRenderTexture", &Scene::getRenderTexture)
		.def("setTint", &Scene::setTint)
		.def("addCopyPass", &Scene::addCopyPass)
        .def("setECSContext", &Scene::setECSContext)
		.def("getFinalResolvePass", &Scene::getFinalResolvePass)
		.def("getFinalResolveViewport", &Scene::getFinalResolveViewport)
		.def_readwrite("renderMask", &Scene::renderMask)
	END_DEFINE_LUA_CLASS()
		
	BEGIN_DEFINE_LUA_CLASS_SHARED(SceneData)
		.def("get", &SceneData::get<Entity>)
		.def("remove", &SceneData::remove<Entity>)
		.def("removeByName", &SceneData::removeByName)
		.def("printStatus", &SceneData::printStatus)
		.def("empty", &SceneData::empty)
		.def("destroy", &SceneData::destroy)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(Actor, Entity)
		
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(Light, Entity)
		.def("setDiffuseIntensity", &Light::setDiffuseIntensity)
		.def("setAmbientIntensity", &Light::setAmbientIntensity)
	END_DEFINE_LUA_CLASS()

	struct LuaReferenceNode
	{
		static EntityPtr toEntity(ReferenceNodePtr& a)
		{
			return std::static_pointer_cast<Entity>(a);
		}
	};

	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(ReferenceNode, Entity)
		.def("getReferenceVolume", &ReferenceNode::getReferenceVolume)
		.def("getReferenceRect", &ReferenceNode::getReferenceRect)
		.def("getReferenceName", &ReferenceNode::getReferenceName)
		.scope
		[
			def("toEntity", &LuaReferenceNode::toEntity),
			def("toggleSpawnChance", &ReferenceNode::toggleSpawnChance)
		]
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(Camera)
		.def("translate", (void(Camera::*)(float32, float32, float32)) &Camera::translate)
		.def("translate", (void(Camera::*)(const vec3&)) &Camera::translate)
		.def("getTranslation", &Camera::getTranslation)
		.def("scale", (void(Camera::*)(float32, float32, float32)) &Camera::scale)
		.def("scale", (void(Camera::*)(const vec3&)) &Camera::scale)
		.def("getScale", &Camera::getScale)
		.def("setNear", &Camera::setNear)
		.def("setFar", &Camera::setFar)
		.def("getRay", (void(Camera::*)(const vec2&, Ray&)) &Camera::getRay)
		.def("getRay", (Ray(Camera::*)(const vec2&)) &Camera::getRay)
		.def("getViewport", &Camera::getViewport)
		.def("getScreenPosition", &Camera::getScreenPosition)
		.def("setViewport", &Camera::setViewport)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(SceneReferenceCache)
	.def("addMapping", &SceneReferenceCache::addMapping)
	.def("clearMapping", &SceneReferenceCache::clearMapping)
	.def("clearReference", &SceneReferenceCache::clearReference)
	.def("clearAllReferences", &SceneReferenceCache::clearAllReferences)
	.def("loadReference", (bool(SceneReferenceCache::*)(const std::string&)) &SceneReferenceCache::loadReference)

	.scope
	[
		def("getInstance", &SceneReferenceCache::getInstance)
	]
	END_DEFINE_LUA_CLASS()
}
