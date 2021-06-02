#include "PCH.h"

#include "scene/SceneNode.h"
#include "global/Stats.h"

namespace cs
{
	BEGIN_META_CLASS(SceneNode)

		ADD_MEMBER(initTransform);
			SET_MEMBER_CALLBACK_POST(&SceneNode::onUpdateCallback);
	END_META()

	SceneNode::SceneNode()
		: currentTransform(Transform())
		, initTransform(Transform())
		, parent(nullptr)
		, overrideParent(false)
		, physicsTouched(false)
		, physUpdateCtr(0)
	{
		EngineStats::incrementStat(StatTypeEntity);
	}

	SceneNode::SceneNode(Transform& transform)
		: currentTransform(transform)
		, initTransform(transform)
		, parent(nullptr)
		, overrideParent(false)
		, physicsTouched(false)
	{
		EngineStats::incrementStat(StatTypeEntity);
	}

	SceneNode::~SceneNode()
	{
		EngineStats::decrementStat(StatTypeEntity);
	}

	void SceneNode::resetTransform()
	{
		this->currentTransform = this->initTransform;
	}

	void SceneNode::reset()
	{
		this->resetTransform();
	}

	void SceneNode::setCurrentPosition(vec3 pos, UpdateType type)
	{
		this->physicsTouched |= (type == UpdateTypePhysics && this->currentTransform.getPosition() != pos) && this->physUpdateCtr++ >= 1;
		
		this->currentTransform.setPosition(pos);
		this->onPositionChanged(this->getWorldPosition(), this->getWorldTransform(), type);
	}

	void SceneNode::setCurrentRotation(quat rot, UpdateType type)
	{
		this->currentTransform.setRotation(rot);
		this->onRotationChanged(this->getWorldRotation(), this->getWorldTransform(), type);
	}

	void SceneNode::setCurrentScale(vec3 scl, UpdateType type)
	{
		this->currentTransform.setScale(scl);
		this->onScaleChanged(this->getWorldScale(), this->getWorldTransform(), type);
	}

	void SceneNode::resetLocalPosition()
	{
		this->currentTransform.setPosition(Transform::kDefaultPosition);
		this->onPositionChanged(this->getWorldPosition(), this->getWorldTransform());
	}

	void SceneNode::resetLocalRotation()
	{
		this->currentTransform.setRotation(Transform::kDefaultRotation);
		this->onRotationChanged(this->getWorldRotation(), this->getWorldTransform());
	}

	void SceneNode::resetLocalScale()
	{
		this->currentTransform.setScale(Transform::kDefaultScale);
		this->onScaleChanged(this->getWorldScale(), this->getWorldTransform());
	}

	void SceneNode::setCurrentTransform(Transform& transform, UpdateType type)
	{
		Transform old_transform = this->currentTransform;
		this->currentTransform = transform;

		if (old_transform.position != this->currentTransform.position)
			this->onPositionChanged(this->getWorldPosition(), this->getWorldTransform(), type);
		
		if (old_transform.rotation != this->currentTransform.rotation)
			this->onRotationChanged(this->getWorldRotation(), this->getWorldTransform(), type);
		
		if (old_transform.scale != this->currentTransform.scale)
			this->onScaleChanged(this->getWorldScale(), this->getWorldTransform(), type);

	}

	void SceneNode::setInitialTransform(Transform& transform)
	{
		this->initTransform = transform;

		if (this->initTransform.position != this->currentTransform.position)
			this->onPositionChanged(this->getWorldPosition(), this->getWorldTransform());

		if (this->initTransform.rotation != this->currentTransform.rotation)
			this->onRotationChanged(this->getWorldRotation(), this->getWorldTransform());

		if (this->initTransform.scale != this->currentTransform.scale)
			this->onScaleChanged(this->getWorldScale(), this->getWorldTransform());
	}

	void SceneNode::setInitialPosition(vec3 pos)
	{
		this->initTransform.setPosition(pos);
		this->setPosition(pos);
	}

	void SceneNode::setInitialRotation(quat rot)
	{
		this->initTransform.setRotation(rot);
		this->setRotation(rot);
	}

	void SceneNode::setInitialScale(vec3 scl)
	{
		this->initTransform.setScale(scl);
		this->setScale(scl);
	}

	vec3 SceneNode::getWorldPosition() const
	{
		if (this->hasParent())
		{
			Transform parent_trans = this->parent->getWorldTransform();
			Transform trans = this->currentTransform.concatenate(parent_trans);
			vec3 pos(0.0f, 0.0f, 0.0f);
			return trans.translate(pos);
		}

		return currentTransform.getPosition();
	}

	vec3 SceneNode::getWorldScale()  const
	{
		if (this->hasParent())
		{
			Transform parent_trans = this->parent->getWorldTransform();
			Transform trans = this->currentTransform.concatenate(parent_trans);
			vec3 scl(1.0f, 1.0f, 1.0f);
			return trans.translateScale(scl);
		}

		return currentTransform.getScale();
	}

	quat SceneNode::getWorldRotation()  const
	{
		if (this->hasParent())
		{
			quat parent_rot = this->parent->getWorldRotation();
			return this->currentTransform.getRotation() * parent_rot;
		}

		return currentTransform.getRotation();
	}

	Transform SceneNode::getWorldTransform() const
	{
		if (this->hasParent())
		{
			if (this->getRotationUncoupled())
			{
				// inherit the rotation from the parent about the local origin, then translate
				Transform parentTransform = this->parent->getWorldTransform();
				Transform posTrans(parentTransform.position);
				Transform current = this->currentTransform;
				current.setRotation(current.rotation * parentTransform.rotation);
				return current.concatenate(posTrans);
			}
			else
			{
				return this->currentTransform.concatenate(this->parent->getWorldTransform());
			}
		}

		return currentTransform;
	}

	Transform SceneNode::getWorldInitialTransform() const
	{
		if (this->hasParent())
		{
			Transform trans = this->initTransform.concatenate(this->parent->getWorldInitialTransform());
			return trans;
		}

		return initTransform;
	}

	void SceneNode::onPostLoad(const LoadFlagMask& flags)
	{
		if (!flags.test(LoadFlagsSceneGraph))
			return;

		this->initTransform.refresh();
		this->currentTransform = this->initTransform;
		this->refreshNode();
	}

	void SceneNode::onUpdateCallback()
	{
		if (this->currentTransform.position != this->initTransform.position)
		{
			this->setPosition(this->initTransform.position);
		}
		if (this->currentTransform.rotation != this->initTransform.rotation)
		{
			this->setRotation(this->initTransform.rotation);
		}
		if (this->currentTransform.scale != this->initTransform.scale)
		{
			this->setScale(this->initTransform.scale);
		}
	}

	void SceneNode::clear()
	{
		this->clearParent();
	}

	void SceneNode::setPitch(float32 degrees)
	{
		this->setRotation(glm::quat(vec3(degreesToRadians(degrees), 0.0f, 0.0f)));
	}

	void SceneNode::setYaw(float32 degrees)
	{
		this->setRotation(glm::quat(vec3(0.0f, degreesToRadians(degrees), 0.0f)));
	}

	void SceneNode::setRoll(float32 degrees)
	{
		this->setRotation(glm::quat(vec3(0.0f, 0.0f, degreesToRadians(degrees))));
	}

	void SceneNode::addPitch(float32 degrees)
	{
		quat currentRotation = this->currentTransform.getRotation();
		this->setRotation(currentRotation * glm::quat(vec3(degreesToRadians(degrees), 0.0f, 0.0f)));
	}

	void SceneNode::addYaw(float32 degrees)
	{
		quat currentRotation = this->currentTransform.getRotation();
		this->setRotation(currentRotation * glm::quat(vec3(0.0f, degreesToRadians(degrees), 0.0f)));
	}

	void SceneNode::addRoll(float32 degrees)
	{
		quat currentRotation = this->currentTransform.getRotation();
		this->setRotation(currentRotation * glm::quat(vec3(0.0f, 0.0f, degreesToRadians(degrees))));
	}


	float32 SceneNode::getPitch() const
	{
		vec3 angles = glm::eulerAngles(this->getWorldRotation());
		return radiansToDegrees(angles.x);
	}

	float32 SceneNode::getYaw() const
	{
		vec3 angles = glm::eulerAngles(this->getWorldRotation());
		return radiansToDegrees(angles.y);
	}

	float32 SceneNode::getRoll() const
	{
		vec3 angles = glm::eulerAngles(this->getWorldRotation());
		return radiansToDegrees(angles.z);
	}

	void SceneNode::setParent(SceneNodePtr& ptr)
	{
		assert(ptr.get() != this);
		this->parent = ptr;
	}

	void SceneNode::clearParent()
	{
		if (this->parent)
		{
			this->parent = nullptr;
		}
	}

}