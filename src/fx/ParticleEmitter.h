#pragma once

#include "ClassDef.h"

#include "fx/ParticleEffect.h"
#include "fx/ParticleHeap.h"
#include "fx/ParticleScript.h"

#include "animation/Animator.h"
#include "scene/SceneNode.h"
#include "global/Timer.h"

#include "gfx/RenderInterface.h"

namespace cs
{
	class ParticleEmitter;

	struct ParticleEmitterInstance
	{
		ParticleEmitterInstance()
			: type(EmissionTypeNone)
			, anim(FloatAnimation())
			, emissionCounter(0.0f)
		{ }

		ParticleEmitterInstance(ParticleEffect* effect);

		ParticleEmitterInstance(const ParticleEmitterInstance& rhs)
			: type(rhs.type)
			, anim(rhs.anim)
			, emissionCounter(rhs.emissionCounter)
		{ }
	
		inline size_t process(ParticleEffect* effect, float32 dt)
		{
			switch (this->type)
			{
				case EmissionTypeInfinite:
				case EmissionTypeBurst:
					return effect->calculateParticlesThisFrame(dt, this->emissionCounter);
				case EmissionTypeTimed:
					this->anim.process(dt);
					if (!this->anim.isAnimDone())
					{
						return effect->calculateParticlesThisFrame(dt, this->emissionCounter);
					}
			}
			return 0;
		}

		EmissionType type;
		FloatAnimation anim;
		float32 emissionCounter;

		ParticleScriptProperties scriptProperties;
	};

	// why is this forward declared?
	class ParticleEmitter;
	CLASS_DEFINITION_DERIVED_REFLECT(ParticleEmitterAnchor, SceneNode)
	public:
		
		ParticleEmitterAnchor();
		ParticleEmitterAnchor(const std::string& effectName, RenderTraversal pass);
		
		virtual ~ParticleEmitterAnchor();
		
		void setEffect(const std::string& effectName);
		void process(float32 dt);

		std::shared_ptr<ParticleEmitter>& getEmitter()
		{
			return this->emitter;
		}

	private:

		RenderTraversal traversal;
		std::shared_ptr<ParticleEmitter> emitter;
	};

	CLASS_DEFINITION_REFLECT(ParticleEmitter)
	public:
		
		typedef std::function<vec3()> GetWorldPositionFunc;
		typedef std::function<quat()> GetWorldOrientationFunc;
		typedef std::function<ColorB()> GetTintFunc;

		struct SpawnParams
		{
			SpawnParams()
				: contentScale(1.0f)
				, renderManually(false)
			{ }

			ParticleEmitter::GetWorldPositionFunc worldFunc;
			float32 contentScale;
			bool renderManually;
		};

		ParticleEmitter();
		ParticleEmitter(ParticleEffectHandlePtr& effect, RenderTraversal pass);
		ParticleEmitter(const std::string& name, std::shared_ptr<ParticleHeapCollection>& col, const SpawnParams& params, RenderTraversal pass);

		virtual ~ParticleEmitter();
		void clearCallbacks();

		virtual void onNew();
		virtual void onPostLoad(const LoadFlagMask& flags);

		void process(float32 dt, bool forceUpdate = false);

		void setWorldPosCallbackFunc(GetWorldPositionFunc func) { this->worldPosCallback = func; }
		void setWorldOrientCallbackFunc(GetWorldOrientationFunc func) { this->worldOrientCallback = func; }
		void setTintCallbackFunc(GetTintFunc func) { this->tintCallback = func; }

		void setEmitting(bool isOn) { this->emitting = isOn; }
		bool getEmitting() const { return this->emitting; }

		void setDeferredDraw(bool isOn) { this->hasDeferredDraw = isOn; }
		bool getDeferredDraw() const { return this->hasDeferredDraw; }

		void spawn(vec3 particle_pos);
		void spawn(std::vector<vec3>& particle_positions);

		static void burstTint(const std::string& effectName, const vec3& world_position, const ColorB& tint, RenderTraversal traversal);
		static void burst(const std::string& effectName, const vec3& world_position, RenderTraversal traversal);
		
		void kill();
		void reset();
		void resetHeap();

		Timer::Tick getMaxLifetime();

		Event onChanged;

		void overrideVelocity(const vec3& direction, const vec3& variance, float32 speed, float32 speedVariance);

		ParticleEffectHandlePtr& getParticleEffectHandle() { return this->particleEffect; }
		ParticleHeapPtr& getHeap() { return this->heap; }

		static std::string getStats();

	private:
		
		void clearHeap();
		void init(const SpawnParams& params);

		void onEffectChanged();
		void onEffectHandleChanged();
		
		void initEffect();
		void initHeap(const SpawnParams& params);

		RenderTraversalBasePtr traversal;
		ParticleEffectHandlePtr particleEffect;
		ParticleHeapPtr heap;
		ParticleEmitterInstance instance;
		std::shared_ptr<ParticleHeapCollection> collection;

		GetWorldPositionFunc worldPosCallback;
		GetWorldOrientationFunc worldOrientCallback;
		GetTintFunc tintCallback;

		bool hasDeferredDraw;
		bool emitting;

		SpawnParams spawnParams;

	};

	struct ParticleEmitterScope
	{
		
		ParticleEmitterScope()
		{ }

		typedef std::shared_ptr<ParticleHeapCollection> ParticleHeapCollectionPtr;
		typedef std::unordered_map<std::string, ParticleEmitterPtr> ParticleEmitterMap;
		typedef std::vector<ParticleHeapPtr> ParticleUnmanagedHeapList;
		typedef std::vector<ParticleEmitterPtr> ParticleEmitterList;
		typedef std::map<std::string, ParticleHeapCollectionPtr> ParticleHeapMap;
		
		void process(float32 dt);
		void draw(RenderTraversal traversal = RenderTraversalMain);

		bool exists(const std::string& tag);
		bool remove(const std::string& tag);
		void removeAll();

		std::shared_ptr<ParticleHeapCollection> getHeap(const std::string& tag, bool renderManually = false);
		ParticleEmitterPtr addEmitter(const std::string& tag, const std::string& effectName, ParticleEmitter::SpawnParams& params, RenderTraversal traversal);

		ParticleEmitterMap emitters;
		ParticleHeapMap heaps;
	};
}