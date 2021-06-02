#pragma once

#include "ecs/comp/Component.h"

#include "fx/ParticleEffect.h"
#include "fx/ParticleEmitter.h"

namespace cs
{
	CLASS_DEFINITION_DERIVED_REFLECT(ParticleComponent, Component)
	public:
		ParticleComponent();
		virtual ~ParticleComponent();

		void process(float32 dt);

		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);
		virtual void onNew();

	private:

		void init();
		void onEmitterChanged();

		ParticleEmitterPtr emitter;

	};
}