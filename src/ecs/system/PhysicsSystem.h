#pragma once

#include "ecs/system/BaseSystem.h"

#include <Box2D/Box2D.h>
#include "physics/PhysicsContact.h"

namespace cs
{
	typedef std::map<std::string, LuaCallbackPtr> OnCollisionScriptParticleCallbacks;

	class PhysicsSystem : public ECSContextSystemBase<PhysicsSystem, ECSPhysics>, public BaseSystem
	{
	public:
		PhysicsSystem(ECSContext* cxt);
		virtual ~PhysicsSystem();

		virtual void processImpl(SystemUpdateParams* params);
		virtual void draw(CameraPtr& camera) { }

		void setGravity(float xGrav, float yGrav);

		b2World& getWorld() { return *world; }

		void addCollisionScriptParticleCallbacks(const std::string& tag, LuaCallbackPtr& func);
		bool invokeOnCollisionScriptParticleCallbacks(const std::string& tag, PhysicsLiquidContactData& data);

		PhysicsContact& getContactListener() { return this->contactListener; }

	private:

		b2World* world;

		PhysicsContact contactListener;
		
		float xGravity;
		float yGravity;

		OnCollisionScriptParticleCallbacks scriptParticleCallbacks;
	};
}