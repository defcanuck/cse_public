#include "PCH.h"

#include "fx/Particle.h"

namespace cs
{
	const char* kParticlePropertyStr[] =
	{
		"Owner",
		"Lifetime",
		"Process Time",
		"Time",
		"Position",
		"Velocity",
		"Acceleration",
		"Orientation",
		"Color",
		"ColorRange",
		"Size",
		"SizeRange",
		"Angle",
		"AngleSpeed",
		"Index"
	};


	void ParticleBuffer::print(std::stringstream& oss)
	{
		for (size_t e = 0; e < this->numElements; e++)
		{
			size_t element_offset = this->currentSize * e;
			void* data = PTR_ADD(this->bufferData, element_offset);

			oss << "Particle " << e << std::endl;
			for (size_t i = 0; i < ParticlePropertyMAX; i++)
			{
				ParticleProperty prop = static_cast<ParticleProperty>(i);
				if (this->propertyMask.test(prop))
				{
					std::unordered_map<ParticleProperty, size_t>::iterator it = this->offsets.find(prop);
					if (it == this->offsets.end())
					{
						log::error("Property does not exist");
						continue;
					}

					oss << kParticlePropertyStr[i] << " ";
					void* ptr_data = PTR_ADD(data, it->second);
					ParticlePropertyManager::getInstance()->propertyData[i]->print(oss, ptr_data);
					oss << std::endl;
				}
			}
		}
	}

	void ParticleBuffer::update(float32 dt, float32 pct, size_t index)
	{
		for (auto& it : this->updateProperties)
		{
			it->update(index, this, dt, pct);
		}
	}
}