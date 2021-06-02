#include "PCH.h"

#include "physics/PhysicsBody.h"
#include "ecs/system/PhysicsSystem.h"

namespace cs
{

	const ColorB kSelectableBodyColor = ColorB::White;

	BEGIN_META_CLASS(PhysicsBodyType)
		END_META();

	BEGIN_META_CLASS(PhysicsBodyTypeDynamic)
		END_META();

	BEGIN_META_CLASS(PhysicsBody)
		ADD_MEMBER(lastPosition);
			SET_MEMBER_IGNORE_SERIALIZATION();
		ADD_MEMBER(lastAngle);
			SET_MEMBER_IGNORE_SERIALIZATION();

		//..........
		ADD_MEMBER_PTR(bodyType);
		SET_MEMBER_CALLBACK_POST(&PhysicsBody::onBodyTypeChangedCallback);
			ADD_COMBO_META_LABEL(PhysicsBodyType, "Static");
			ADD_COMBO_META_LABEL(PhysicsBodyTypeDynamic, "Dynamic");
		ADD_MEMBER(offset);
			SET_MEMBER_DEFAULT(kZero2);
		ADD_MEMBER(density);
			SET_MEMBER_DEFAULT(1.0f);
			SET_MEMBER_MIN(0.0f);
			SET_MEMBER_MAX(10.0f);
			SET_MEMBER_CALLBACK_POST(&PhysicsBody::reloadBody);
		ADD_MEMBER(friction);
			SET_MEMBER_DEFAULT(0.0f);
			SET_MEMBER_MIN(0.0f);
			SET_MEMBER_MAX(10.0f);
			SET_MEMBER_CALLBACK_POST(&PhysicsBody::reloadBody);
		ADD_MEMBER(gravityScale);
			SET_MEMBER_DEFAULT(1.0f);
			SET_MEMBER_MIN(0.0f);
			SET_MEMBER_MAX(10.0f);
			SET_MEMBER_CALLBACK_POST(&PhysicsBody::reloadBody);	
		ADD_MEMBER(linearDamping);
			SET_MEMBER_DEFAULT(0.0f);
			SET_MEMBER_MIN(0.0f);
			SET_MEMBER_MAX(1.0f);
			SET_MEMBER_CALLBACK_POST(&PhysicsBody::reloadBody);
		ADD_MEMBER(angularDamping);
			SET_MEMBER_DEFAULT(0.1f);
			SET_MEMBER_MIN(0.0f);
			SET_MEMBER_MAX(1.0f);
			SET_MEMBER_CALLBACK_POST(&PhysicsBody::reloadBody);
		ADD_MEMBER(sensor);
			SET_MEMBER_CALLBACK_POST(&PhysicsBody::reloadBody);
		ADD_MEMBER_PTR(shape);
			SET_MEMBER_CALLBACK_POST(&PhysicsBody::onShapeTypeChangedCallback);
			ADD_COMBO_META_LABEL(PhysicsShapeBox, "Box");
			ADD_COMBO_META_LABEL(PhysicsShapeMesh, "Mesh");
			ADD_COMBO_META_LABEL(PhysicsShapeTriangle, "Triangle");
			ADD_COMBO_META_LABEL(PhysicsShapeCircle, "Circle");
		ADD_MEMBER_PTR(onParticleCollision);
			ADD_COMBO_META_LABEL(PhysicsLiquidCollision, "None");
			ADD_COMBO_META_LABEL(PhysicsLiquidCollisionKill, "Kill");
			ADD_COMBO_META_LABEL(PhysicsLiquidCollisionWarp, "Warp");
			ADD_COMBO_META_LABEL(PhysicsLiquidCollisionScript, "Script");
		
	END_META()

	PhysicsBody::PhysicsBody()
		: parent(nullptr)
		, body(nullptr)
		, bodyType(CREATE_CLASS(PhysicsBodyType))
		, offset(kZero2)
		, density(1.0f)
		, friction(0.0f)
		, gravityScale(1.0f)
		, linearDamping(0.0f)
		, angularDamping(0.1f)
		, sensor(false)
		, shape(std::static_pointer_cast<PhysicsShape>(CREATE_CLASS(PhysicsShapeBox)))
		, onParticleCollision(CREATE_CLASS(PhysicsLiquidCollision))
		, onBodyCollision(CREATE_CLASS(PhysicsBodyCollision))
		, active(true)
		, lastPosition(kZero2)
		, lastAngle(0.0f)
	{
	
	}

	PhysicsBody::PhysicsBody(PhysicsShapePtr& s)
		: parent(nullptr)
		, body(nullptr)
		, bodyType(CREATE_CLASS(PhysicsBodyType))
		, offset(kZero2)
		, density(1.0f)
		, friction(0.0f)
		, gravityScale(1.0f)
		, linearDamping(0.0f)
		, angularDamping(0.1f)
		, sensor(false)
		, shape(s)
		, onParticleCollision(CREATE_CLASS(PhysicsLiquidCollision))
		, onBodyCollision(CREATE_CLASS(PhysicsBodyCollision))
		, active(true)
		, lastPosition(kZero2)
		, lastAngle(0.0f)
	{
		assert(this->shape.get());
	}

	PhysicsBody::~PhysicsBody()
	{
		if (this->parent)
		{
			this->destroyFixtures();
			this->destroyBody();
		}
	}

	const b2Vec2& PhysicsBody::getPosition() const
	{
		return this->currentPosition;
	}

	const float32& PhysicsBody::getAngle() const
	{
		return this->currentAngle;
	}


	bool PhysicsBody::isDynamic() const
	{
		return this->bodyType.get() && this->bodyType->getType() == b2_dynamicBody;
	}

	void PhysicsBody::setVelocity(const b2Vec2& vel)
	{
		if (!this->body || this->bodyType->getType() != b2_dynamicBody)
		{
			return;
		}

		this->body->SetLinearVelocity(vel);
	}

	void PhysicsBody::setVelocity(float32 x, float32 y)
	{
		b2Vec2 velocity(x, y);
		this->setVelocity(velocity);
	}


	void PhysicsBody::setAngularVelocity(float32 av)
	{
		if (!this->body || this->bodyType->getType() != b2_dynamicBody)
		{
			return;
		}

		this->body->SetAngularVelocity(av);
	}

	float32 PhysicsBody::getAngularVelocity() const
	{
		if (!this->body || this->bodyType->getType() != b2_dynamicBody)
		{
			return 0.0f;
		}
		return this->body->GetAngularVelocity();
	}

	b2Vec2 PhysicsBody::getVelocity() const
	{
		if (!this->body || this->bodyType->getType() != b2_dynamicBody)
		{
			return b2Vec2(0.0f, 0.0f);
		}

		return this->body->GetLinearVelocity();
	}

	void PhysicsBody::setPosition(const b2Vec2& pos, bool force)
	{
		if (pos.x != this->currentPosition.x ||
			pos.y != this->currentPosition.y ||
			force)
		{
			this->currentPosition = pos + b2Vec2(this->offset.x, this->offset.y);
			this->updateBody(force);
		}
	}

	void PhysicsBody::setAngle(const quat& rotation, bool force)
	{
		float angle = 2.0f * acos(rotation.w) * ((rotation.z < 0.0f) ? -1.0f : 1.0f);
		if (this->currentAngle != angle || force)
		{
			this->currentAngle = angle;
			this->updateBodyAngle(force);
		}
	}

	void PhysicsBody::setDynamic()
	{
		this->bodyType = CREATE_CLASS(PhysicsBodyTypeDynamic);
		this->onBodyTypeChangedCallback();
	}

	void PhysicsBody::onBodyTypeChangedCallback()
	{
		this->onPostLoad();
		this->onBodyTypeChanged.invoke();
	}

	void PhysicsBody::onShapeTypeChangedCallback()
	{
		this->createFixtures();
		this->onShapeTypeChanged.invoke();
	}

	void PhysicsBody::createBody()
	{
		b2World& world = PhysicsSystem::getInstance()->getWorld();

		b2BodyDef bodyDef;
		bodyDef.type = this->bodyType->getType();
		
		bodyDef.position.Set(
			PhysicsConst::worldToBox2D(this->currentPosition.x + this->offset.x),
			PhysicsConst::worldToBox2D(this->currentPosition.y + this->offset.y)
		);

		bodyDef.angle = this->currentAngle;
		bodyDef.gravityScale = this->gravityScale;
		bodyDef.angularDamping = this->angularDamping;
		bodyDef.linearDamping = this->linearDamping;
		
		body = world.CreateBody(&bodyDef);

		if (body)
		{
			body->SetUserData(this);
		}

	}

	void PhysicsBody::reloadBody()
	{
		this->unloadBody();

		this->createBody();
		this->createFixtures();
		this->active = true;
	}

	void PhysicsBody::unloadBody()
	{
		this->active = false;
		this->destroyFixtures();
		this->destroyBody();
	}

	void PhysicsBody::onNew()
	{
		if (!this->shape)
		{
			log::error("No shape defined!");
			return;
		}

		this->shape->onChanged += createCallbackArg0(&PhysicsBody::onShapeParamsChangedCallback, this);
		this->shape->onPostLoad();

		this->reloadBody();
	}

	void PhysicsBody::onPostLoad(const LoadFlagMask& flags)
	{
		if (!this->shape)
		{
			log::error("No shape defined!");
			return;
		}

		// hack friction to one
		this->friction = 1.0f;

		this->shape->onChanged += createCallbackArg0(&PhysicsBody::onShapeParamsChangedCallback, this);
		this->shape->onPostLoad(flags);

		this->reloadBody();

		if (this->onParticleCollision.get())
			this->onParticleCollision->onPostLoad(flags);
	}

	void PhysicsBody::onShapeParamsChangedCallback()
	{
		this->reloadBody();
		this->onShapeParamsChanged.invoke();
	}

	void PhysicsBody::destroyBody()
	{
		if (this->body)
		{
			PhysicsSystem* sys = PhysicsSystem::getInstance();
			b2World& world = sys->getWorld();
			if (world.GetBodyCount() > 0)
			{
				world.DestroyBody(this->body);
				this->body = nullptr;
			}
			else
			{
				log::warning("No body in world?  Strange!");
                this->body = nullptr;
			}
		}
	}

	void PhysicsBody::setDefaultAngle()
	{
		quat rot(0.0f, 0.0f, 0.0f, 1.0f);
		this->setAngle(rot, true);
		this->currentAngle = 0.0f;
	}

	void PhysicsBody::destroyFixtures()
	{
		for (size_t i = 0; i < this->fixtures.size(); ++i)
		{
			b2Fixture* fix = this->fixtures[i];
			body->DestroyFixture(fix);
		}
		this->fixtures.clear();
	}

	void PhysicsBody::createFixtures()
	{
		if (!this->body)
			return;

		this->destroyFixtures();

		PhysicsShapeCollection shapes;
		if (this->shape->createPolygon(this->body, shapes))
		{
			for (auto it : shapes.list)
			{
				b2FixtureDef fixtureDef;
				fixtureDef.shape = it;
				fixtureDef.density = this->density;
				fixtureDef.friction = this->friction;
				fixtureDef.isSensor = this->sensor;
				this->fixtures.push_back(body->CreateFixture(&fixtureDef));
			}
		}
	}

	bool PhysicsBody::sync()
	{
		if (!this->body || this->bodyType->getType() == b2_staticBody)
			return false;

		this->currentPosition = this->body->GetPosition();
		this->currentPosition.x = PhysicsConst::box2DToWorld(this->currentPosition.x) - this->offset.x;
		this->currentPosition.y = PhysicsConst::box2DToWorld(this->currentPosition.y) - this->offset.y;

		this->currentAngle = this->body->GetAngle();

#if defined(CS_EDITOR)
		this->lastPosition = vec2(this->currentPosition.x, this->currentPosition.y);
		this->lastAngle = this->currentAngle;
#endif

		return true;
	}

	void PhysicsBody::updateBody(bool force)
	{
		if (!this->body)
			return;

		if (this->bodyType->getType() == b2_dynamicBody && !force)
		{
			// log::info("Ignoring position for dynamic body update");
			return;
		}
			
		b2Vec2 adjusted_position = this->currentPosition;
		adjusted_position.x = PhysicsConst::worldToBox2D(adjusted_position.x);
		adjusted_position.y = PhysicsConst::worldToBox2D(adjusted_position.y);

		this->body->SetTransform(adjusted_position, this->currentAngle);
	}

	void PhysicsBody::updateBodyAngle(bool force)
	{
		if (!this->body)
			return;

		if (this->bodyType->getType() == b2_dynamicBody && !force)
		{
			// log::info("Ignoring angle for dynamic body update");
			return;
		}

		this->body->SetTransform(this->body->GetPosition(), this->currentAngle);
	}

	void PhysicsBody::updateTransform(const Transform& transform)
	{
		this->updateCollision(transform);
	}

	void PhysicsBody::updateCollision(const Transform& transform)
	{
		if (this->onParticleCollision)
			this->onParticleCollision->updateTransform(transform);
	}

	bool PhysicsBody::intersects(b2Vec2& hit)
	{
		if (!this->body)
			return false;

		b2Vec2 adjusted_position = hit;
		adjusted_position.x = PhysicsConst::worldToBox2D(adjusted_position.x);
		adjusted_position.y = PhysicsConst::worldToBox2D(adjusted_position.y);

		b2Fixture* f = this->body->GetFixtureList();
		if (!f)
			return false;
		do
		{
			if (f->TestPoint(adjusted_position))
				return true;
			f = f->GetNext();
		} while (f);

		return false;
	}

	size_t PhysicsBody::getSelectableShapes(SelectableVolumeList& shapes)
	{ 
		SelectableVolume selectable;
		selectable.volume = this->shape->getVolume();
		selectable.type = SelectableVolumeTypePhysics;
		selectable.offset = vec3(this->offset.x, this->offset.y, 0.0f);

		shapes.push_back(selectable);

		if (this->onParticleCollision)
		{
			this->onParticleCollision->getSelectableShapes(shapes);
		}

		return shapes.size();
	}

}
