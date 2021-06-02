#pragma once

#include "ClassDef.h"

#include "physics/PhysicsShape.h"
#include "physics/PhysicsLiquidCollision.h"
#include "physics/PhysicsContact.h"

#include "global/Event.h"


namespace cs
{

	class PhysicsComponent;

	CLASS_DEFINITION_REFLECT(PhysicsBodyType)
	public:
		virtual b2BodyType getType() { return b2_staticBody; }
	};

	CLASS_DEFINITION_DERIVED_REFLECT(PhysicsBodyTypeDynamic, PhysicsBodyType)
	public:
		virtual b2BodyType getType() { return b2_dynamicBody; }
	};

	CLASS_DEFINITION_REFLECT(PhysicsBody)
	public:

		PhysicsBody();
		PhysicsBody(PhysicsShapePtr& s);

		virtual ~PhysicsBody();

		void process(float32 dt) { }
		void updateBody(bool force = false);
		void updateBodyAngle(bool force = false);

		void onShapeParamsChangedCallback();
		void onShapeTypeChangedCallback();
		void onBodyTypeChangedCallback();

		virtual void onNew();
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);

		const b2Vec2& getPosition() const;
		const float32& getAngle() const;

		void setPosition(const b2Vec2& pos, bool force = false);
		void setVelocity(const b2Vec2& vel);
		void setVelocity(float32 x, float32 y);
		void setAngle(const quat& rotation, bool force = false);
		void setDefaultAngle();
		b2Vec2 getVelocity() const;

		void setAngularVelocity(float32 av);
		float32 getAngularVelocity() const;

		bool sync();
		void updateTransform(const Transform& transform);
		bool intersects(b2Vec2& hit);

		void setDensity(float32 d) { this->density = d; }
		void setFriction(float32 f) { this->friction = f; }
		void setOffset(const vec2& off) { this->offset = off; }

		size_t getSelectableShapes(SelectableVolumeList& shapes);

		PhysicsLiquidCollisionPtr& getOnParticleCollision() { return this->onParticleCollision; }
		PhysicsBodyCollisionPtr& getOnBodyCollision() { return this->onBodyCollision; }

		void reloadBody();
		void unloadBody();

		bool isDynamic() const;
		void setDynamic();

		template <class T>
		void setBodyCollision(std::shared_ptr<T>& collision)
		{
			this->onBodyCollision = std::static_pointer_cast<PhysicsBodyCollision>(collision);
		}

		template <class T>
		void setShape(std::shared_ptr<T>& s)
		{
			this->shape = std::static_pointer_cast<PhysicsShape>(s);
			this->onPostLoad();
		}

		Event onBodyTypeChanged;
		Event onShapeParamsChanged;
		Event onShapeTypeChanged;

		void setParentComponent(PhysicsComponent* ptr) { this->parent = ptr; }
		PhysicsComponent* getParentComponent() { return this->parent; }

		bool isActive() const { return this->active; }
		
	protected:

		void createBody();
		void destroyBody();
		
		void createFixtures();
		void destroyFixtures();

		void updateCollision(const Transform& transform);

		PhysicsComponent* parent;
		b2Body* body;
		std::vector<b2Fixture*> fixtures;
		PhysicsBodyTypePtr bodyType;
		vec2 offset;
		
		float32 density;
		float32 friction;
		float32 gravityScale;
		float32 linearDamping;
		float32 angularDamping;
		bool sensor;

		vec2 lastPosition;
		float32 lastAngle;

		PhysicsShapePtr shape;
		PhysicsLiquidCollisionPtr onParticleCollision;
		PhysicsBodyCollisionPtr onBodyCollision;

		b2Vec2 currentPosition;
		float currentAngle;

		bool active;
	};
}