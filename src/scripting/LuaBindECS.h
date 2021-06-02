#pragma once

#include "scripting/LuaMacro.h"

#include "ecs/Entity.h"
#include "ecs/EntitySearch.h"

#include "ecs/comp/ScriptComponent.h"
#include "ecs/comp/DrawableComponent.h"
#include "ecs/comp/PhysicsComponent.h"
#include "ecs/comp/GameComponent.h"
#include "ecs/comp/AnimationComponent.h"
#include "ecs/comp/CollisionComponent.h"

#include "ecs/system/DrawableSystem.h"
#include "ecs/system/PhysicsSystem.h"
#include "ecs/system/GameSystem.h"

namespace cs
{
    PROTO_LUA_CLASS(ECSSystems);
    PROTO_LUA_CLASS(ECSSystemMask);
	PROTO_LUA_CLASS(SceneNode);
	PROTO_LUA_CLASS(Entity);
	PROTO_LUA_CLASS(Component);
	PROTO_LUA_CLASS(EntitySearchParams);
	PROTO_LUA_CLASS(EntitySharedList);

	PROTO_LUA_CLASS(DrawableSystem);
	PROTO_LUA_CLASS(DrawableComponent);

	PROTO_LUA_CLASS(PhysicsSystem);
	PROTO_LUA_CLASS(PhysicsComponent);

	PROTO_LUA_CLASS(AnimationComponent);
	PROTO_LUA_CLASS(AnimationMethod);
	PROTO_LUA_CLASS(AnimationMethodPath);
	PROTO_LUA_CLASS(AnimationMethodValue);
	PROTO_LUA_CLASS(AnimatedValue);
	PROTO_LUA_CLASS(AnimationSize);
	PROTO_LUA_CLASS(AnimationTextureUV);
	PROTO_LUA_CLASS(AnimationColor);
	PROTO_LUA_CLASS(AnimationScaleType);

	PROTO_LUA_CLASS(GameSystem);
	PROTO_LUA_CLASS(GameComponent);
	PROTO_LUA_CLASS(CollisionComponent);
	PROTO_LUA_CLASS(GameComponentList);
	PROTO_LUA_CLASS(GameSelectableBehavior);
	PROTO_LUA_CLASS(GameSelectableType);

	PROTO_LUA_CLASS(ScriptInstanceState);
	PROTO_LUA_CLASS(ScriptComponent);
	PROTO_LUA_CLASS(ScriptComponentInstance);
	PROTO_LUA_CLASS(CollisionScriptInstance);



}
