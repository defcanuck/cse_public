#pragma once

#include "ecs/ECS.h"
#include "ClassDef.h"

#include "scene/SceneNode.h"
#include "math/Transform.h"

#include <memory>

namespace cs
{
	


	class Entity;

	CLASS_DEFINITION_REFLECT(Component)

	public:

		Component() 
			: parent(nullptr)
			, enabled(true) 
		{ }

		virtual ~Component() { }

		void setParent(Entity* entity) { parent = entity; }
		Entity* getParent() { return parent; }
		void clearParent() { parent = nullptr; }

		unsigned int getId();

		virtual void onNew() { }
		virtual void reset(bool active = false) { }

		virtual void onPositionChanged(const vec3& pos, const Transform& transform, SceneNode::UpdateType type = SceneNode::UpdateTypeNone) { }
		virtual void onScaleChanged(const vec3& scale, const Transform& transform, SceneNode::UpdateType type = SceneNode::UpdateTypeNone) { }
		virtual void onRotationChanged(const quat& rot, const Transform& transform, SceneNode::UpdateType type = SceneNode::UpdateTypeNone) { }

		virtual void destroy() { }
		virtual std::type_index getType() const
		{
			return std::type_index(typeid(*this));
		}

		void setEnabled()
		{
			if (!this->enabled)
			{
				this->onEnabledImpl();
				this->enabled = true;
			}
		}

		void setDisabled()
		{
			if (this->enabled)
			{
				this->onDisabledImpl();
				this->enabled = false;
			}
		}

		bool getEnabled() const { return this->enabled; }

	protected:

		virtual void onEnabledImpl() { }
		virtual void onDisabledImpl() { }

	private:

		Entity* parent;
		bool enabled;

	};
}

#include "ecs/comp/SerializeComponent.h"