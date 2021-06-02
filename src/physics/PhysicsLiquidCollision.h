#pragma once

#include "ClassDef.h"

#include "gfx/RenderInterface.h"
#include "physics/PhysicsShape.h"
#include "fx/ParticleEmitter.h"

#include "Box2D/Box2D.h"

namespace cs
{
	CLASS_DEFINITION_REFLECT(PhysicsLiquidCollision)
	public:

		PhysicsLiquidCollision() { }

		virtual void onCollide(b2ParticleSystem* particleSystem, const b2ParticleHandle* handle) { }
		virtual size_t getSelectableShapes(SelectableVolumeList& shapes) { return 0; }
		virtual void updateTransform(const Transform& transform) { void(0); }

		void killParticle(b2ParticleSystem* particleSystem, const b2ParticleHandle* handle);
		
	};

	CLASS_DEFINITION_DERIVED_REFLECT(PhysicsLiquidCollisionKill, PhysicsLiquidCollision)
	public:

		PhysicsLiquidCollisionKill()
			: PhysicsLiquidCollision()
			, particleEffect(CREATE_CLASS(ParticleEffectHandle))
			, emitter(nullptr)
		{ }

		virtual ~PhysicsLiquidCollisionKill();

		virtual void onNew();
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);

		virtual void onCollide(b2ParticleSystem* particleSystem, const b2ParticleHandle* handle);

		void onEffectChanged();

		ParticleEffectHandlePtr particleEffect;
		ParticleEmitterPtr emitter;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(PhysicsLiquidCollisionWarp, PhysicsLiquidCollision)
	public:

		PhysicsLiquidCollisionWarp()
			: PhysicsLiquidCollision()
			, warpTime(1.0f)
			, warpVelocity(vec2(0.0f, 1.0))
			, warpVolume(std::static_pointer_cast<Volume>(CREATE_CLASS(QuadVolume)))
			, transform(CREATE_CLASS(Transform))
			, lineVolume(nullptr)
		{

		}

		virtual void onCollide(b2ParticleSystem* particleSystem, const b2ParticleHandle* handle);
		virtual size_t getSelectableShapes(SelectableVolumeList& shapes);
		virtual void updateTransform(const Transform& transform) { *this->transform = transform; }

		void onVelocityChanged();

	private:

		float32 warpTime;
		vec2 warpVelocity;
		VolumePtr warpVolume;
		TransformPtr transform;

		LineVolumePtr lineVolume;

	};


	CLASS_DEFINITION_DERIVED_REFLECT(PhysicsLiquidCollisionScript, PhysicsLiquidCollision)
	public:
		PhysicsLiquidCollisionScript()
			: PhysicsLiquidCollision()
			, tag("") { }

		virtual void onCollide(b2ParticleSystem* particleSystem, const b2ParticleHandle* handle);

	private:

		std::string tag;
	};
}

