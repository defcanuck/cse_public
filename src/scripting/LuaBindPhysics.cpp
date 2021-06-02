#include "PCH.h"

#include "scripting/LuaBindPhysics.h"

#include <string>

namespace cs
{

	using namespace luabind;

	BEGIN_DEFINE_LUA_CLASS(PhysicsLiquidContactData)
		.def(constructor<>())
		END_DEFINE_LUA_CLASS()

		BEGIN_DEFINE_LUA_CLASS_SHARED(PhysicsBody)
		.def("setDynamic", &PhysicsBody::setDynamic)
		.def("setBodyCollision", (void(PhysicsBody::*)(std::shared_ptr<PhysicsBodyCollision>&)) &PhysicsBody::setBodyCollision<PhysicsBodyCollision>)
		.def("setBodyCollision", (void(PhysicsBody::*)(std::shared_ptr<PhysicsBodyCollisionScript>&)) &PhysicsBody::setBodyCollision<PhysicsBodyCollisionScript>)
		.def("setShape", (void(PhysicsBody::*)(std::shared_ptr<PhysicsShape>&)) &PhysicsBody::setShape<PhysicsShape>)
		.def("setShape", (void(PhysicsBody::*)(std::shared_ptr<PhysicsShapeBox>&)) &PhysicsBody::setShape<PhysicsShapeBox>)
		.def("setOffset", &PhysicsBody::setOffset)
		.def("reloadBody", &PhysicsBody::reloadBody)
		.def("setAngularVelocity", &PhysicsBody::setAngularVelocity)
		.def("getAngularVelocity", &PhysicsBody::getAngularVelocity)
		.def("setVelocity", (void(PhysicsBody::*)(float32, float32)) &PhysicsBody::setVelocity)
		.def("setDefaultAngle", &PhysicsBody::setDefaultAngle)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(PhysicsBodyCollision)
		.def(constructor<>())
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(PhysicsBodyCollisionScript, PhysicsBodyCollision)
		.def(constructor<>())
		.def_readwrite("onCollide", &PhysicsBodyCollisionScript::onCollide)
		.def_readwrite("onExit", &PhysicsBodyCollisionScript::onExit)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(PhysicsShape)
		.def(constructor<>())
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(PhysicsShapeBox)
		.def(constructor<>())
		.def(constructor<const vec2&>())
		.def(constructor<const PointF&>())
		.def(constructor<QuadVolumePtr&>())
	END_DEFINE_LUA_CLASS()
}