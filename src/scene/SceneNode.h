#pragma once


#include "ClassDef.h"
#include "math/Transform.h"

namespace cs
{


	CLASS_DEFINITION_REFLECT(SceneNode)

	public:

		enum UpdateType
		{
			UpdateTypeNone = -1,
			UpdateTypePhysics,
			//...
			UpdateTypeMAX
		};

		typedef std::function<void(const vec3&)> OnPositionChanged;
		typedef std::vector<OnPositionChanged> OnPositionChangedList;
		
		SceneNode();
		SceneNode(Transform& transform);
		virtual ~SceneNode();

		SceneNodePtr& getParent() { return this->parent; }

		inline void setTransform(Transform& transform) { this->setCurrentTransform(transform); }
		inline void setPosition(vec3 pos) { this->setCurrentPosition(pos); }
		inline void setRotation(quat rot) { this->setCurrentRotation(rot); }
		inline void setScale(vec3 scl) { this->setCurrentScale(scl); }

		void setCurrentTransform(Transform& transform, UpdateType type = UpdateTypeNone);
		void setCurrentPosition(vec3 pos, UpdateType type = UpdateTypeNone);
		void setCurrentRotation(quat rot, UpdateType type = UpdateTypeNone);
		void setCurrentScale(vec3 scl, UpdateType type = UpdateTypeNone);

		void resetLocalPosition();
		void resetLocalRotation();
		void resetLocalScale();
		
		void setInitialTransform(Transform& transform);
		void setInitialPosition(vec3 pos);
		void setInitialRotation(quat rot);
		void setInitialScale(vec3 scl);

		bool hasMoved() const { return this->physicsTouched; }

		const vec3& getPosition() { return currentTransform.getPosition(); }
		const quat& getRotation() { return currentTransform.getRotation(); }
		const vec3& getScale() { return currentTransform.getScale(); }

		const vec3& getInitialPosition() { return currentTransform.getPosition(); }
		const quat& getInitialRotation() { return currentTransform.getRotation(); }
		const vec3& getInitialScale() { return currentTransform.getScale(); }

		vec3 getWorldPosition() const;
		vec3 getWorldScale() const;
		quat getWorldRotation() const;

		Transform getWorldTransform() const;
		Transform getWorldInitialTransform() const;

		const Transform& getLocalInitialTransform() const { return this->initTransform; }
		const Transform& getLocalTransform() const { return this->currentTransform; }

		virtual void onPositionChanged(const vec3& pos, const Transform& transform, UpdateType type = UpdateTypeNone) { }
		virtual void onScaleChanged(const vec3& scale, const Transform& transform, UpdateType type = UpdateTypeNone) { }
		virtual void onRotationChanged(const quat& rot, const Transform& transform, UpdateType type = UpdateTypeNone) { }

		virtual void setRenderableSize(vec2 sz, AnimationScaleType type) { }
		virtual void setRenderableUV(RectF uv) { }

		void setPitch(float32 degrees);
		void setYaw(float32 degrees);
		void setRoll(float32 degrees);

		void addPitch(float32 degrees);
		void addYaw(float32 degrees);
		void addRoll(float32 degrees);

		float32 getPitch() const;
		float32 getYaw() const;
		float32 getRoll() const;

		void onUpdateCallback();

		void setParentOverride(bool override) { this->overrideParent = override; }

		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);

		void refreshNode()
		{
			const Transform transform = this->getWorldTransform();
			this->onPositionChanged(this->getWorldPosition(), transform);
			this->onScaleChanged(this->getWorldScale(), transform);
			this->onRotationChanged(this->getWorldRotation(), transform);
		}

		inline bool hasParent() const { return this->parent.get() != nullptr && !this->overrideParent; }
		inline bool useParent() const { return this->parent.get() != nullptr; };

		virtual void reset();
		virtual void clear();

		void resetTransform();

		virtual bool getRotationUncoupled() const { return false; }

	protected:

		void setParent(SceneNodePtr& ptr);
		void clearParent();

		Transform initTransform;
		Transform currentTransform;

		SceneNodePtr parent;
		bool overrideParent;

		bool physicsTouched;
		int physUpdateCtr;

	};
}