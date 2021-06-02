#pragma once

#include "ClassDef.h"

#include "gfx/DynamicGeometry.h"
#include "fx/ParticleEffect.h"
#include "gfx/DisplayList.h"
#include "global/Timer.h"
#include <list>

namespace cs
{
	class ParticleEmitter;

	struct ParticleHeapUpdate
	{
		static void updateColorDefault(ColorB* dst, ParticleBuffer& buffer, size_t index);
		static void updateColorBuffer(ColorB* dst, ParticleBuffer& buffer, size_t index);
		
	};

	CLASS_DEFINITION(ParticleHeap)
	public:
		ParticleHeap(ParticleEffectPtr& eff);
		virtual ~ParticleHeap();
	
		void setContentScale(float32 cs) { this->contentScale = cs; }

		size_t updateVertices(uchar* data, size_t bufferSize, VertexDeclaration& decl);
		size_t updateIndices(uchar* data, size_t bufferSize);
		void setDrawParams(int32 index, std::vector<DrawCallPtr>& dcs);

		void process(float32 dt);
		void flush(DisplayListTraversal& traversal_list);
		void draw();

		void link(ParticleEmitter* emitter);
		void unlink(ParticleEmitter* emitter);

		size_t getLinks() { return this->links.size(); }
		size_t getNumParticles() const {return this->numParticles; }
		size_t addParticles(ParticleInitList& particle_list);
		bool hasGracePeriodExpired() const { return this->gracePeriod <= 0; }
		void resetGracePeriod() { this->gracePeriod = 5; }

		const ParticlePropertyMask& getMask() const 
		{ 
			assert(this->buffer);
			return this->buffer->getMask(); 
		}

		size_t getVertexBufferSize();
		size_t getIndexBufferSize();

		bool hasActiveParticles(void* owner_ptr) const;
		size_t removeParticlesByOwner(void* owner_ptr);

		ParticleEffectPtr& getEffect() { return this->effect; }

		bool isExpired() const;

		static void resetStats();
		static int32 gTotalParticles;
		static int32 gTotalHeapSize;

	private:
    
        void setMaxParticles(int32 max_particles);

		float32 contentScale;
		size_t maxParticles;
		size_t numParticles;
		int32 didResize;
		Timer::Tick lastTick;
		int32 gracePeriod;

		ParticleEffectPtr effect;
		DynamicGeometryPtr geom;
		ParticleBuffer* buffer;

		void addParticleToOwner(void* owner_ptr);
		void removeParticleFromOwner(void* owner_ptr);
	
		std::list<ParticleEmitter*> links;

		typedef std::unordered_map<void*, size_t> ParticleCreatorMap;
		ParticleCreatorMap owners;

		typedef void(*updateColorFunc)(ColorB*, ParticleBuffer&, size_t);
		updateColorFunc updateColorCallback;

	};

	struct ParticleHeapCollection
	{
		ParticleHeapCollection()
			: renderManually(false)
		{ }

		ParticleHeapPtr getHeap(ParticleEffectPtr& effect, RenderTraversal traversal);
		void clearUnusedHeaps();

		typedef std::pair<ParticleEffectPtr, size_t> ParticleEffectHeapMask;
		typedef std::map<ParticleEffectHeapMask, ParticleHeapPtr> ParticleHeapMap;
		ParticleHeapMap buffers[RenderTraversalMAX];
		bool renderManually;
	};
}
