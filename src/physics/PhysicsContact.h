#pragma once

#include "ClassDef.h"

#include <list>
#include <set>
#include <functional>

#include "Box2D/Box2D.h"
#include "os/LogManager.h"
#include "scripting/LuaCallback.h"

namespace cs
{
	class PhysicsBody;

	CLASS_DEFINITION_REFLECT(PhysicsBodyCollision)
	public:

		typedef std::pair<uintptr_t, uintptr_t> CollisionPair;
		typedef std::set<CollisionPair> CollisionHistory;

		
		PhysicsBodyCollision() { }
		virtual ~PhysicsBodyCollision() { }

		virtual void onCollisionBegin(PhysicsBody* thisBody, PhysicsBody* otherBody) { }
		virtual void onCollisionEnd(PhysicsBody* thisBody, PhysicsBody* otherBody) { }

		
	};

	CLASS_DEFINITION_DERIVED_REFLECT(PhysicsBodyCollisionScript, PhysicsBodyCollision)
	public:

		PhysicsBodyCollisionScript() 
			: tag("")
		{ }

		virtual void onCollisionBegin(PhysicsBody* thisBody, PhysicsBody* otherBody);
		virtual void onCollisionEnd(PhysicsBody* thisBody, PhysicsBody* otherBody);

		
		std::string tag;
		LuaCallbackPtr onCollide;
		LuaCallbackPtr onExit;
		
	};

	class PhysicsLiquidContactData
	{
	public:
		PhysicsLiquidContactData() { }
		virtual ~PhysicsLiquidContactData() { }
	};

	class PhysicsContact : public b2ContactListener
	{
	public:

		static bool gIgnoreScriptEvents;

		virtual void BeginContact(b2Contact* contact);
		virtual void EndContact(b2Contact* contact);

		virtual void BeginContact(b2ParticleSystem* particleSystem,
			b2ParticleBodyContact* particleBodyContact);
		virtual void EndContact(b2Fixture* fixture,
			b2ParticleSystem* particleSystem, int32 index);

		PhysicsBodyCollision::CollisionHistory history;

	};
}
