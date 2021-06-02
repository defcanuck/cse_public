#pragma once

#include "ClassDef.h"
#include "fx/Particle.h"
#include "fx/ParticleTypes.h"

#define ADD_PARTICLE_MODULES() \
	ADD_COMBO_META_LABEL(ParticleModuleSize, "Particle Size"); \
	ADD_COMBO_META_LABEL(ParticleModuleColor, "Particle Color"); \
	ADD_COMBO_META_LABEL(ParticleModuleSimulation, "Particle Simulate"); \
	ADD_COMBO_META_LABEL(ParticleModuleRotation, "Particle Rotation");

#define ADD_PARTICLE_MODULE_VALUE_COLOR() \
	ADD_COMBO_META_LABEL(ParticleModuleValueColor, "Constant Color"); \
	ADD_COMBO_META_LABEL(ParticleModuleValueColorLerp, "Interpolated Color");

#define ADD_PARTICLE_MODULE_VALUE_SIZE() \
	ADD_COMBO_META_LABEL(ParticleModuleValueSize, "Constant Size"); \
	ADD_COMBO_META_LABEL(ParticleModuleValueSizeLerp, "Interpolated Size");

#define ADD_PARTICLE_MODULE_VALUE_VELOCITY() \
	ADD_COMBO_META_LABEL(ParticleModuleValueVelocity, "Constant Velocity"); \

#define ADD_PARTICLE_MODULE_VALUE_ACCELERATION() \
	ADD_COMBO_META_LABEL(ParticleModuleValueAcceleration, "Constant Acceleration"); \

#define ADD_PARTICLE_MODULE_VALUE_ROTATION() \
	ADD_COMBO_META_LABEL(ParticleModuleValueRotation, "Constant Start Rotation"); \

#define ADD_PARTICLE_MODULE_VALUE_ROTATIONSPEED() \
	ADD_COMBO_META_LABEL(ParticleModuleValueRotationSpeed, "Constant Start Rotation Speed"); \


#define BEGIN_META_MODULE_VALUE_CONSTANT(name, options) \
	BEGIN_META_CLASS(name) \
		ADD_MEMBER_PTR(value); \
		options(); \
	END_META();

#define BEGIN_META_MODULE_VALUE_LERP(name, options) \
	BEGIN_META_CLASS(name) \
		ADD_MEMBER_PTR(start_value); \
			options(); \
		ADD_MEMBER_PTR(end_value); \
			options(); \
	END_META();

namespace cs
{
	
	CLASS_DEFINITION_REFLECT(ParticleModuleValue)
	public:

		virtual void setMask(ParticlePropertyMask& mask) { }
		virtual void populate(ParticleInitProps& initProps, ParticlePropertyMask ignoreMask) { }
	};

	template <class T, class V, class DefaultValue, ParticleProperty Flag>
	class ParticleModuleValueConstant : public ParticleModuleValue
	{
	public:
		ParticleModuleValueConstant()
			: ParticleModuleValue()
			, value(CREATE_CLASS(DefaultValue, T()))
		{ }

		ParticleModuleValueConstant(const T& val)
			: ParticleModuleValue()
			, value(CREATE_CLASS(DefaultValue, val))
		{ }

		virtual void setMask(ParticlePropertyMask& mask)
		{
			mask.set(static_cast<ParticleProperty>(MaskFlag));
		}

		virtual void populate(ParticleInitProps& initProps, ParticlePropertyMask ignoreMask)
		{
			addProperty(initProps.propertyMap, static_cast<ParticleProperty>(MaskFlag), this->value->getValue(), ignoreMask);
		}

		enum { MaskFlag = Flag };

		std::shared_ptr<V> value;
	};

	template <class T, class V, class DefaultValue, ParticleProperty Flag, ParticleProperty FlagRange>
	class ParticleModuleValueLerp : public ParticleModuleValue
	{
	public:
		ParticleModuleValueLerp()
			: ParticleModuleValue()
			, start_value(CREATE_CLASS(DefaultValue, T()))
			, end_value(CREATE_CLASS(DefaultValue, T()))
		{ }

		ParticleModuleValueLerp(const T& min_val, const T& max_val)
			: ParticleModuleValue()
			, start_value(CREATE_CLASS(DefaultValue, min_val))
			, end_value(CREATE_CLASS(DefaultValue, max_val))
		{ }

		virtual void setMask(ParticlePropertyMask& mask)
		{
			mask.set(static_cast<ParticleProperty>(MaskFlag));
			mask.set(static_cast<ParticleProperty>(MaskFlagRange));
		}

		virtual void populate(ParticleInitProps& initProps, ParticlePropertyMask ignoreMask)
		{
			RangeValue<T> range(this->start_value->getValue(), this->end_value->getValue());
			addProperty(initProps.propertyMap, static_cast<ParticleProperty>(MaskFlag), this->start_value->getValue(), ignoreMask);
			addProperty(initProps.propertyMap, static_cast<ParticleProperty>(MaskFlagRange), range, ignoreMask);
		}

		enum 
		{ 
			MaskFlag = Flag,
			MaskFlagRange = FlagRange
		};

		std::shared_ptr<V> start_value;
		std::shared_ptr<V> end_value;
	};

	template <class T>
	struct ParticleKeyframeValue
	{
		std::shared_ptr<T> value;
		float32 dt;
	};

	typedef ParticleKeyframeValue<ParticleColorValue> ParticleKeyFrameValueColorImpl;

	CLASS_DEFINITION_DERIVED_REFLECT(ParticleKeyFrameValueColor, ParticleKeyFrameValueColorImpl)
	public:

	};

	template <class T, class V>
	class ParticleModuleValueKeyFrame : public ParticleModuleValue
	{
		virtual void setMask(ParticlePropertyMask& mask)
		{
			// mask.set(static_cast<ParticleProperty>(MaskFlag));
			// mask.set(static_cast<ParticleProperty>(MaskFlagRange));
		}

		virtual void populate(ParticleInitProps& initProps, ParticlePropertyMask ignoreMask)
		{
			// RangeValue<T> range(this->start_value->getValue(), this->end_value->getValue());
			// addProperty(initProps.propertyMap, static_cast<ParticleProperty>(MaskFlag), this->start_value->getValue(), ignoreMask);
			// addProperty(initProps.propertyMap, static_cast<ParticleProperty>(MaskFlagRange), range, ignoreMask);
		}

		typedef std::vector<ParticleKeyframeValue<V>> KeyFrameValues;
		KeyFrameValues values;
	};

	typedef ParticleModuleValueConstant<vec2, ParticleVec2Value, ParticleVec2ValueConstant, ParticlePropertySize> ParticleModuleSizeImpl;
	CLASS_DEFINITION_DERIVED_REFLECT(ParticleModuleValueSize, ParticleModuleSizeImpl)
	public:
		ParticleModuleValueSize()
			: ParticleModuleSizeImpl()
		{ }

		ParticleModuleValueSize(const vec2& sz)
			: ParticleModuleSizeImpl(sz)
		{ }
	};

	typedef ParticleModuleValueConstant<vec3, ParticleVec3Value, ParticleVec3ValueConstant, ParticlePropertyVelocity> ParticleModuleVelocityImpl;
	CLASS_DEFINITION_DERIVED_REFLECT(ParticleModuleValueVelocity, ParticleModuleVelocityImpl)
	public:
		ParticleModuleValueVelocity()
			: ParticleModuleVelocityImpl()
		{ }

		ParticleModuleValueVelocity(const vec3& vel)
			: ParticleModuleVelocityImpl(vel)
		{ }
	}; 
	
	typedef ParticleModuleValueConstant<vec3, ParticleVec3Value, ParticleVec3ValueConstant, ParticlePropertyAcceleration> ParticleModuleAccelerationImpl;
	CLASS_DEFINITION_DERIVED_REFLECT(ParticleModuleValueAcceleration, ParticleModuleAccelerationImpl)
	public:
		ParticleModuleValueAcceleration()
			: ParticleModuleAccelerationImpl()
		{ }

		ParticleModuleValueAcceleration(const vec3& accel)
			: ParticleModuleAccelerationImpl(accel)
		{ }
	};

	typedef ParticleModuleValueConstant<ColorB, ParticleColorValue, ParticleColorValueConstant, ParticlePropertyColor> ParticleModuleColorImpl;
	CLASS_DEFINITION_DERIVED_REFLECT(ParticleModuleValueColor, ParticleModuleColorImpl)
	public:
		ParticleModuleValueColor()
			: ParticleModuleColorImpl()
		{ }

		ParticleModuleValueColor(const ColorB& color)
			: ParticleModuleColorImpl(color)
		{ }
	};

	typedef ParticleModuleValueConstant<float32, ParticleFloatValue, ParticleFloatValueConstant, ParticlePropertyAngle> ParticleModuleRotationImpl;
	CLASS_DEFINITION_DERIVED_REFLECT(ParticleModuleValueRotation, ParticleModuleRotationImpl)
	public:
		ParticleModuleValueRotation()
			: ParticleModuleRotationImpl()
		{ }

		ParticleModuleValueRotation(const float32 &rot)
			: ParticleModuleRotationImpl(rot)
		{ }
	};

	typedef ParticleModuleValueConstant<float32, ParticleFloatValue, ParticleFloatValueConstant, ParticlePropertyAngleSpeed> ParticleModuleRotationSpeedImpl;
	CLASS_DEFINITION_DERIVED_REFLECT(ParticleModuleValueRotationSpeed, ParticleModuleRotationSpeedImpl)
	public:
		ParticleModuleValueRotationSpeed()
			: ParticleModuleRotationSpeedImpl()
		{ }

		ParticleModuleValueRotationSpeed(const float32 &rot)
			: ParticleModuleRotationSpeedImpl(rot)
		{ }
	};

	typedef ParticleModuleValueLerp<vec2, ParticleVec2Value, ParticleVec2ValueConstant, ParticlePropertySize, ParticlePropertySizeRange> ParticleModuleSizeRangeImpl;
	CLASS_DEFINITION_DERIVED_REFLECT(ParticleModuleValueSizeLerp, ParticleModuleSizeRangeImpl)
	public:
		ParticleModuleValueSizeLerp()
			: ParticleModuleSizeRangeImpl()
		{ }

		ParticleModuleValueSizeLerp(const vec2& start_val, const vec2& end_val)
			: ParticleModuleSizeRangeImpl(start_val, end_val)
		{ }
	};

	typedef ParticleModuleValueLerp<ColorB, ParticleColorValue, ParticleColorValueConstant, ParticlePropertyColor, ParticlePropertyColorRange> ParticleModuleColorRangeImpl;
	CLASS_DEFINITION_DERIVED_REFLECT(ParticleModuleValueColorLerp, ParticleModuleColorRangeImpl)
	public:
		ParticleModuleValueColorLerp()
			: ParticleModuleColorRangeImpl()
		{ }

		ParticleModuleValueColorLerp(const ColorB& start_val, const ColorB& end_val)
			: ParticleModuleColorRangeImpl(start_val, end_val)
		{ }
	};

	CLASS_DEFINITION_REFLECT(ParticleModule)
	public:

		virtual void setMask(ParticlePropertyMask& mask) { }
		virtual void populate(ParticleInitProps& initProps, ParticlePropertyMask ignoreMask) { }
	};

	typedef std::vector<ParticleModulePtr> ParticleModuleList;

	CLASS_DEFINITION_DERIVED_REFLECT(ParticleModuleSize, ParticleModule)
	public:

		ParticleModuleSize()
			: ParticleModule()
			, size(CREATE_CLASS(ParticleModuleValueSize))
		{ }

		ParticleModuleSize(const vec2& sz)
			: ParticleModule()
			, size(CREATE_CLASS(ParticleModuleValueSize, sz))
		{ }

		virtual void setMask(ParticlePropertyMask& mask)
		{
			assert(this->size);
			this->size->setMask(mask);
		}
		virtual void populate(ParticleInitProps& initProps, ParticlePropertyMask ignoreMask)
		{
			assert(this->size);
			this->size->populate(initProps, ignoreMask);
		}

	private:

		ParticleModuleValuePtr size;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(ParticleModuleColor, ParticleModule)
	public:
		ParticleModuleColor()
			: ParticleModule()
			, color(CREATE_CLASS(ParticleModuleValueColor, ColorB::White))
		{ }

		ParticleModuleColor(const ColorB& col)
			: ParticleModule()
			, color(CREATE_CLASS(ParticleModuleValueColor, col))
		{ }

		virtual void setMask(ParticlePropertyMask& mask) 
		{ 
			assert(this->color);
			this->color->setMask(mask);
		}
		virtual void populate(ParticleInitProps& initProps, ParticlePropertyMask ignoreMask)
		{ 
			assert(this->color);
			this->color->populate(initProps, ignoreMask);
		}

	private:

		ParticleModuleValuePtr color;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(ParticleModuleSimulation, ParticleModule)
	public:
		ParticleModuleSimulation()
			: ParticleModule()
			, velocity(CREATE_CLASS(ParticleModuleValueVelocity, kZero3))
			, acceleration(CREATE_CLASS(ParticleModuleValueAcceleration, kZero3))
		{ }

		ParticleModuleSimulation(const vec3& vecl, const vec3& accel)
			: ParticleModule()
			, velocity(CREATE_CLASS(ParticleModuleValueVelocity, vecl))
			, acceleration(CREATE_CLASS(ParticleModuleValueAcceleration, accel))
		{ }

		virtual void setMask(ParticlePropertyMask& mask)
		{
			assert(this->velocity);
			assert(this->acceleration);
			this->velocity->setMask(mask);
			this->acceleration->setMask(mask);
		}
		virtual void populate(ParticleInitProps& initProps, ParticlePropertyMask ignoreMask)
		{
			assert(this->velocity);
			assert(this->acceleration);
			this->velocity->populate(initProps, ignoreMask);
			this->acceleration->populate(initProps, ignoreMask);
		}

	private:

		ParticleModuleValuePtr velocity;
		ParticleModuleValuePtr acceleration;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(ParticleModuleRotation, ParticleModule)
	public:

		ParticleModuleRotation()
			: ParticleModule()
			, start(CREATE_CLASS(ParticleModuleValueRotation, 0.0f))
			, speed(CREATE_CLASS(ParticleModuleValueRotationSpeed, 0.0f))
		{ }

		virtual void setMask(ParticlePropertyMask& mask)
		{
			assert(this->start);
			assert(this->speed);
			this->start->setMask(mask);
			this->speed->setMask(mask);
		}
		virtual void populate(ParticleInitProps& initProps, ParticlePropertyMask ignoreMask)
		{
			assert(this->start);
			assert(this->speed);
			this->start->populate(initProps, ignoreMask);
			this->speed->populate(initProps, ignoreMask);
		}

		ParticleModuleValuePtr start;
		ParticleModuleValuePtr speed;
	};
}