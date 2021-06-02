#pragma once

#include "ClassDef.h"

#include "math/GLM.h"
#include "geom/Shape.h"
#include "geom/Volume.h"

namespace cs
{
	CLASS_DEFINITION_REFLECT(LiquidGenerator)
	public:
		LiquidGenerator() { }
		LiquidGenerator(uint32 num) { }
 
		virtual void populate(std::vector<vec2>& positions, uint32 requested = 0)
		{
			assert(false);
		}

		virtual bool intersects(Ray& ray, vec3& hit_pos) 
		{
			assert(false); 
			return false;
		}

		virtual VolumePtr getVolume() { return VolumePtr(nullptr); }

	protected:

	};

	typedef std::shared_ptr<LiquidGenerator> LiquidGeneratorPtr;

	CLASS_DEFINITION_DERIVED_REFLECT(LiquidGeneratorRandom, LiquidGenerator)
	public:
		LiquidGeneratorRandom();
		LiquidGeneratorRandom(uint32 num);
		LiquidGeneratorRandom(uint32 num, VolumePtr& vol);

		virtual void populate(std::vector<vec2>& positions, uint32 requested = 0);
		virtual bool intersects(Ray& ray, vec3& hit_pos);
		virtual VolumePtr getVolume() { return this->volume; }
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);

	private:

		VolumePtr volume;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(LiquidGeneratorPreset, LiquidGenerator)
	public:

		LiquidGeneratorPreset() :
			LiquidGenerator() { }

		virtual void populate(std::vector<vec2>& positions, uint32 requested = 0);
		virtual bool intersects(Ray& ray, vec3& hit_pos); 
		virtual VolumePtr getVolume() { return this->volume; }
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);

		void addParticle(const vec2 pos);

	private:
		
		void updateVolume();

		VolumePtr volume;
		Vec2List particles;
	
	};

	CLASS_DEFINITION_DERIVED_REFLECT(LiquidGeneratorGrid, LiquidGenerator)
	public:

		typedef std::vector<vec2> InitPositionVector;
		LiquidGeneratorGrid();
		LiquidGeneratorGrid(uint32 num);
		LiquidGeneratorGrid(uint32 num, VolumePtr& vol);

		virtual void populate(std::vector<vec2>& positions, uint32 requested = 0);
		virtual bool intersects(Ray& ray, vec3& hit_pos);
		virtual VolumePtr getVolume() { return this->volume; }
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);

		void addParticle(const vec2& pos);

	private:

		void updateVolume();

		VolumePtr volume;
		InitPositionVector particles;

	};
}
