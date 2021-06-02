#pragma once

#include "ecs/comp/Component.h"
#include "math/BoundingVolume.h"
#include "geom/Volume.h"
#include "ecs/comp/ScriptComponent.h"

namespace cs
{

	CLASS_DEFINITION(CollisionScriptInstance)
	public:
		CollisionScriptInstance(Entity* e) : entity(e) { }
		virtual ~CollisionScriptInstance() { }

		virtual void onMoved() { log::info("onMoved"); }

		void setEntity(Entity* e) { this->entity = e; }
		Entity* getEntity();

	private:

		Entity* entity;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(CollisionComponent, Component)
	public:

		CollisionComponent();
		CollisionComponent(BoundingVolumePtr volume) 
			: boundingVolume(volume) 
		{ }
		
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);

		bool load(LuaStatePtr& state);
		bool intersect(const Ray& ray, const Transform& trans, HitParams& params);
		bool test(const vec3& pos) const;

		void onVolumeChangedCallback();

		const mat4& getInverseMatrix();
		
		bool getNotifyOnMoved() const { return this->notifyOnMoved; }
		bool getHasMoved() const { return this->hasMoved; }

		void setHasMoved() { this->hasMoved = true; }

		void onScriptChanged();
		void onMoved();

		void setLuaState(LuaStatePtr& ptr) { this->luaState = ptr; }
		void createInstance();

	private:

		void refreshScript();

		BoundingVolumePtr boundingVolume;
		VolumePtr volume;

		bool initialized;
		mat4 inverseMatrix;

		bool notifyOnMoved;
		bool hasMoved;

		LuaStatePtr luaState;
		LuaScriptHandlePtr onHitScript;
		bool scriptLoaded;
		LuaObjectPtr<CollisionScriptInstance> instance;

	};
}