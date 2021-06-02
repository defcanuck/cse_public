#pragma once

#include "math/GLM.h"
#include "gfx/Color.h"
#include "os/LogManager.h"

#include "global/Utils.h"
#include "global/BitMask.h"
#include "global/Callback.h"
#include "global/Singleton.h"
#include "math/Transform.h"

#include <unordered_map>

#define MAX_EMITTER_PARTICLES 256

namespace cs
{

	enum ParticleProperty
	{
		ParticlePropertyNone = -1,
		ParticlePropertyOwner,
		ParticlePropertyLifetime,
		ParticlePropertyProcessTime,
		ParticlePropertyTime,
		ParticlePropertyPosition,
		ParticlePropertyVelocity,
		ParticlePropertyAcceleration,
		ParticlePropertyOrientation,
		ParticlePropertyColor,
		ParticlePropertyColorRange,
		ParticlePropertyColorKeyFrame,
		ParticlePropertySize,
		ParticlePropertySizeRange,
		ParticlePropertyAngle,
		ParticlePropertyAngleSpeed,
		ParticlePropertyIndex,
		ParticlePropertyMAX
	};

	typedef BitMask<ParticleProperty, ParticlePropertyMAX> ParticlePropertyMask;
	extern const char* kParticlePropertyStr[];

	struct ParticlePropertyUpdate
	{
		template <class T>
		static void lerpUpdateFunc(float32 dt, float32 pct, RangeValue<T>* range, T* val)
		{
			RangeValue<T>::applyLerp(pct, range, val);
		}

		template <class T>
		static void eulerUpdateFunc(float32 dt, float32 pct, T* src, T* dst)
		{
			(*dst) = (*dst) + ((*src) * dt);
		}
	};

	struct ParticlePropertyValue
	{
		ParticlePropertyValue()
		{ }

		virtual ~ParticlePropertyValue()
		{ }

		virtual void* getData() const { return nullptr; }
		virtual size_t getSize() const { return 0; }
	};

	template <typename T>
	struct ParticlePropertyValueTyped : public ParticlePropertyValue
	{
		ParticlePropertyValueTyped(T* data_ptr)
			: data(data_ptr)
		{ }

		virtual ~ParticlePropertyValueTyped()
		{
			if (this->data)
				delete this->data;
		}

		virtual void* getData() const { return (void*) this->data; }
		virtual size_t getSize() const { return sizeof(T); }

		T* data;
	};

	typedef std::map<ParticleProperty, ParticlePropertyValue*> ParticlePropertyDataMap;

	template <typename T>
	void addProperty(ParticlePropertyDataMap& map, ParticleProperty prop, const T& value, ParticlePropertyMask& ignoreMask)
	{
		if (!ignoreMask.test(prop)) map[prop] = new ParticlePropertyValueTyped<T>(new T(value));
	}

	template <typename T>
	void addPropertyForce(ParticlePropertyDataMap& map, ParticleProperty prop, const T& value)
	{
		map[prop] = new ParticlePropertyValueTyped<T>(new T(value));
	}

	struct ParticleInitProps
	{
		ParticleInitProps()
			: position(kZero3)
			, rotation(Transform::kDefaultRotation)
			, lifeTime(1.0f)
			, processTime(-1.0f)

		{ }

		~ParticleInitProps()
		{
			this->clear();
		}

		void clear()
		{
			for (auto& it : this->propertyMap)
				delete it.second;
			this->propertyMap.clear();
		}

		vec3 position;
		quat rotation;
		float32 lifeTime;
		float32 processTime;
		ColorB tint;

		ParticlePropertyDataMap propertyMap;
	};

	struct ParticleScriptProperties;
	
	struct ParticleInitList
	{
		ParticleInitList(void* creator_ptr, ParticleScriptProperties* script_props = nullptr)
			: creator(creator_ptr)
			, numParticles(0)
			, scriptProperties(script_props)
			, tint(ColorB::White)
		{ }

		ParticleInitProps& next() 
		{
			assert(this->numParticles < MAX_EMITTER_PARTICLES);
			return this->initList[this->numParticles++]; 
		}

		void* creator;
		size_t numParticles;
		ParticleInitProps initList[MAX_EMITTER_PARTICLES];
		ParticleScriptProperties* scriptProperties;
		ColorB tint;
	};

	class ParticleBuffer;

	struct ParticlePropertyData
	{
		ParticlePropertyData(ParticleProperty update_src, ParticleProperty update_dst = ParticlePropertyNone)
			: src(update_src)
			, dst(update_dst) 
		{ }

		virtual void print(std::stringstream& str, void* data) { assert(false); }
		virtual size_t getSize() { return 0; }
		virtual bool canUpdate() const { return false; }
		virtual void update(int32 index, ParticleBuffer* buffer, float32 dt, float32 pct) { }

		ParticleProperty getPropetySource() { return this->src; }
		ParticleProperty getPropertyDest() { return this->dst; }

	protected:

		ParticleProperty src;
		ParticleProperty dst;
	};

	template <class T>
	struct ParticlePropertyDataTyped : public ParticlePropertyData
	{
		ParticlePropertyDataTyped(ParticleProperty update_src, ParticleProperty update_dst = ParticlePropertyNone)
			: ParticlePropertyData(update_src, update_dst)
		{ }

		void print_typed(std::stringstream& str, void* data)
		{
			T* typed_data = reinterpret_cast<T*>(data);
			str << *typed_data;
		}

		virtual void print(std::stringstream& str, void* data)
		{
			this->print_typed(str, data);
		}

		virtual size_t getSize() { return sizeof(T); }
	};

	template <class T, class V>
	struct ParticlePropertyDataUpdater : public ParticlePropertyDataTyped<T>
	{
		typedef std::function<void(float32, float32, T*, V*)> TypedUpdateFunction;

		ParticlePropertyDataUpdater(ParticleProperty update_src, ParticleProperty update_dst = ParticlePropertyNone)
			: ParticlePropertyDataTyped<T>(update_src, update_dst)
		{
			
		}

		ParticlePropertyDataUpdater(TypedUpdateFunction func, ParticleProperty update_src, ParticleProperty update_dst = ParticlePropertyNone)
			: ParticlePropertyDataTyped<T>(update_src, update_dst)
			, updateFunction(func)
		{

		}

		virtual bool canUpdate() const { return true; }
		virtual void update(int32 index, ParticleBuffer* buffer, float32 dt, float32 pct);

		TypedUpdateFunction updateFunction;
	};

	class ParticlePropertyManager : public Singleton<ParticlePropertyManager>
	{
	public:
		ParticlePropertyManager()
		{

			ParticlePropertyDataUpdater<Vec2RangeValue, vec2>::TypedUpdateFunction callSizeUpdateFunc = 
				std::bind(&ParticlePropertyUpdate::lerpUpdateFunc<vec2>, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

			ParticlePropertyDataUpdater<ColorBRangeValue, ColorB>::TypedUpdateFunction callColorUpdateFunc =
				std::bind(&ParticlePropertyUpdate::lerpUpdateFunc<ColorB>, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

			ParticlePropertyDataUpdater<vec3, vec3>::TypedUpdateFunction callEulerUpdateFunc = 
				std::bind(&ParticlePropertyUpdate::eulerUpdateFunc<vec3>, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

			ParticlePropertyDataUpdater<float32, float32>::TypedUpdateFunction callAngleUpdateFunc =
				std::bind(&ParticlePropertyUpdate::eulerUpdateFunc<float32>, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

			memset(this->propertyData, 0, ParticlePropertyMAX * sizeof(ParticlePropertyData*));

			this->propertyData[ParticlePropertyOwner] = 
				new ParticlePropertyDataTyped<size_t>(ParticlePropertyOwner);
			
			this->propertyData[ParticlePropertyLifetime] = 
				new ParticlePropertyDataTyped<float32>(ParticlePropertyLifetime);

			this->propertyData[ParticlePropertyProcessTime] =
				new ParticlePropertyDataTyped<float32>(ParticlePropertyProcessTime);
			
			this->propertyData[ParticlePropertyTime] = 
				new ParticlePropertyDataTyped<float32>(ParticlePropertyTime);
			
			this->propertyData[ParticlePropertyPosition] = 
				new ParticlePropertyDataTyped<vec3>(ParticlePropertyPosition);
			
			this->propertyData[ParticlePropertyVelocity] = 
				new ParticlePropertyDataUpdater<vec3, vec3>(callEulerUpdateFunc, ParticlePropertyVelocity, ParticlePropertyPosition);

			this->propertyData[ParticlePropertyAcceleration] = 
				new ParticlePropertyDataUpdater<vec3, vec3>(callEulerUpdateFunc, ParticlePropertyAcceleration, ParticlePropertyVelocity);

			this->propertyData[ParticlePropertyOrientation] = 
				new ParticlePropertyDataTyped<quat>(ParticlePropertyOrientation);
			
			this->propertyData[ParticlePropertyColor] 
				= new ParticlePropertyDataTyped<ColorB>(ParticlePropertyColor);

			this->propertyData[ParticlePropertyColorKeyFrame]
				= new ParticlePropertyDataTyped<ColorB>(ParticlePropertyColor);

			this->propertyData[ParticlePropertyColorRange]
				= new ParticlePropertyDataUpdater<ColorBRangeValue, ColorB>(callColorUpdateFunc, ParticlePropertyColorRange, ParticlePropertyColor);

			this->propertyData[ParticlePropertySize]
				= new ParticlePropertyDataTyped<vec2>(ParticlePropertySize);

			this->propertyData[ParticlePropertySizeRange]
				= new ParticlePropertyDataUpdater<Vec2RangeValue, vec2>(callSizeUpdateFunc, ParticlePropertySizeRange, ParticlePropertySize);

			this->propertyData[ParticlePropertyAngle]
				= new ParticlePropertyDataTyped<float32>(ParticlePropertyAngle);

			this->propertyData[ParticlePropertyAngleSpeed]
				= new ParticlePropertyDataUpdater<float32, float32>(callAngleUpdateFunc, ParticlePropertyAngleSpeed, ParticlePropertyAngle );

			this->propertyData[ParticlePropertyIndex]
				= new ParticlePropertyDataTyped<int32>(ParticlePropertyIndex);

			for (size_t i = 0; i < ParticlePropertyMAX; i++)
			{
				assert(this->propertyData[i] != nullptr);
				ParticlePropertyData* prop = this->propertyData[i];
				if (prop->canUpdate())
				{
					this->updateProperties[prop->getPropetySource()] = prop;
				}
			}
		}

		virtual ~ParticlePropertyManager()
		{
			for (size_t i = 0; i < ParticlePropertyMAX; i++)
			{
				delete this->propertyData[i];
			}
		}

		ParticlePropertyData* getUpdateProperty(ParticleProperty prop)
		{
			ParticlePropertyDataMap::iterator it = this->updateProperties.find(prop);
			if (it != ParticlePropertyManager::getInstance()->updateProperties.end())
			{
				return it->second;
			}
			return nullptr;
		}

		typedef std::map<uint32, ParticlePropertyData*> ParticlePropertyDataMap;

		ParticlePropertyData* propertyData[ParticlePropertyMAX];
		ParticlePropertyDataMap updateProperties;
	};

	class ParticleBuffer
	{
	public:
		ParticleBuffer()
			: propertyMask(ParticlePropertyMask())
			, currentSize(0)
			, numElements(0)
			, bufferData(nullptr)
		{ }

		ParticleBuffer(const ParticlePropertyMask& mask, size_t num)
			: propertyMask(mask)
			, currentSize(0)
			, numElements(num)
			, bufferData(nullptr)
		{
			for (size_t i = 0; i < ParticlePropertyMAX; i++)
			{
				ParticleProperty prop = static_cast<ParticleProperty>(i);
				if (mask.test(prop))
				{
					this->offsets[prop] = this->currentSize;
					this->currentSize += ParticlePropertyManager::getInstance()->propertyData[prop]->getSize();
					
					ParticlePropertyData* update_prop = ParticlePropertyManager::getInstance()->getUpdateProperty(prop);
					if (update_prop)
					{
						this->updateProperties.push_back(update_prop);
					}
				}
			}
		}

		const ParticlePropertyMask& getMask() const { return this->propertyMask; }

		void allocate()
		{
			this->bufferData = new char[this->currentSize * this->numElements];
			memset(this->bufferData, 0, this->currentSize * this->numElements);
		}

		void resize(size_t newSize)
		{
			this->numElements = std::max<size_t>(newSize, this->numElements);
			this->clear();
			this->allocate();
		}

		void resizeCopy(size_t new_size)
		{
			char* newData = new char[this->currentSize * new_size];
			memset(newData, 0, this->currentSize * new_size);

			size_t to_copy = std::min<size_t>(this->numElements, new_size);
			memcpy(newData, this->bufferData, to_copy);
			delete[] this->bufferData;

			this->numElements = new_size;
			this->bufferData = newData;
		}

		void clear()
		{
			if (this->bufferData)
			{
				delete[] this->bufferData;
				this->bufferData = nullptr;
			}
		}

		void operator=(const ParticleBuffer& rhs)
		{
			this->offsets = rhs.offsets;
			this->currentSize = rhs.currentSize;
			this->numElements = rhs.numElements;

			if (rhs.bufferData)
			{
				this->bufferData = new char[this->currentSize * this->numElements];
				memcpy(this->bufferData, rhs.bufferData, this->currentSize * this->numElements);
			}
			this->propertyMask = rhs.propertyMask;
		}

		~ParticleBuffer()
		{
			this->clear();
		}

		template <class T>
		T* get(ParticleProperty prop, size_t index)
		{
			assert(index < this->numElements);
			std::unordered_map<ParticleProperty, size_t>::iterator it = this->offsets.find(prop);
			if (it == this->offsets.end())
			{
				log::error("Property ", kParticlePropertyStr[prop], " does not exist");
				return nullptr;
			}

			size_t data_offset = (this->currentSize * index) + it->second;
			return reinterpret_cast<T*>(PTR_ADD(this->bufferData, data_offset));
		}

		bool hasProperty(ParticleProperty prop) const { return this->propertyMask.test(prop); }

		void copy(ParticleProperty prop, size_t index, void* data, size_t sz)
		{
			assert(index < this->numElements);
			std::unordered_map<ParticleProperty, size_t>::iterator it = this->offsets.find(prop);
			if (it == this->offsets.end())
			{
				log::error("Property ", kParticlePropertyStr[prop], " does not exist");
				return;
			}

			assert(ParticlePropertyManager::getInstance()->propertyData[prop]->getSize() == sz);
			size_t data_offset = (this->currentSize * index) + it->second;
			void* dst = PTR_ADD(this->bufferData, data_offset);
			memcpy(dst, data, sz);
		}

		void swap(size_t dst_index, size_t src_index)
		{
			size_t dst = this->currentSize * (dst_index);
			size_t src = this->currentSize * (src_index);
			memcpy(this->bufferData + dst, this->bufferData + src, this->currentSize);
		}

		void print(std::stringstream& oss);
		void update(float32 dt, float32 pct, size_t index);
		

	private:

		ParticleBuffer(const ParticleBuffer& rhs) = delete;

		ParticlePropertyMask propertyMask;
		std::unordered_map<ParticleProperty, size_t> offsets;
		size_t currentSize;

		size_t numElements;
		char* bufferData;

		std::vector<ParticlePropertyData*> updateProperties;
		
	};

	template <class T, class V>
	void ParticlePropertyDataUpdater<T, V>::update(int32 index, ParticleBuffer* buffer, float32 dt, float32 pct)
	{
		// TODO: SLOW!
		assert(buffer->hasProperty(this->src));
		assert(buffer->hasProperty(this->dst));
		
		T* src_data = buffer->get<T>(this->src, index);
		V* dst_data = buffer->get<V>(this->dst, index);
		assert(src_data);
		assert(dst_data);
		assert(this->updateFunction);

		this->updateFunction(dt, pct, src_data, dst_data);
	}

	

}