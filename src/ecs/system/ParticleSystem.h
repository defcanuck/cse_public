#pragma once

#include "ecs/system/BaseSystem.h"

#include "fx/Particle.h"
#include "fx/ParticleHeap.h"

namespace cs
{

	class ParticleSystem : public ECSContextSystemBase<ParticleSystem, ECSParticle>, public BaseSystem
	{
	public:
        
        
		ParticleSystem(ECSContext* cxt);
		virtual ~ParticleSystem();

		virtual void processImpl(SystemUpdateParams* params);
        void clear();
		void flush(DisplayList& display_list);

		std::shared_ptr<ParticleHeapCollection> heaps;

	};
}
