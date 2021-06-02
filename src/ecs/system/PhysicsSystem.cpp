#include "PCH.h"

#include "ecs/system/PhysicsSystem.h"
#include "ecs/ECS_Utils.h"

#include "ecs/comp/PhysicsComponent.h"
#include "ecs/comp/LiquidComponent.h"
#include "physics/PhysicsContact.h"

namespace cs
{
	const float32 kDefaultGravityX = 0.0f;
	const float32 kDefaultGravityY = -10.0f;

	PhysicsSystem::PhysicsSystem(ECSContext* cxt) 
		: BaseSystem(cxt)
		, xGravity(0.0f)
		, yGravity(0.0f)
		, world(nullptr)
	{
		this->subscribeForComponent<PhysicsComponent>(this->parentContext);
		this->subscribeForComponent<LiquidComponent>(this->parentContext);

		b2Vec2 gravity(kDefaultGravityX, kDefaultGravityY);
		world = new b2World(gravity);
		
		this->world->SetContactListener((b2ContactListener*) &this->contactListener);

		this->setGravity(kDefaultGravityX, kDefaultGravityY);
		
	}

	PhysicsSystem::~PhysicsSystem()
	{
        this->removeComponentSubscription<PhysicsComponent>(this->parentContext);
        this->removeComponentSubscription<LiquidComponent>(this->parentContext);
        
		this->world->SetContactListener(nullptr);
	}

	void PhysicsSystem::setGravity(float xGrav, float yGrav)
	{
		if (xGrav == xGravity && yGrav == yGravity)
			return;

		this->xGravity = xGrav;
		this->yGravity = yGrav;
		b2Vec2 gravity(xGravity, yGravity);
		world->SetGravity(gravity);
	}

	void PhysicsSystem::processImpl(SystemUpdateParams* params)
	{

		float32 adjusted_dt = params->updateDt;

		int32 velocityIterations = 6;
		int32 positionIterations = 2;

		world->Step(adjusted_dt, velocityIterations, positionIterations);
		
		world->ClearForces();

		BaseSystem::ComponentIdMap physics;
		this->getEnabledComponents<PhysicsComponent>(physics);
		for (const auto it : physics)
		{
			const PhysicsComponentPtr& phys =
				std::static_pointer_cast<PhysicsComponent>(it.second);

			SceneNode* self_node = it.second->getParent();

			// Sync dynamic physics components
			if (phys->sync())
			{
				vec3 world_position = phys->getWorldPosition();
				quat world_rotation = phys->getWorldRotation();

				self_node->setParentOverride(true);
				self_node->setCurrentPosition(world_position, SceneNode::UpdateTypePhysics);
				self_node->setCurrentRotation(world_rotation, SceneNode::UpdateTypePhysics);
			}
		}

		BaseSystem::ComponentIdMap particles;
		this->getEnabledComponents<LiquidComponent>(particles);
		for (const auto it : particles)
		{
			const LiquidComponentPtr& particle =
				std::static_pointer_cast<LiquidComponent>(it.second);

			particle->process(adjusted_dt);
		}
	}

	void PhysicsSystem::addCollisionScriptParticleCallbacks(const std::string& tag, LuaCallbackPtr& func)
	{
		OnCollisionScriptParticleCallbacks::iterator it = this->scriptParticleCallbacks.find(tag);
		if (it != this->scriptParticleCallbacks.end())
		{
			log::info("Duplicate Script Event in the Physics Cache!");
			return;
		}

		this->scriptParticleCallbacks[tag] = func;
	}

	bool PhysicsSystem::invokeOnCollisionScriptParticleCallbacks(const std::string& tag, PhysicsLiquidContactData& data)
	{
		OnCollisionScriptParticleCallbacks::iterator it = this->scriptParticleCallbacks.find(tag);
		if (it == this->scriptParticleCallbacks.end())
		{
			// Default to killing particles if we don't have a way to handle them
			log::info("No callback registered for tag ", tag);
			return true;
		}

		return it->second->call<bool>(data);
	}
}
