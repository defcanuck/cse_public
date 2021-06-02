#pragma once

#include "scripting/LuaMacro.h"

#include "physics/PhysicsContact.h"
#include "physics/PhysicsBody.h"

namespace cs
{
	PROTO_LUA_CLASS(PhysicsLiquidContactData);
	PROTO_LUA_CLASS(PhysicsBody);
	PROTO_LUA_CLASS(PhysicsBodyCollision);
	PROTO_LUA_CLASS(PhysicsBodyCollisionScript);
	PROTO_LUA_CLASS(PhysicsShape);
	PROTO_LUA_CLASS(PhysicsShapeBox);
}
