#include "PCH.h"

#include "scripting/LuaBindECS.h"
#include "global/PropertySet.h"

#include "gfx/BatchRenderable.h"
#include "gfx/TrailRenderable.h"
#include "gfx/MeshRenderable.h"
#include "gfx/SplineRenderable.h"

#include "scene/ReferenceNode.h"
#include "scene/Actor.h"
#include "scene/Light.h"

#include <luabind/adopt_policy.hpp>
#include <luabind/iterator_policy.hpp>

namespace cs
{
	struct LuaCollisionScriptInstance : CollisionScriptInstance, luabind::wrap_base
	{
		LuaCollisionScriptInstance(Entity* e)
			: CollisionScriptInstance(e) { }

		virtual void onMoved()
		{
            try
            {
                call<void>("onMoved");
            }
            catch (luabind::error& err)
            {
                checkLuaError(err.state(), -1);
            }
		}

		static void default_onMoved(CollisionScriptInstance* ptr)
		{
			return ptr->CollisionScriptInstance::onMoved();
		}

	};
	
	struct LuaScriptComponentInstance : ScriptComponentInstance, luabind::wrap_base
	{
		LuaScriptComponentInstance(Entity* e)
			: ScriptComponentInstance(e) { }
	
		virtual ScriptInstanceState populate(PropertySet& propertySet)
		{
			return CALL_LUA_FUNCTION_RET("populate", ScriptInstanceState, propertySet);
		}

		static ScriptInstanceState default_populate(ScriptComponentInstance* ptr, PropertySet& propertySet)
		{
			return ptr->ScriptComponentInstance::populate(propertySet);
		}

		virtual void process(float32 dt)
		{
			CALL_LUA_FUNCTION("process", dt);
		}

		static void default_process(ScriptComponentInstance* ptr, float32 dt)
		{
			return ptr->ScriptComponentInstance::process(dt);
		}

		virtual void onPress(const vec2& screen_pos)
		{
			CALL_LUA_FUNCTION("onPress", screen_pos);
		}

		static void default_onPress(ScriptComponentInstance* ptr, const vec2& screen_pos)
		{
			return ptr->ScriptComponentInstance::onPress(screen_pos);
		}

		virtual void onRelease(const vec2& screen_pos)
		{
			CALL_LUA_FUNCTION("onRelease", screen_pos);
		}

		static void default_onRelease(ScriptComponentInstance* ptr, const vec2& screen_pos)
		{
			return ptr->ScriptComponentInstance::onRelease(screen_pos);
		}
	};

	using namespace luabind;

	struct LuaEntity
	{
		static bool equals(EntityPtr& a, EntityPtr& b)
		{
			return a.get() == b.get();
		}
		static bool equals(EntityPtr& a, Entity* b)
		{
			return a.get() == b;
		}
		static bool equals(Entity* a, EntityPtr& b)
		{
			return a == b.get();
		}

		static ReferenceNodePtr toReferenceNode(EntityPtr& a)
		{
			return std::static_pointer_cast<ReferenceNode>(a);
		}
		static ActorPtr toActor(EntityPtr& a)
		{
			return std::static_pointer_cast<Actor>(a);
		}
		static LightPtr toLight(EntityPtr& a)
		{
			return std::static_pointer_cast<Light>(a);
		}
	};
    
    BEGIN_DEFINE_LUA_ENUM(ECSSystems)
        .enum_("constants")
        [
            value("Draw", ECSDraw),
            value("Physics", ECSPhysics),
            value("Script", ECSScript),
            value("Animation", ECSAnimation),
            value("Particle", ECSParticle),
            value("Game", ECSGame),
			value("Audio", ECSAudio)
        ]
    END_DEFINE_LUA_ENUM()
    
    ADD_META_BITSET(ECSSystemMask, ECSSystems, ECSMAX);

	BEGIN_DEFINE_LUA_CLASS_SHARED(SceneNode)
		.def("setPosition", &SceneNode::setPosition)
		.def("setRotation", &SceneNode::setRotation)
		.def("setScale", &SceneNode::setScale)
		.def("getPosition", &SceneNode::getPosition)
		.def("getRotation", &SceneNode::getRotation)
		.def("getWorldPosition", &SceneNode::getWorldPosition)
		.def("getWorldRotation", &SceneNode::getWorldRotation)
		.def("setPitch", &SceneNode::setPitch)
		.def("setYaw", &SceneNode::setYaw)
		.def("setRoll", &SceneNode::setRoll)
		.def("addPitch", &SceneNode::addPitch)
		.def("addYaw", &SceneNode::addYaw)
		.def("addRoll", &SceneNode::addRoll)
		.def("getPitch", &SceneNode::getPitch)
		.def("getYaw", &SceneNode::getYaw)
		.def("getRoll", &SceneNode::getRoll)
		.def("resetLocalPosition", &SceneNode::resetLocalPosition)
		.def("resetLocalRotation", &SceneNode::resetLocalRotation)
		.def("resetLocalScale", &SceneNode::resetLocalScale)
		.def("setInitialPosition", &SceneNode::setInitialPosition)
		.def("setInitialRotation", &SceneNode::setInitialRotation)
		.def("setInitialScale", &SceneNode::setInitialScale)

	END_DEFINE_LUA_CLASS()
	
	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(Entity, SceneNode)
		.def(constructor<>())
		.def("setColor", &Entity::setColor)
		.def("getPhysicsComponent", &Entity::getComponent<PhysicsComponent>)
		.def("getScriptComponent", &Entity::getComponent<ScriptComponent>)
		.def("getDrawableComponent", &Entity::getComponent<DrawableComponent>)
		.def("getGameComponent", &Entity::getComponent<GameComponent>)
		.def("getAnimationComponent", &Entity::getComponent<AnimationComponent>)
		.def("getCollisionComponent", &Entity::getComponent<CollisionComponent>)
		.def("getName", &Entity::getName)
		.def("getChild", &Entity::getChild<Entity>)
		.def("removeChild", &Entity::removeChild<Entity>)
		.def("removeChild", &Entity::removeChildByName)
		.def("removeSelf", &Entity::removeSelf)
		.def("getParent", &Entity::getParentEntity)
		.def("getEntity", &Entity::getEntity)
		.def("getEntityAt", &Entity::getEntityAt)
		.def("setEnabled", &Entity::setEnabled)
		.def("setAllEnabled", &Entity::setAllEnabled)
		.def("getEnabled", &Entity::getEnabled)
		.def("onCustomLoad", &Entity::onCustomLoad)
		.def("getNumChildren", &Entity::getNumChildren)
		.def("enableComponents", &Entity::enableComponents)
		.def("disableComponents", &Entity::disableComponents)
		.def("reset", &Entity::reset)
		.def("destroy", &Entity::destroy)
		.def("destroyAll", &Entity::destroyAll)
		.def("getPropertySet", &Entity::getPropertySet)
		.def("setPropertySet", &Entity::setPropertySet)
		.def("setRotationUncoupled", &Entity::setRotationUncoupled)
		.scope
		[
			def("equals", (bool(*)(EntityPtr&, EntityPtr&)) &LuaEntity::equals),
			def("equals", (bool(*)(EntityPtr&, Entity*)) &LuaEntity::equals),
			def("equals", (bool(*)(Entity*, EntityPtr&)) &LuaEntity::equals),
			def("toReferenceNode", &LuaEntity::toReferenceNode),
			def("toLight", &LuaEntity::toLight),
			def("toActor", &LuaEntity::toActor)
		]
	END_DEFINE_LUA_CLASS()

	ADD_SHARED_LIST(EntitySharedList)

	BEGIN_DEFINE_LUA_CLASS(EntitySearchParams)
		.def(constructor<>())
		.def_readwrite("onCheck", &EntitySearchParams::onCheck)
		.def_readwrite("foundList", &EntitySearchParams::foundList)
		.scope
		[
			def("traverseAndCollect", &EntitySearchParams::traverseAndCollect)
		]
	END_DEFINE_LUA_CLASS()
	
	BEGIN_DEFINE_LUA_CLASS_SHARED(AnimationMethod)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(AnimationMethodPath, AnimationMethod)
	END_DEFINE_LUA_CLASS()
		
	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(AnimationMethodValue, AnimationMethod)
		.def(constructor<>())
		.def("addAnimationValue", &AnimationMethodValue::addAnimationValue<AnimationSize>)
		.def("addAnimationValue", &AnimationMethodValue::addAnimationValue<AnimationTextureUV>)
		.def("addAnimationValue", &AnimationMethodValue::addAnimationValue<AnimationColor>)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(Component)
		.def("onPostLoad", &Component::onPostLoad)
		.def("setEnabled", &Component::setEnabled)
		.def("setDisabled", &Component::setDisabled)
		.def("getEnabled", &Component::getEnabled)
		.def("getParent", &Component::getParent)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(PhysicsComponent, Component)
		.def("onNew", &PhysicsComponent::onNew)
		.def("getBody", &PhysicsComponent::getBody)
		.def("clearBody", &PhysicsComponent::clearBody)
		.def("setBody", &PhysicsComponent::setBody)
		.def("getVelocity", &PhysicsComponent::getVelocity)
		.def("setVelocity", &PhysicsComponent::setVelocity)
		.def("setSyncToEntity", &PhysicsComponent::setSyncToEntity)
	END_DEFINE_LUA_CLASS();

	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(AnimationComponent, Component)
		.def(constructor<>())
		.def("setAnimationMethod", &AnimationComponent::setAnimationMethod<AnimationMethodPath>)
		.def("setAnimationMethod", &AnimationComponent::setAnimationMethod<AnimationMethodValue>)
		.def("resetAnimations", &AnimationComponent::resetAnimations)
		.def("clearAnimationMethod", &AnimationComponent::clearAnimationMethod)
		.def_readwrite("onAnimationComplete", &AnimationComponent::onAnimationComplete)
	END_DEFINE_LUA_CLASS();

	BEGIN_DEFINE_LUA_CLASS_SHARED(AnimatedValue)
		.def("setAnimationType", &AnimatedValue::setAnimationType)
	END_DEFINE_LUA_CLASS();

	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(AnimationTextureUV, AnimatedValue)
		.def(constructor<>())
		.def(constructor<const vec2&, const vec2&, const vec2&, const vec2&, float32, float32>())
	END_DEFINE_LUA_CLASS();

	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(AnimationColor, AnimatedValue)
		.def(constructor<>())
		.def(constructor<const ColorB&, const ColorB&, float32, float32>())
	END_DEFINE_LUA_CLASS();

	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(AnimationSize, AnimatedValue)
		.def(constructor<>())
		.def(constructor<const vec2&, const vec2&, float32, float32>())
		.def("setAnimationScaleType", &AnimationSize::setAnimationScaleType)
	END_DEFINE_LUA_CLASS();

	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(CollisionComponent, Component)

	END_DEFINE_LUA_CLASS();

	BEGIN_DEFINE_LUA_ENUM(AnimationScaleType)
		.enum_("constants")
		[
			value("Center", AnimationScaleTypeCenter),
			value("Left", AnimationScaleTypeLeft)
		]
	END_DEFINE_LUA_ENUM()

	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(GameComponent, Component)
		.def("isSelectable", &GameComponent::isSelectable)
		.def("getBehavior", &GameComponent::getBehavior)
	END_DEFINE_LUA_CLASS();

	BEGIN_DEFINE_LUA_ENUM(GameSelectableType)
		.enum_("constants")
		[
			value("None", GameSelectableTypeNone),
			value("Move", GameSelectableTypeMove),
			value("Rotate", GameSelectableTypeRotate)
		]
	END_DEFINE_LUA_ENUM()

	BEGIN_DEFINE_LUA_CLASS_SHARED(GameSelectableBehavior)
		.def_readwrite("onPressed", &GameSelectableBehavior::onPressed)
		.def_readwrite("onMove", &GameSelectableBehavior::onMove)
		.def_readwrite("onReleased", &GameSelectableBehavior::onReleased)
		.def("getType", &GameSelectableBehavior::getType)
	END_DEFINE_LUA_CLASS();

	ADD_SHARED_LIST(GameComponentList)
	
	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(DrawableComponent, Component)
		.def("setRenderable", &DrawableComponent::setRenderable<BatchRenderable>)
		.def("setRenderable", &DrawableComponent::setRenderable<TrailRenderable>)
		.def("setRenderable", &DrawableComponent::setRenderable<MeshRenderable>)
		.def("setRenderable", &DrawableComponent::setRenderable<SplineRenderable>)
		.def("getRenderable", &DrawableComponent::getRenderable)
	END_DEFINE_LUA_CLASS();

	BEGIN_DEFINE_LUA_ENUM(ScriptInstanceState)
		.enum_("constants")
		[
			value("None", ScriptInstanceStateNone),
			value("Retry", ScriptInstanceStateRetry),
			value("Loaded", ScriptInstanceStateLoaded),
			value("Error", ScriptInstanceStateError)
		]
	END_DEFINE_LUA_ENUM()

	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(ScriptComponent, Component)
		.def("getScriptComponentInstance", &ScriptComponent::getScriptComponentInstance)
	END_DEFINE_LUA_CLASS();

	BEGIN_DEFINE_LUA_CLASS_DERIVED_LUA(CollisionScriptInstance, LuaCollisionScriptInstance)
		.def(constructor<Entity*>())
		.def("getEntity", &CollisionScriptInstance::getEntity)
		.def("onMoved", &LuaCollisionScriptInstance::onMoved, &LuaCollisionScriptInstance::default_onMoved)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_DERIVED_LUA(ScriptComponentInstance, LuaScriptComponentInstance)
		.def(constructor<Entity*>())
		.def("init", &LuaScriptComponentInstance::populate, &LuaScriptComponentInstance::default_populate)
		.def("process", &LuaScriptComponentInstance::process, &LuaScriptComponentInstance::default_process)
		.def("onPress", &LuaScriptComponentInstance::onPress, &LuaScriptComponentInstance::default_onPress)
		.def("onRelease", &LuaScriptComponentInstance::onRelease, &LuaScriptComponentInstance::default_onRelease)
		.def("getEntity", &ScriptComponentInstance::getEntity)
	END_DEFINE_LUA_CLASS();

	BEGIN_DEFINE_LUA_CLASS_SHARED(PhysicsSystem)
		.def("addCollisionScriptParticleCallbacks", &PhysicsSystem::addCollisionScriptParticleCallbacks)
		.def("setGravity", &PhysicsSystem::setGravity)
		.scope
		[
			def("getInstance", &PhysicsSystem::getInstance)
		]
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(DrawableSystem)
	
		.scope
		[
			def("getInstance", &DrawableSystem::getInstance)
		]
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(GameSystem)
		.def("getComponentList", &GameSystem::getComponentList<GameComponent>)
		.def("testPoint", &GameSystem::testPoint)
		.def("setPlayer", &GameSystem::setPlayer)
		.def("clearPlayer", &GameSystem::clearPlayer)
		.scope
		[
			def("getInstance", &GameSystem::getInstance)
		]
	END_DEFINE_LUA_CLASS()

	
}
