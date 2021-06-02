#include "PCH.h"

#include "physics/PhysicsLiquidCollision.h"
#include "ecs/system/PhysicsSystem.h"

#include "liquid/LiquidGroup.h"

namespace cs
{
	BEGIN_META_CLASS(PhysicsLiquidCollision)

	END_META()

	BEGIN_META_CLASS(PhysicsLiquidCollisionKill)
		ADD_MEMBER_PTR(particleEffect);
			SET_MEMBER_CALLBACK_POST(&PhysicsLiquidCollisionKill::onEffectChanged);
	END_META()

	BEGIN_META_CLASS(PhysicsLiquidCollisionWarp)
		ADD_MEMBER(warpTime);
			SET_MEMBER_MIN(0.0f);
			SET_MEMBER_MAX(6.0f);
		ADD_MEMBER(warpVelocity);
			SET_MEMBER_CALLBACK_POST(&PhysicsLiquidCollisionWarp::onVelocityChanged);
		ADD_MEMBER_PTR(warpVolume);
	END_META()

	BEGIN_META_CLASS(PhysicsLiquidCollisionScript)
			ADD_MEMBER(tag);
	END_META()

	void PhysicsLiquidCollision::killParticle(b2ParticleSystem* particleSystem, const b2ParticleHandle* handle)
	{
		int32 index = handle->GetIndex();
		void* userData = particleSystem->GetUserDataBuffer()[index];
		LiquidGroup* group = reinterpret_cast<LiquidGroup*>(userData);
		group->removeParticle(handle);
	}

	void PhysicsLiquidCollisionKill::onCollide(b2ParticleSystem* particleSystem, const b2ParticleHandle* handle)
	{
		this->killParticle(particleSystem, handle);
		if (this->emitter.get())
		{
			int32 index = handle->GetIndex();
			void* userData = particleSystem->GetUserDataBuffer()[index];
			LiquidGroup* group = reinterpret_cast<LiquidGroup*>(userData);

			b2Vec2 position;
			if (group->getParticlePosition(handle, position))
			{
				vec3 vec_pos(position.x, position.y, 0.0f);
				this->emitter->spawn(vec_pos);
			}
		}
	}

	PhysicsLiquidCollisionKill::~PhysicsLiquidCollisionKill()
	{
		this->particleEffect->onChanged.removeKeyCallback(nullptr, this);
	}

	void PhysicsLiquidCollisionKill::onNew()
	{
		CallbackPtr callback = createCallbackArg0(&PhysicsLiquidCollisionKill::onEffectChanged, this);
		this->particleEffect->onChanged.addKeyCallback(callback, nullptr, this);

		this->onEffectChanged();
	}

	void PhysicsLiquidCollisionKill::onPostLoad(const LoadFlagMask& flags)
	{
		CallbackPtr callback = createCallbackArg0(&PhysicsLiquidCollisionKill::onEffectChanged, this);
		this->particleEffect->onChanged.addKeyCallback(callback, nullptr, this);

		if (flags.test(LoadFlagsPhysics))
		{
			this->onEffectChanged();
		}
	}

	void PhysicsLiquidCollisionKill::onEffectChanged()
	{
		if (this->particleEffect.get() && this->particleEffect->hasEffect())
		{
			this->emitter = CREATE_CLASS(ParticleEmitter, this->particleEffect, RenderTraversalMain);
		}
	}

	void PhysicsLiquidCollisionWarp::onCollide(b2ParticleSystem* particleSystem, const b2ParticleHandle* handle)
	{
		int32 index = handle->GetIndex();
		void* userData = particleSystem->GetUserDataBuffer()[index];
		LiquidGroup* group = reinterpret_cast<LiquidGroup*>(userData);
		group->warpParticle(handle, this->warpTime, this->warpVolume, this->transform, this->warpVelocity);
	}

	void PhysicsLiquidCollisionWarp::onVelocityChanged()
	{
		// reset the velocity overlays
		this->lineVolume = nullptr;
	}

	size_t PhysicsLiquidCollisionWarp::getSelectableShapes(SelectableVolumeList& shapes)
	{
		SelectableVolume selectable;
		selectable.volume = this->warpVolume;
		selectable.type = SelectableVolumeTypeCollision;
		shapes.push_back(selectable);

		if (!this->lineVolume.get())
		{
			vec3 start = this->warpVolume->getCenter();
			float32 length = std::min<float32>(this->warpVelocity.length(), 100.0f);
			vec2 dir = glm::normalize(this->warpVelocity);

			vec3 end(dir.x * length + start.x, dir.y * length + start.y, 0.0f);
			this->lineVolume = CREATE_CLASS(LineVolume, start, end);
		}

		SelectableVolume velocity_selectable;
		velocity_selectable.volume = this->lineVolume;
		velocity_selectable.type = SelectableVolumeTypeDirection;
		shapes.push_back(velocity_selectable);
		
		return shapes.size();
	}

	void PhysicsLiquidCollisionScript::onCollide(b2ParticleSystem* particleSystem, const b2ParticleHandle* handle)
	{

		int32 index = handle->GetIndex();
		void* userData = particleSystem->GetUserDataBuffer()[index];
		LiquidGroup* group = reinterpret_cast<LiquidGroup*>(userData);

		PhysicsLiquidContactData data;
		if (PhysicsSystem::getInstance()->invokeOnCollisionScriptParticleCallbacks(tag, data))
		{
			this->killParticle(particleSystem, handle);
		}
	}

}