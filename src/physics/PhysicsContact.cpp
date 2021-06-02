#include "PCH.h"

#include "physics/PhysicsContact.h"
#include "physics/PhysicsBody.h"
#include "liquid/LiquidGroup.h"
#include "ecs/comp/PhysicsComponent.h"

namespace cs
{

	BEGIN_META_CLASS(PhysicsBodyCollision)
	END_META()

	BEGIN_META_CLASS(PhysicsBodyCollisionScript)
		ADD_MEMBER(tag);
	END_META()

	bool PhysicsContact::gIgnoreScriptEvents = false;

	bool getEntitiesForBody(PhysicsBody* body, Entity** entity)
	{
		PhysicsComponent* component = body->getParentComponent();
		if (!component)
		{
			log::error("No parent component for Physics Body - that's bad right?");
			return false;
		}
			
		*entity = component->getParent();
		if (!(*entity))
		{
			log::error("No parent entity for physics component!");
			return false;
		}
		return true;
	}

	void PhysicsBodyCollisionScript::onCollisionBegin(PhysicsBody* thisBody, PhysicsBody* otherBody)
	{
		if (this->onCollide.get())
		{
			Entity* thisEntity;
			Entity* otherEntity;
			if (!getEntitiesForBody(thisBody, &thisEntity) || !getEntitiesForBody(otherBody, &otherEntity))
				return;

			(*this->onCollide)(thisEntity, otherEntity);
		}
	}

	void PhysicsBodyCollisionScript::onCollisionEnd(PhysicsBody* thisBody, PhysicsBody* otherBody)
	{
		if (this->onExit.get())
		{
			Entity* thisEntity;
			Entity* otherEntity;
			if (!getEntitiesForBody(thisBody, &thisEntity) || !getEntitiesForBody(otherBody, &otherEntity))
				return;

			(*this->onExit)(thisEntity, otherEntity);
		}
	}

	void PhysicsContact::BeginContact(b2Contact* contact)
	{

		PhysicsBody* bodyA = reinterpret_cast<PhysicsBody*>(contact->GetFixtureA()->GetBody()->GetUserData());
		PhysicsBody* bodyB = reinterpret_cast<PhysicsBody*>(contact->GetFixtureB()->GetBody()->GetUserData());
		
		PhysicsBodyCollisionPtr bodyCollisionA = bodyA->getOnBodyCollision();
		if (bodyCollisionA.get())
		{
			PhysicsBodyCollision::CollisionPair pair((uintptr_t)bodyA, (uintptr_t)bodyB);
			if (!this->history.count(pair))
			{
				bodyCollisionA->onCollisionBegin(bodyA, bodyB);
				this->history.insert(pair);
			}
		}
			
		PhysicsBodyCollisionPtr bodyCollisionB = bodyB->getOnBodyCollision();
		if (bodyCollisionB.get())
		{
			
			PhysicsBodyCollision::CollisionPair pair((uintptr_t)bodyB, (uintptr_t)bodyA);
			if (!this->history.count(pair))
			{
				bodyCollisionB->onCollisionBegin(bodyB, bodyA);
				this->history.insert(pair);
			}
		}
	}
	
	void PhysicsContact::EndContact(b2Contact* contact)
	{
		// Kill any and all contact events b/c we don't care
		if (PhysicsContact::gIgnoreScriptEvents)
		{
			this->history.clear();
			return;
		}

		PhysicsBody* bodyA = reinterpret_cast<PhysicsBody*>(contact->GetFixtureA()->GetBody()->GetUserData());
		PhysicsBody* bodyB = reinterpret_cast<PhysicsBody*>(contact->GetFixtureB()->GetBody()->GetUserData());

		PhysicsBodyCollisionPtr bodyCollisionA = bodyA->getOnBodyCollision();
		if (bodyCollisionA.get())
		{
			PhysicsBodyCollision::CollisionPair pair((uintptr_t)bodyA, (uintptr_t)bodyB);
			PhysicsBodyCollision::CollisionHistory::iterator it = this->history.find(pair);
			if (it != this->history.end())
			{
				if (bodyA->isActive())
				{
					bodyCollisionA->onCollisionEnd(bodyA, bodyB);
				}
				this->history.erase(it);
			}
		}

		PhysicsBodyCollisionPtr bodyCollisionB = bodyB->getOnBodyCollision();
		if (bodyCollisionB.get())
		{
			PhysicsBodyCollision::CollisionPair pair((uintptr_t)bodyB, (uintptr_t)bodyA);
			PhysicsBodyCollision::CollisionHistory::iterator it = this->history.find(pair);
			if (it != this->history.end())
			{
				if (bodyB->isActive())
				{
					bodyCollisionB->onCollisionEnd(bodyB, bodyA);
				}
				this->history.erase(it);
			}
		}
	}
	
	void PhysicsContact::BeginContact(b2ParticleSystem* particleSystem,
		b2ParticleBodyContact* particleBodyContact)
	{

		int32 index = particleBodyContact->index;
		const b2ParticleHandle* handle = particleSystem->GetParticleHandleFromIndex(index);

		PhysicsBody* body = reinterpret_cast<PhysicsBody*>(particleBodyContact->body->GetUserData());
		assert(body);
		body->getOnParticleCollision()->onCollide(particleSystem, handle);
	}
	
	void PhysicsContact::EndContact(b2Fixture* fixture,
		b2ParticleSystem* particleSystem, int32 index)
	{
		// log::info("EndContact!b2ParticleSystem");
	}	
}