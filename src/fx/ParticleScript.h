#pragma once

#include "fx/Particle.h"
#include "gfx/Color.h"

namespace cs
{

	struct ParticleScriptProperty
	{
		virtual void populate(ParticlePropertyDataMap& data) const { }
	};

	struct ParticleScriptProperties
	{	
		typedef std::unordered_map<ParticleProperty, ParticleScriptProperty*> ParticleScriptOverrideMap;

		ParticleScriptProperties()
		{ }

		~ParticleScriptProperties()
		{
			for (auto& it : this->overrides)
			{
				delete it.second;
			}
			this->overrides.clear();
		}

		void setProperty(ParticleProperty type, ParticleScriptProperty* prop)
		{
			this->clearProperty(type);
			this->overrides[type] = prop;
		}

		void clearProperty(const ParticleProperty& prop)
		{
			ParticleScriptOverrideMap::iterator it = this->overrides.find(prop);
			if (it != this->overrides.end())
			{
				delete it->second;
				this->overrides.erase(it);
			}
		}

		ParticlePropertyMask getMask() const
		{
			ParticlePropertyMask mask;
			for (auto& it : this->overrides)
			{
				mask.set(it.first);
			}
			return mask;
		}

		ParticleScriptOverrideMap overrides;
	};

	struct ParticleScriptVelocity : public ParticleScriptProperty
	{
		explicit ParticleScriptVelocity(const vec3& dir, const vec3& var, float32 s, float32 sv)
			: direction(dir)
			, variance(var)
			, speed(s)
			, speedVariance(sv)
		{ }

		virtual void populate(ParticlePropertyDataMap& data) const
		{
			assert(data.find(ParticlePropertyVelocity) == data.end());
			vec3 direction_range = this->direction + (variance * randomRange(-1.0f, 1.0f));
			direction_range = glm::normalize(direction_range) * (this->speed + randomRange(-this->speedVariance, this->speedVariance));
			addPropertyForce(data, ParticlePropertyVelocity, direction_range);
		}

		vec3 direction;
		vec3 variance;
		float32 speed;
		float32 speedVariance;
	};
}