#pragma once

#include "ClassDef.h"
#include "fx/Particle.h"
#include "fx/ParticleTypes.h"
#include "fx/ParticleModule.h"
#include "fx/ParticleScript.h"

#include "gfx/TextureHandle.h"
#include "gfx/ShaderHandle.h"
#include "gfx/DrawOptions.h"

#include "global/SerializableHandle.h"

namespace cs
{
	enum EmissionType
	{
		EmissionTypeNone = -1,
		EmissionTypeBurst,
		EmissionTypeTimed,
		EmissionTypeInfinite,
		//...
		EmissionTypeMAX
	};

	CLASS_DEFINITION_REFLECT(EmissionHandler)
	public:
		virtual EmissionType getType() const { return EmissionTypeNone; }
		virtual size_t calculateParticlesThisFrame(float32 dt, float32& emissionCounter) { return 0; }
		virtual float32 getLifetime() const { return 0.0f; }
		virtual int32 getMaxParticles(float32 maxLifeTime) { return 0; }

		Event onChanged;

		void onChangedCallback()
		{
			this->onChanged.invoke();
		}
	};

	CLASS_DEFINITION_DERIVED_REFLECT(EmissionHandlerBurst, EmissionHandler)
	public:
		EmissionHandlerBurst()
			: EmissionHandler()
			, numParticles(10)
		{ }

		EmissionHandlerBurst(int32 num)
			: numParticles(num)
		{ }

		virtual size_t calculateParticlesThisFrame(float32 dt, float32& emissionCounter);
		virtual float32 getLifetime() const { return 0.0f; }
		virtual EmissionType getType() const { return EmissionTypeBurst; }
		virtual int32 getMaxParticles(float32 maxLifeTime) { return numParticles; }
	
	private:
		int32 numParticles;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(EmissionHandlerInfinite, EmissionHandler)
	public:
		EmissionHandlerInfinite()
			: emissionRate(10)
		{ }

		EmissionHandlerInfinite(int32 rate)
			: emissionRate(rate)
		{ }

		virtual size_t calculateParticlesThisFrame(float32 dt, float32& emissionCounter);
		virtual EmissionType getType() const { return EmissionTypeInfinite; }
		virtual float32 getLifetime() const { return -1.0f; }
		virtual int32 getMaxParticles(float32 maxLifeTime) { return int32(this->emissionRate * maxLifeTime + 0.5f); }

		int32 emissionRate; // particles per second
	};

	CLASS_DEFINITION_DERIVED_REFLECT(EmissionHandlerTimed, EmissionHandlerInfinite)
	public:
		EmissionHandlerTimed()
			: EmissionHandlerInfinite()
			, emissionTime(10.0f)
		{ }

		EmissionHandlerTimed(float32 time, int32 rate)
			: EmissionHandlerInfinite(rate)
			, emissionTime(time)
		{ }

		virtual EmissionType getType() const { return EmissionTypeTimed; }
		virtual float32 getLifetime() const { return this->emissionTime; }

		float32 emissionTime;
	};

	inline EmissionHandlerPtr getEmissionHandler(EmissionType type)
	{
		switch (type)
		{
			case EmissionTypeBurst:
				return std::static_pointer_cast<EmissionHandler>(CREATE_CLASS(EmissionHandlerBurst));
			case EmissionTypeTimed:
				return std::static_pointer_cast<EmissionHandler>(CREATE_CLASS(EmissionHandlerTimed));
			case EmissionTypeNone:
			default:
				return CREATE_CLASS(EmissionHandler);
		}
	}

	CLASS_DEFINITION_REFLECT(ParticleEffectData)
	public:

		ParticleEffectData();

		virtual void onNew();
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);

		static ParticlePropertyMask kDefaultMaskProperties;
		ParticlePropertyMask mask;

		EmissionHandlerPtr emissionHandler;
		ParticleFloatValuePtr lifeTime;
		ParticleVec3ValuePtr spawn;
		float32 processTime;
		ParticleColorValuePtr tint;

		ParticleModuleList modules;

		int32 getMaxParticles() const { return this->maxParticles; }
		
		void updateMask();
		void updateMaxParticles();
		
		Event onParticlesChanged;

		void createParticles(
			ParticleInitList& particle_data, 
			size_t num_particles = 1, 
			const ParticleScriptProperties* script_props = nullptr);

		const TextureHandlePtr& getTextureHandle() const { return this->texture; }
		const ShaderHandlePtr& getShaderHandle() const { return this->shader; }

		const DrawOptions& getDrawOptions() const { return this->options; }

	private:

		int32 maxParticles;
		int32 maxParticlesOverride;
		
		DrawOptions options;
		TextureHandlePtr texture;
		bool textureAnimationIndex;

		ShaderHandlePtr shader;

		void onEmissionHandlerChanged();
		void onLifetimeChanged();
		void onModuleChanged();
	};

	CLASS_DEFINITION_DERIVED_REFLECT(ParticleEffect, Resource)
	public:

		ParticleEffect(const std::string& name, ParticleEffectDataPtr& pdata);
		ParticleEffect(const std::string& name, const std::string& path);

		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);

		const ParticlePropertyMask& getMask() const { return this->data.const_get()->mask; }
		size_t getMaxParticles() const { return this->data.const_get()->getMaxParticles(); }

		SerializableHandle<ParticleEffectData>& getParticleDataHandle() { return this->data; }
		ParticleEffectDataPtr& getParticleEffectData() { return this->data.get(); }

		size_t calculateParticlesThisFrame(float32 dt, float32& emissionCounter);

		void createParticles(
			ParticleInitList& particle_data, 
			size_t num_particles = 1,
			const ParticleScriptProperties* script_props = nullptr);

		Event onUpdateHeap;

	private:

		void onParticlesChangedCallback();

		ParticleEffect() { }

		SerializableHandle<ParticleEffectData> data;

	};

	CLASS_DEFINITION_REFLECT(ParticleEffectHandle)
	public:

		ParticleEffectHandle()
			: particleEffect(nullptr)
		{ }

		ParticleEffectHandle(const std::string& effectName);

		ParticleEffectHandle(ParticleEffectPtr& effect);
		ParticleEffectHandle(ParticleEffectHandlePtr& rhs);

		virtual ~ParticleEffectHandle();
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);

		ParticleEffectPtr& getEffect() { return this->particleEffect; }
		bool hasEffect() const { return this->particleEffect.get() != nullptr; }

		ParticleEffectDataPtr getEffectData() 
		{
			if (this->particleEffect.get())
				return this->particleEffect->getParticleEffectData();
			return ParticleEffectDataPtr();
		}

		void onEffectChanged();

		void onEffectResourcePreChanged();
		void onEffectResourcePostChanged();

		size_t calculateParticlesThisFrame(float32 dt, float32& emissionCounter);
		
		Event onChanged;

		void clear()
		{
			this->particleEffect = nullptr;
			this->onChanged.invoke();
		}

	private:

		void init();

		ParticleEffectPtr particleEffect;
	};

	template <>
    std::string SerializableHandle<ParticleEffectData>::getExtension();

	template <>
    std::string SerializableHandle<ParticleEffectData>::getDescription();

}
