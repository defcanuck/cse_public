#include "PCH.h"

#include "ecs/comp/PhysicsComponent.h"
#include "ecs/system/PhysicsSystem.h"
#include "ecs/comp/DrawableComponent.h"
#include "os/LogManager.h"
#include "math/Plane.h"

#include "physics/PhysicsUtils.h"

namespace cs
{

	BEGIN_META_CLASS(PhysicsComponent)
		ADD_META_FUNCTION("Refresh Physics Body", &PhysicsComponent::onUpdateCallback);
		ADD_META_FUNCTION("Compute Physics Body", &PhysicsComponent::computeFromDrawable);
		ADD_MEMBER_PTR(body);
			SET_MEMBER_CALLBACK_POST(&PhysicsComponent::onBodyTypeChanged);

	END_META()

	const vec3 k2DNormal = vec3(0.0f, 0.0f, -1.0f);

	PhysicsComponent::PhysicsComponent() 
		: body(nullptr)
		, syncToEntity(true)
	{ 
		
	}

	void PhysicsComponent::onNew()
	{
		this->computeFromDrawable();

		if (this->body.get())
		{
			this->body->onBodyTypeChanged.addKeyCallback(createCallbackArg0(&PhysicsComponent::onBodyTypeChanged, this), nullptr, this);
			this->body->onShapeParamsChanged.addKeyCallback(createCallbackArg0(&PhysicsComponent::onShapeParamsChanged, this), nullptr, this);
		}
	}

	void PhysicsComponent::onShapeParamsChanged()
	{
		this->updateBodyWorldTransform();
	}

	void PhysicsComponent::setVelocity(const vec2 vel)
	{
		if (!this->body.get())
			return;

		b2Vec2 boxVec2(vel.x, vel.y);
		this->body->setVelocity(boxVec2);
	}

	vec2 PhysicsComponent::getVelocity() const
	{
		if (!this->body.get())
			return kZero2;

		b2Vec2 vel = this->body->getVelocity();
		return vec2(vel.x, vel.y);
	}

	void PhysicsComponent::setBodyInternal(const PhysicsBodyPtr& b)
	{
		this->clearBody();

		this->body = b;
		if (this->body.get())
		{
			this->body->setParentComponent(this);
		}
	}

	void PhysicsComponent::setBody(PhysicsBodyPtr& bd)
	{
		this->setBodyInternal(bd);
	}

	void PhysicsComponent::destroy()
	{
		//log::info("Destroying Physics for ", this->getParent()->getName());
		this->clearBody();
	}

	void PhysicsComponent::clearBody()
	{
		if (this->body.get())
		{
			this->body->unloadBody();
			this->body->setParentComponent(nullptr);
			this->body = nullptr;
		}
	}

	void PhysicsComponent::computeFromDrawable()
	{
		DrawableComponentPtr draw = this->getParent()->getComponent<DrawableComponent>();
		if (draw)
		{
			RenderablePtr& renderable = draw->getRenderable();
			if (renderable.get())
			{
				PhysicsShapePtr shape = PhysicsUtils::createShapeFromRenderable(renderable);
				assert(shape.get());
				if (shape)
				{
					this->setBodyInternal(CREATE_CLASS(PhysicsBody, shape));
				}
			}
		}

		if (!this->body.get())
		{
			this->setBodyInternal(CREATE_CLASS(PhysicsBody));
		}

		this->body->onNew();
		this->updateBodyWorldTransform();
	}

	void PhysicsComponent::onBodyTypeChanged()
	{
		DrawableComponentPtr draw = this->getParent()->getComponent<DrawableComponent>();
		if (draw)
		{
			RenderablePtr& renderable = draw->getRenderable();
			PhysicsShapePtr shape = PhysicsUtils::createShapeFromRenderable(renderable);
			if (shape)
			{
				this->body->setShape(shape);
			}
		}
		
		this->updateBodyWorldTransform();
	}

	void PhysicsComponent::updateBodyWorldTransform()
	{
		Entity* parent = this->getParent();
		if (!parent)
			return;

		this->setWorldPosition(parent->getWorldPosition(), true);
		this->setWorldRotation(parent->getWorldRotation(), true);

		if (this->body.get())
			this->body->updateTransform(parent->getWorldTransform());
	}

	void PhysicsComponent::onPositionChanged(const vec3& pos, const Transform& transform, SceneNode::UpdateType type)
	{
		if (type == SceneNode::UpdateTypePhysics)
			return;

		if (!this->body.get())
		{
			return;
		}

		this->setWorldPosition(pos, true);
		this->body->updateTransform(transform);
	}

	void PhysicsComponent::onScaleChanged(const vec3& scale, const Transform& transform, SceneNode::UpdateType type)
	{

	}

	void PhysicsComponent::onRotationChanged(const quat& rot, const Transform& transform, SceneNode::UpdateType type)
	{
		if (type == SceneNode::UpdateTypePhysics)
			return;

		if (!this->body.get())
		{
			return;
		}

		this->setWorldRotation(rot, true);
		this->body->updateTransform(transform);
	}

	void PhysicsComponent::onUpdateCallback()
	{
		this->onPostLoad();
	}

	void PhysicsComponent::onPostLoad(const LoadFlagMask& flags)
	{
		
		if (!flags.test(LoadFlagsPhysics))
		{
			return;
		}

		if (this->body)
		{
			this->body->onPostLoad(flags);
			this->body->onBodyTypeChanged.addCallback(createCallbackArg0(&PhysicsComponent::onBodyTypeChanged, this));
			this->body->onShapeParamsChanged.addCallback(createCallbackArg0(&PhysicsComponent::onShapeParamsChanged, this));
		}

		Entity* parent = this->getParent();
		if (!parent || !this->body.get())
			return;

		this->body->setParentComponent(this);
		this->body->updateTransform(parent->getWorldTransform());
		this->body->updateBody();

	}

	bool PhysicsComponent::isDynamic() const
	{
		return this->body.get() && this->body->isDynamic();
	}

	void PhysicsComponent::reset(bool active)
	{
		Entity* parent = this->getParent();
		if (!parent)
			return;

		if (this->body.get())
		{
			if (this->body->isDynamic())
			{
				this->setWorldOverrideTransform(active);
				
				this->body->updateTransform(parent->getWorldInitialTransform());
				this->setWorldPosition(parent->getWorldPosition(), true);
				this->setWorldRotation(parent->getWorldRotation(), true);

				this->body->reloadBody();
			}
		}
	}

	void PhysicsComponent::setWorldOverrideTransform(bool active)
	{
		Entity* parent = this->getParent();
		if (!parent)
			return;

		if (!parent->useParent())
		{
			parent->resetTransform();
			return;
		}


		// Fully reset the transform
		parent->setParentOverride(false);
		parent->resetTransform();

		if (active)
		{
			// Next get the World Transform
			Transform world_transform = parent->getWorldTransform();

			// Flag override to false - Transform is absolute now
			parent->setParentOverride(true);
			parent->setTransform(world_transform);
		}
		
	}

	void PhysicsComponent::onEnabledImpl()
	{
		if (this->body.get())
		{
			this->body->reloadBody();
			
			Entity* parent = this->getParent();
			if (!parent)
				this->body->updateTransform(parent->getWorldTransform());
			this->body->updateBody();
		}
	}

	void PhysicsComponent::onDisabledImpl()
	{
		if (this->body.get())
		{
			this->body->unloadBody();
		}
	}

	vec3 PhysicsComponent::getWorldPosition() const
	{
		if (!this->body)
			return vec3();

		const b2Vec2& pos = this->body->getPosition();
		return vec3(pos.x, pos.y, 0.0f);
	}

	quat PhysicsComponent::getWorldRotation() const
	{
		if (!this->body)
			return quat();
		
		float32 angle = this->body->getAngle();
		return glm::normalize(quat(vec3(0.0f, 0.0f, angle)));
	}

	bool PhysicsComponent::sync()
	{
		if (!this->body)
			return false;
		
		bool ret = this->body->sync();

		// Assert that a dynamic body has no parents in the scene graph - that's bad :(
		return ret && this->syncToEntity;
	}

	void PhysicsComponent::setWorldPosition(const vec3& position, bool force)
	{
		if (!this->body)
			return;

		b2Vec2 newPos(position.x, position.y);
		this->body->setPosition(newPos, force);
	}

	void PhysicsComponent::setWorldRotation(const quat& rotation, bool force)
	{
		if (!this->body)
			return;
		
		this->body->setAngle(rotation, force);
	}

	bool PhysicsComponent::intersects(const Ray& ray, vec3& hit_pos)
	{
		if (!this->body)
			return false;

		float t = 0;
		Entity* parent = this->getParent();
		if (!parent)
			return false;

		const Plane plane(parent->getWorldTransform().getPosition(), k2DNormal);
		if (!plane.intersects(ray, t))
			return false;

		// calculate hit point
		hit_pos = ray.getPointAt(t);
		b2Vec2 hit(hit_pos.x, hit_pos.y);

		return this->body->intersects(hit);
	}

	size_t PhysicsComponent::getSelectableShapes(SelectableVolumeList& shapes)
	{
		if (this->body)
			this->body->getSelectableShapes(shapes);
		return shapes.size();
	}

}