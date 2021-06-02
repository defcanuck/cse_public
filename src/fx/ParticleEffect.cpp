#include "PCH.h"

#include "fx/ParticleEffect.h"
#include "gfx/RenderInterface.h"
#include "global/ResourceFactory.h"

namespace cs
{

	BEGIN_META_CLASS(EmissionHandler)
	END_META();

	BEGIN_META_CLASS(EmissionHandlerBurst)
		ADD_MEMBER(numParticles);
			SET_MEMBER_MIN(0);
			SET_MEMBER_MAX(MAX_EMITTER_PARTICLES);
			SET_MEMBER_CALLBACK_POST(&EmissionHandler::onChangedCallback);
		END_META();

	BEGIN_META_CLASS(EmissionHandlerInfinite)
		ADD_MEMBER(emissionRate);
			SET_MEMBER_MIN(0);
			SET_MEMBER_MAX(MAX_EMITTER_PARTICLES);
			SET_MEMBER_CALLBACK_POST(&EmissionHandler::onChangedCallback);
	END_META();

	BEGIN_META_CLASS(EmissionHandlerTimed)
		ADD_MEMBER(emissionTime);
			SET_MEMBER_NO_SLIDER();
			SET_MEMBER_CALLBACK_POST(&EmissionHandler::onChangedCallback);
	END_META();

	BEGIN_META_CLASS(ParticleEffectData)
		
		ADD_MEMBER_PTR(texture);
			SET_MEMBER_COLLAPSEABLE();
			SET_MEMBER_START_COLLAPSED();
		ADD_MEMBER(textureAnimationIndex);
			SET_MEMBER_DEFAULT(false);
			SET_MEMBER_CALLBACK_POST(&ParticleEffectData::updateMask);

		ADD_MEMBER_PTR(shader);
			SET_MEMBER_COLLAPSEABLE();
			SET_MEMBER_START_COLLAPSED();

		ADD_MEMBER_PTR(emissionHandler);
			ADD_COMBO_META_LABEL(EmissionHandlerBurst, "Burst");
			ADD_COMBO_META_LABEL(EmissionHandlerTimed, "Timed");
			ADD_COMBO_META_LABEL(EmissionHandlerInfinite, "Infinite");
			SET_MEMBER_CALLBACK_POST(&ParticleEffectData::onEmissionHandlerChanged);
			SET_MEMBER_COLLAPSEABLE();
			SET_MEMBER_START_COLLAPSED();

		ADD_MEMBER_PTR(lifeTime);
			ADD_PARTICLE_FLOAT_OPTIONS();
			SET_MEMBER_CALLBACK_POST(&ParticleEffectData::onLifetimeChanged);
			SET_MEMBER_COLLAPSEABLE();
			SET_MEMBER_START_COLLAPSED();

		ADD_MEMBER(processTime);
			SET_MEMBER_DEFAULT(-1.0f);
			SET_MEMBER_MIN(-1.0f);
			SET_MEMBER_MAX(1.0f);
			SET_MEMBER_COLLAPSEABLE();
			SET_MEMBER_START_COLLAPSED();

		ADD_MEMBER_PTR(tint);
			SET_MEMBER_COLLAPSEABLE();
			SET_MEMBER_START_COLLAPSED();
			ADD_PARTICLE_COLOR_OPTIONS();

		ADD_MEMBER_PTR(spawn);
			ADD_PARTICLE_VEC3_OPTIONS();
			SET_MEMBER_COLLAPSEABLE();
			SET_MEMBER_START_COLLAPSED();

		ADD_MEMBER(maxParticles);
			SET_MEMBER_IGNORE_GUI();

		ADD_MEMBER(maxParticlesOverride);
			SET_MEMBER_MIN(-1);
			SET_MEMBER_MAX(MAX_EMITTER_PARTICLES);
			SET_MEMBER_NO_SLIDER();
			SET_MEMBER_CALLBACK_POST(&ParticleEffectData::updateMaxParticles);

		ADD_MEMBER(modules);
			ADD_PARTICLE_MODULES();
			SET_MEMBER_CALLBACK_POST(&ParticleEffectData::onModuleChanged);

		ADD_MEMBER(options);
			SET_MEMBER_COLLAPSEABLE();
			SET_MEMBER_START_COLLAPSED();

	END_META();

	BEGIN_META_RESOURCE(ParticleEffect)
		ADD_MEMBER_PTR(data);
	END_META();

	BEGIN_META_CLASS(ParticleEffectHandle)
		ADD_MEMBER_RESOURCE(particleEffect);
			SET_MEMBER_CALLBACK_PRE(&ParticleEffectHandle::onEffectResourcePreChanged);
			SET_MEMBER_CALLBACK_POST(&ParticleEffectHandle::onEffectResourcePostChanged);
	END_META();

	size_t EmissionHandlerBurst::calculateParticlesThisFrame(float32 dt, float32& emissionCounter)
	{
		// flag the emission counter only once to emit all particles at once
		if (emissionCounter < 0.0f)
			return 0;

		emissionCounter = -1.0f;
		return this->numParticles;
	}

	size_t EmissionHandlerInfinite::calculateParticlesThisFrame(float32 dt, float32& emissionCounter)
	{
		float32 emission = (dt * float32(this->emissionRate)) + emissionCounter;
		size_t newEmission = size_t(emission);
		emissionCounter = emission - float32(newEmission);
		return newEmission;
	}

	ParticlePropertyMask ParticleEffectData::kDefaultMaskProperties(
		{ 
			ParticlePropertyOwner,
			ParticlePropertyTime, 
			ParticlePropertyLifetime,
			ParticlePropertyProcessTime,
			ParticlePropertyPosition 
		}
	);

	ParticleEffectData::ParticleEffectData()
		: emissionHandler(CREATE_CLASS(EmissionHandlerTimed))
		, lifeTime(CREATE_CLASS(ParticleFloatValueConstant, 1.0f))
		, processTime(-1.0f)
		, tint(CREATE_CLASS(ParticleColorValueConstant))
		, spawn(CREATE_CLASS(ParticleVec3ValueConstant, kZero3))
		, maxParticles(0)
		, maxParticlesOverride(-1)
		, mask(kDefaultMaskProperties)
		, texture(CREATE_CLASS(TextureHandle, RenderInterface::kWhiteTexture))
		, textureAnimationIndex(false)
		, shader(CREATE_CLASS(ShaderHandle, RenderInterface::kDefaultTextureColorShader))
	{ 
		this->options.depth.setDepthTest(true);
		this->options.depth.setDepthWrite(false);
		this->updateMaxParticles();
	}

	void ParticleEffectData::onEmissionHandlerChanged()
	{
		this->emissionHandler->onChanged += createCallbackArg0(&ParticleEffectData::updateMaxParticles, this);
		this->updateMaxParticles();
	}

	void ParticleEffectData::onLifetimeChanged()
	{
		this->lifeTime->onChanged += createCallbackArg0(&ParticleEffectData::updateMaxParticles, this);
		this->updateMaxParticles();
	}

	void ParticleEffectData::onNew()
	{
		this->lifeTime->onChanged += createCallbackArg0(&ParticleEffectData::updateMaxParticles, this);
		this->emissionHandler->onChanged += createCallbackArg0(&ParticleEffectData::updateMaxParticles, this);
		this->updateMaxParticles();
	}

	void ParticleEffectData::onPostLoad(const LoadFlagMask& flags)
	{
		this->lifeTime->onChanged += createCallbackArg0(&ParticleEffectData::updateMaxParticles, this);
		this->emissionHandler->onChanged += createCallbackArg0(&ParticleEffectData::updateMaxParticles, this);
		this->updateMask();
	}

	void ParticleEffectData::updateMaxParticles()
	{
		if (this->maxParticlesOverride != -1)
		{ 
			this->maxParticles = this->maxParticlesOverride;
		}
		else
		{
			this->maxParticles = (int32) this->emissionHandler->getMaxParticles(this->lifeTime->getMaxValue());
		}
		this->onParticlesChanged.invoke();
		// log::info("Updating MaxParticles to ", this->maxParticles);
	}

	void ParticleEffectData::createParticles(ParticleInitList& particle_data, size_t num_particles, const ParticleScriptProperties* script_props)
	{
		assert(this->lifeTime.get());
		assert(this->spawn.get());

		ParticlePropertyMask ignoreMask;
		if (script_props)
		{
			ignoreMask = (*script_props).getMask();
		}

		bool hasAnimationIndex = this->textureAnimationIndex && this->texture->hasAnimation();

		ColorB frameTint = this->tint->getValue();


		for (size_t i = 0; i < num_particles; i++)
		{
			ParticleInitProps& data = particle_data.next();
			data.lifeTime = this->lifeTime->getValue();
			data.position = this->spawn->getValue();
			data.processTime = this->processTime;
			data.tint = frameTint;

			for (auto& it : this->modules)
			{
				it->populate(data, ignoreMask);
			}

			if (script_props)
			{
				for (const auto& it : script_props->overrides)
				{
					it.second->populate(data.propertyMap);
				}
			}

			if (hasAnimationIndex)
			{
				TextureAnimationPtr& animation = this->texture->getAnimation();
				if (animation.get())
				{
					int32 index = randomRange(0, animation->getNumFrames() - 1);
					addPropertyForce(data.propertyMap, ParticlePropertyIndex, index);
				}
				else
				{
					log::error("Lies - we require an animation to do framesets!");
				}
			}
		}
	}

	void ParticleEffectData::updateMask()
	{
		this->mask = kDefaultMaskProperties;

		for (auto& it : this->modules)
		{
			it->setMask(this->mask);
		}

		if (this->textureAnimationIndex && this->texture->hasAnimation())
		{
			this->mask.set(ParticlePropertyIndex);
		}
	}

	void ParticleEffectData::onModuleChanged()
	{
		this->updateMask();
		this->onParticlesChanged.invoke();
	}

	ParticleEffect::ParticleEffect(const std::string& name, ParticleEffectDataPtr& pdata)
		: Resource(name)
		, data(SerializableHandle<ParticleEffectData>(pdata))
	{ }

	ParticleEffect::ParticleEffect(const std::string& name, const std::string& path)
		: Resource(name)
	{
        ParticleEffectDataPtr data_ptr = CREATE_CLASS(ParticleEffectData);
		this->data = SerializableHandle<ParticleEffectData>(data_ptr);
		this->data.setFullPath(path);
		this->data.refresh();
	}

	void ParticleEffect::onPostLoad(const LoadFlagMask& flags)
	{
		if (this->data.get())
		{
			this->data->onPostLoad(flags);
			this->data->onParticlesChanged += createCallbackArg0(&ParticleEffect::onParticlesChangedCallback, this);
		}
	}

	size_t ParticleEffect::calculateParticlesThisFrame(float32 dt, float32& emissionCounter)
	{
		if (!this->data.get().get())
			return 0;

		ParticleEffectDataPtr& dataPtr = this->data.get();
		assert(dataPtr->emissionHandler);
		return dataPtr->emissionHandler->calculateParticlesThisFrame(dt, emissionCounter);
	}

	void ParticleEffect::onParticlesChangedCallback()
	{
		this->onUpdateHeap.invoke();
	}
	
	void ParticleEffect::createParticles(ParticleInitList& particle_data, size_t num_particles, const ParticleScriptProperties* script_props)
	{
		if (!this->data.get().get())
		{
			log::error("No particle data to create effects!");
			return;
		}

		ParticleEffectDataPtr& dataPtr = this->data.get();
		dataPtr->createParticles(particle_data, num_particles, script_props);
	}


	ParticleEffectHandle::ParticleEffectHandle(const std::string& effectName)
	{
		this->particleEffect = std::static_pointer_cast<ParticleEffect>(ResourceFactory::getInstance()->loadResource<ParticleEffect>(effectName));
		this->init();
	}

	ParticleEffectHandle::ParticleEffectHandle(ParticleEffectPtr& effect)
		: particleEffect(effect)
	{
		this->init();
	}

	ParticleEffectHandle::ParticleEffectHandle(ParticleEffectHandlePtr& rhs)
		: particleEffect(rhs->particleEffect)
	{
		this->init();
	}

	ParticleEffectHandle::~ParticleEffectHandle()
	{
		if (this->particleEffect.get())
		{
			this->particleEffect->onUpdateHeap.removeKeyCallback(nullptr, this);
		}
	}

	void ParticleEffectHandle::init()
	{
		if (this->particleEffect.get())
		{
			CallbackPtr callback = createCallbackArg0(&ParticleEffectHandle::onEffectChanged, this);
			this->particleEffect->onUpdateHeap.addKeyCallback(callback, nullptr, this);
		}
	}

	void ParticleEffectHandle::onEffectChanged()
	{
		this->onChanged.invoke();
	}

	void ParticleEffectHandle::onEffectResourcePreChanged()
	{
		if (this->particleEffect.get())
		{
			this->particleEffect->onUpdateHeap.removeKeyCallback(nullptr, this);
		}
	}

	void ParticleEffectHandle::onEffectResourcePostChanged()
	{
		this->init();
		this->onChanged.invoke();
	}

	size_t ParticleEffectHandle::calculateParticlesThisFrame(float32 dt, float32& emissionCounter)
	{
		if (!this->particleEffect.get())
			return 0;

		return this->particleEffect->calculateParticlesThisFrame(dt, emissionCounter);
	}

	void ParticleEffectHandle::onPostLoad(const LoadFlagMask& flags)
	{
		this->init();
		if (this->particleEffect.get())
		{
			this->particleEffect->onPostLoad(flags);
		}
	}
    
    template <>
    std::string SerializableHandle<ParticleEffectData>::getExtension()
    {
        return "fx";
    }
    
    template <>
    std::string SerializableHandle<ParticleEffectData>::getDescription()
    {
        return "CSE ParticleEffect";
    }

}
