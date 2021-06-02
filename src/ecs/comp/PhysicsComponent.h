#pragma once

#include "ClassDef.h"
#include "ecs/comp/Component.h"

#include "math/GLM.h"
#include "math/Ray.h"
#include "physics/PhysicsGlobals.h"
#include "physics/PhysicsModifier.h"

#include <Box2D/Box2D.h>

namespace cs
{

	CLASS_DEFINITION_DERIVED_REFLECT(PhysicsComponent, Component)
	
	public:

		PhysicsComponent();
		virtual ~PhysicsComponent() { }

		virtual void reset(bool active = false);
		virtual void onPositionChanged(const vec3& pos, const Transform& transform, SceneNode::UpdateType type = SceneNode::UpdateTypeNone);
		virtual void onScaleChanged(const vec3& scale, const Transform& transform, SceneNode::UpdateType type = SceneNode::UpdateTypeNone);
		virtual void onRotationChanged(const quat& rot, const Transform& transform, SceneNode::UpdateType type = SceneNode::UpdateTypeNone);

		virtual void destroy();

		vec3 getWorldPosition() const;
		quat getWorldRotation() const;
		void setWorldOverrideTransform(bool active = true);
		void setVelocity(const vec2 vel);
		vec2 getVelocity() const;

		void onBodyTypeChanged();
		void onShapeParamsChanged();
		void onShapeChanged();

		bool intersects(const Ray& ray, vec3& hit_pos);

		virtual void onNew();

		bool isDynamic() const;
		bool sync();
	
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);
		
		void onUpdateCallback();
		void computeFromDrawable();

		size_t getSelectableShapes(SelectableVolumeList& shapes);

		PhysicsBodyPtr getBody() { return this->body; }

		void setBody(PhysicsBodyPtr& bd);
		void clearBody();
		
		void setSyncToEntity(bool syncTo) { this->syncToEntity = syncTo; }
		bool getSyncToEntity() const { return this->syncToEntity; }

	protected:

		virtual void onEnabledImpl();
		virtual void onDisabledImpl();

	private:

		void setWorldPosition(const vec3& position, bool force = false);
		void setWorldRotation(const quat& rotation, bool force = false);
		
		void updateTransform();
		void updateBodyWorldTransform();
		void setBodyInternal(const PhysicsBodyPtr& b);

		PhysicsBodyPtr body;
		bool syncToEntity;
	};
}