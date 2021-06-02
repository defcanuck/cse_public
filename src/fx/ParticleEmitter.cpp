#include "PCH.h"

#include "fx/ParticleEmitter.h"
#include "ecs/system/ParticleSystem.h"
#include "global/ResourceFactory.h"

namespace cs
{
	BEGIN_META_CLASS(ParticleEmitter)
		ADD_META_FUNCTION("Kill Emitter", &ParticleEmitter::kill);
	ADD_META_FUNCTION("Reset Emitter", &ParticleEmitter::reset);
	ADD_META_FUNCTION("Reset Heap", &ParticleEmitter::resetHeap);
	ADD_MEMBER_PTR(particleEffect);
	SET_MEMBER_CALLBACK_POST(&ParticleEmitter::onEffectHandleChanged);
	ADD_MEMBER(emitting);
	SET_MEMBER_IGNORE_SERIALIZATION();
	ADD_MEMBER_PTR(traversal);
	ADD_COMBO_META_LABEL(RenderTraversalMainPass, "Main");
	ADD_COMBO_META_LABEL(RenderTraversalShadowPass, "Shadow");
	END_META();

	BEGIN_META_CLASS(ParticleEmitterAnchor)

		END_META()

		ParticleEmitterAnchor::ParticleEmitterAnchor()
		: emitter(nullptr)
	{

	}

	ParticleEmitterAnchor::~ParticleEmitterAnchor()
	{
		this->emitter->clearCallbacks();
	}

	ParticleEmitterAnchor::ParticleEmitterAnchor(const std::string& effectName, RenderTraversal pass)
		: emitter(nullptr)
		, traversal(pass)
	{
		this->setEffect(effectName);
	}

	void ParticleEmitterAnchor::setEffect(const std::string& effectName)
	{
		ParticleEffectPtr effect = std::static_pointer_cast<ParticleEffect>(ResourceFactory::getInstance()->loadResource<ParticleEffect>(effectName));
		if (effect.get())
		{
			ParticleEffectHandlePtr handle = CREATE_CLASS(ParticleEffectHandle, effect);
			this->emitter = CREATE_CLASS(ParticleEmitter, handle, this->traversal);

			this->emitter->setWorldPosCallbackFunc(std::bind(&SceneNode::getWorldPosition, this));
			this->emitter->setWorldOrientCallbackFunc(std::bind(&SceneNode::getWorldRotation, this));
		}
	}

	void ParticleEmitterAnchor::process(float32 dt)
	{
		if (this->emitter)
		{
			this->emitter->process(dt);
		}
	}

	ParticleEmitterInstance::ParticleEmitterInstance(ParticleEffect* effect)
		: type(EmissionTypeNone)
		, anim(FloatAnimation())
		, emissionCounter(0.0f)
	{

		float32 emitterTime = 1.0f;
		ParticleEffectDataPtr& data = effect->getParticleEffectData();
		if (data.get())
		{
			this->type = data->emissionHandler->getType();
			emitterTime = data->emissionHandler->getLifetime();
		}

		if (this->type == EmissionTypeTimed && emitterTime > 0.0f)
		{
			std::shared_ptr<FloatLerpAnimator> animator = CREATE_CLASS(FloatLerpAnimator, 0.0f, 1.0f, emitterTime);
			std::shared_ptr<AnimatorTyped<float32>> anim = std::static_pointer_cast<AnimatorTyped<float32>>(animator);
			this->anim = FloatAnimation(anim, AnimationTypeNone);
			this->anim.setInitialValue(0.0f);
		}
	}

	ParticleEmitter::ParticleEmitter()
		: particleEffect(CREATE_CLASS(ParticleEffectHandle))
		, worldPosCallback(nullptr)
		, worldOrientCallback(nullptr)
		, emitting(true)
		, hasDeferredDraw(false)
		, collection(nullptr)
		, traversal(RenderTraversalBase::getTypePtr(RenderTraversalMain))
	{

	}

	ParticleEmitter::ParticleEmitter(ParticleEffectHandlePtr& effect, RenderTraversal pass)
		: particleEffect(CREATE_CLASS(ParticleEffectHandle, effect->getEffect()))
		, worldPosCallback(nullptr)
		, worldOrientCallback(nullptr)
		, emitting(true)
		, hasDeferredDraw(false)
		, collection(nullptr)
		, traversal(RenderTraversalBase::getTypePtr(pass))
	{
		SpawnParams params;
		this->init(params);
	}

	ParticleEmitter::ParticleEmitter(const std::string& name, std::shared_ptr<ParticleHeapCollection>& col, const SpawnParams& params, RenderTraversal pass)
		: particleEffect(CREATE_CLASS(ParticleEffectHandle, name))
		, worldPosCallback(nullptr)
		, worldOrientCallback(nullptr)
		, emitting(true)
		, hasDeferredDraw(false)
		, collection(col)
		, traversal(RenderTraversalBase::getTypePtr(pass))
	{
		this->init(params);
	}

	ParticleEmitter::~ParticleEmitter()
	{
		this->clearHeap();
		if (this->particleEffect.get())
		{
			this->particleEffect->onChanged.removeKeyCallback(nullptr, this);
		}
	}

	void ParticleEmitter::onNew()
	{
		this->particleEffect = CREATE_CLASS(ParticleEffectHandle, RenderInterface::kDefaultParticleEffect);
		SpawnParams params;
		this->init(params);
	}

	void ParticleEmitter::onPostLoad(const LoadFlagMask& flags)
	{
		SpawnParams params;
		this->init(params);
	}

	void ParticleEmitter::init(const SpawnParams& params)
	{
		if (this->particleEffect.get())
		{
			CallbackPtr callback = createCallbackArg0(&ParticleEmitter::onEffectChanged, this);
			this->particleEffect->onChanged.addKeyCallback(callback, nullptr, this);
		}

		this->initEffect();
		this->initHeap(params);
	}

	void ParticleEmitter::onEffectHandleChanged()
	{
		this->clearHeap();
		this->initHeap(this->spawnParams);
		this->initEffect();
	}

	void ParticleEmitter::initEffect()
	{
		if (!this->particleEffect.get())
			return;

		// create the emitter instance
		ParticleEffectPtr& effect = this->particleEffect->getEffect();
		if (effect)
		{
			this->instance = ParticleEmitterInstance(effect.get());
		}
	}

	void ParticleEmitter::initHeap(const SpawnParams& params)
	{
		if (!this->particleEffect.get() || this->heap.get())
			return;

		ParticleEffectPtr& effect = this->particleEffect->getEffect();
		if (!effect.get())
		{
			log::info("Cannot create emitter until a particle effect is set");
			return;
		}

		if (effect && effect->getMaxParticles() <= 0)
		{
			log::info("No heap for an effect with 0 particles!");
			return;
		}

		if (!this->collection)
		{
			this->collection = ParticleSystem::getInstance()->heaps;
		}

		// find a heap for this emitter
		ParticleHeapPtr heap_to_use = this->collection->getHeap(effect, this->traversal->getType());
		
		if (heap_to_use)
		{
			this->heap = heap_to_use;
			this->heap->link(this);
			this->heap->setContentScale(params.contentScale);
			this->heap->resetGracePeriod();
		}

		this->spawnParams = params;
	}

	void ParticleEmitter::clearHeap()
	{
		if (this->heap)
		{
			this->heap->unlink(this);
			this->heap = nullptr;
			assert(this->collection);
			this->collection->clearUnusedHeaps();
		}
	}

	void ParticleEmitter::onEffectChanged()
	{
		this->onChanged.invoke();

		if (this->particleEffect.get() && this->particleEffect->getEffect().get())
		{
			if (!this->heap.get())
			{
				log::info("Creating new heap for Emitter");
				this->initEffect();
				this->initHeap(this->spawnParams);
			}
			else if (this->particleEffect->getEffect().get() != this->heap->getEffect().get() ||
				!(this->heap->getMask() == this->particleEffect->getEffect()->getMask()))
			{
				log::info("Reassigning heap due to Particle Mask mismatch");
				this->onEffectHandleChanged();
			}
		}
	}

	void ParticleEmitter::spawn(std::vector<vec3>& particle_positions)
	{
		ParticleEffectPtr& effect = this->particleEffect->getEffect();
		if (!effect.get())
		{
			log::info("Cannot spawn particle - no Effect defined!");
			return;
		}

		ParticleInitList particle_list(this, &this->instance.scriptProperties);
		effect->createParticles(particle_list, particle_positions.size());

		particle_list.numParticles =
			std::min <size_t>(particle_list.numParticles, particle_positions.size());

		for (size_t i = 0; i < particle_list.numParticles; i++)
		{
			particle_list.initList[i].position = particle_positions[i];
		}

		if (this->heap)
		{
			this->heap->addParticles(particle_list);
		}
	}

	void ParticleEmitter::spawn(vec3 particle_pos)
	{
		std::vector<vec3> particle_positions;
		particle_positions.push_back(particle_pos);
		this->spawn(particle_positions);
	}

	void ParticleEmitter::resetHeap()
	{
		ParticleEffectPtr& effect = this->particleEffect->getEffect();
		ParticlePropertyMask prev_mask = effect->getMask();
		ParticleEffectDataPtr& data = effect->getParticleEffectData();
		if (data)
		{
			data->updateMask();
			this->onEffectHandleChanged();
		}
	}

	void ParticleEmitter::process(float32 dt, bool forceUpdate)
	{
		// ignore large delta times
		if (!forceUpdate && (dt < 0.001 || dt > 1.0f))
			return;

		vec3 world_pos = kZero3;
		quat world_rot(Transform::kDefaultRotation);
		ColorB tint = ColorB::White;

		if (this->worldPosCallback)
		{
			world_pos = this->worldPosCallback();
		}

		if (this->worldOrientCallback)
		{
			world_rot = this->worldOrientCallback();
		}
		if (this->tintCallback)
		{
			tint = this->tintCallback();
		}

		if (this->emitting)
		{

			ParticleEffectPtr& effect = this->particleEffect->getEffect();
			if (effect.get())
			{
				size_t particles_to_spawn = std::min<size_t>(this->instance.process(effect.get(), dt), 100);
				if (particles_to_spawn > 0)
				{
					ParticleInitList particle_list(this, &this->instance.scriptProperties);
					particle_list.tint = tint;

					effect->createParticles(particle_list, particles_to_spawn, &this->instance.scriptProperties);

					for (size_t i = 0; i < particle_list.numParticles; i++)
					{
						vec3 pos = particle_list.initList[i].position * this->spawnParams.contentScale;
						particle_list.initList[i].position = (world_rot * pos) + world_pos;
						particle_list.initList[i].rotation = world_rot;
					}

					if (this->heap)
					{
						this->heap->addParticles(particle_list);
					}
				}
			}
		}
	}

	void ParticleEmitter::kill()
	{
		this->reset();
		this->setEmitting(false);
	}

	void ParticleEmitter::reset()
	{
		if (!this->heap)
		{
			log::info("No heap attacted to emitter");
			return;
		}

		this->heap->removeParticlesByOwner(this);
		this->setEmitting(true);
	}

	void ParticleEmitter::burst(const std::string& effectName, const vec3& world_position, RenderTraversal traversal)
	{
		ParticleEmitter::burstTint(effectName, world_position, ColorB::White, traversal);
	}

	void ParticleEmitter::burstTint(const std::string& effectName, const vec3& world_position, const ColorB& tint, RenderTraversal traversal)
	{
		ParticleEmitter::GetWorldPositionFunc func = [=]()
		{
			return world_position;
		};

		ParticleEmitter::GetTintFunc tintFunc = [=]()
		{
			return tint;
		};

		ParticleEffectPtr effect = std::static_pointer_cast<ParticleEffect>(ResourceFactory::getInstance()->loadResource<ParticleEffect>(effectName));
		if (!effect.get())
		{
			log::error("Cannot find effect: ", effectName);
			return;
		}

		ParticleEffectDataPtr data = effect->getParticleEffectData();
		if (data->emissionHandler->getType() != EmissionTypeBurst)
		{
			log::error("Cannot create burst with a non-burst emission handler");
			return;
		}

		ParticleEffectHandlePtr handle = CREATE_CLASS(ParticleEffectHandle, effect);
		ParticleEmitterPtr emitter = CREATE_CLASS(ParticleEmitter, handle, traversal);

		emitter->setWorldPosCallbackFunc(func);
		if (tint != ColorB::White)
		{
			emitter->setTintCallbackFunc(tintFunc);
		}
		emitter->process(1.0f / 20.0f);
	}


	Timer::Tick ParticleEmitter::getMaxLifetime()
	{
		if (!this->particleEffect.get())
		{
			return 0;
		}

		ParticleEffectDataPtr data = this->particleEffect->getEffectData();
		if (!data.get())
		{
			return 0;
		}

		return static_cast<uint32>(data->lifeTime->getMaxValue() * 1000);
	}

	void ParticleEmitter::overrideVelocity(const vec3& direction, const vec3& variance, float32 speed, float32 speedVariance)
	{
		ParticleScriptProperty* prop = new ParticleScriptVelocity(direction, variance, speed, speedVariance);
		this->instance.scriptProperties.setProperty(ParticlePropertyVelocity, prop);
	}

	void ParticleEmitter::clearCallbacks()
	{
		this->worldPosCallback = nullptr;
		this->worldOrientCallback = nullptr;
	}

	void ParticleEmitterScope::process(float32 dt)
	{
		for (auto& it : this->emitters)
		{
			if (it.second.get())
			{
				it.second->process(dt);
			}
		}

		for (auto& heap_collection : this->heaps)
		{
			for (int32 traversal = 0; traversal < RenderTraversalMAX; ++traversal)
			{
				for (auto& it : heap_collection.second->buffers[traversal])
				{
					ParticleHeapPtr& heap = it.second;
					heap->process(dt);
				}
			}
		}
	}

	void ParticleEmitterScope::draw(RenderTraversal traversal)
	{
		for (auto& heap_collection : this->heaps)
		{
			if (heap_collection.second->renderManually)
				continue;

			for (auto& it : heap_collection.second->buffers[traversal])
			{
				ParticleHeapPtr& heap = it.second;
				heap->draw();
			}
		}
	}


	std::shared_ptr<ParticleHeapCollection> ParticleEmitterScope::getHeap(const std::string& tag, bool renderManually)
	{

		ParticleHeapMap::iterator it = this->heaps.find(tag);
		if (it == this->heaps.end())
		{
			std::shared_ptr<ParticleHeapCollection> newHeap = CREATE_CLASS(ParticleHeapCollection);
			newHeap->renderManually = true;
			this->heaps[tag] = newHeap;
			return newHeap;
		}

		return it->second;

	}

	ParticleEmitterPtr ParticleEmitterScope::addEmitter(const std::string& tag, const std::string& effectName, ParticleEmitter::SpawnParams& params, RenderTraversal traversal)
	{
		ParticleEmitterMap::iterator it = this->emitters.find(tag);
		if (it != this->emitters.end())
		{
			log::error("Duplicate emitter tag - ", tag);
			return ParticleEmitterPtr();
		}

		std::shared_ptr<ParticleHeapCollection> heapToUse = this->getHeap(tag, params.renderManually);
		ParticleEmitterPtr emitter = CREATE_CLASS(ParticleEmitter, effectName, heapToUse, params, traversal);
		emitter->setWorldPosCallbackFunc(params.worldFunc);

		ParticleEffectHandlePtr handle = emitter->getParticleEffectHandle();
		ParticleEffectDataPtr data = handle->getEffectData();

		// trigger a burst but don't save off the emitter - it's a one-off
		if (data->emissionHandler->getType() == EmissionTypeBurst)
		{
			emitter->process(0.0f, true);
		}

		this->emitters[tag] = emitter;
		
		return emitter;
	}


	bool ParticleEmitterScope::exists(const std::string& tag)
	{
		return this->emitters.find(tag) != this->emitters.end();
	}

	bool ParticleEmitterScope::remove(const std::string& tag)
	{
		ParticleEmitterMap::iterator it = this->emitters.find(tag);
		if (it != this->emitters.end())
		{
			it->second->clearCallbacks();
			this->emitters.erase(tag);
			return true;
		}
		return false;
	}

	void ParticleEmitterScope::removeAll()
	{
		this->emitters.clear();
		for (auto& heap_collection : this->heaps)
		{
			heap_collection.second->clearUnusedHeaps();
		}
		this->heaps.clear();
	}


	std::string ParticleEmitter::getStats()
	{
		std::stringstream str;
		str << "Particles: " << ParticleHeap::gTotalParticles << " Heap Size: " << ParticleHeap::gTotalHeapSize;
		return str.str();
	}

}
