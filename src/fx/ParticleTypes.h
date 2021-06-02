#pragma once

#include "ClassDef.h"

#include "global/Event.h"

#define ADD_PARTICLE_VEC2_OPTIONS() \
	ADD_COMBO_META_LABEL(ParticleVec2ValueConstant, "Constant"); \
	ADD_COMBO_META_LABEL(ParticleVec2ValueRandomCubed, "Random Cubed"); \
	ADD_COMBO_META_LABEL(ParticleVec2ValueRandom, "Random Range"); 

#define ADD_PARTICLE_VEC3_OPTIONS() \
	ADD_COMBO_META_LABEL(ParticleVec3ValueConstant, "Constant"); \
	ADD_COMBO_META_LABEL(ParticleVec3ValueRandom, "Random Range"); \
	ADD_COMBO_META_LABEL(ParticleVec3ValueRandomDirection, "Random Direction");

#define ADD_PARTICLE_FLOAT_OPTIONS() \
	ADD_COMBO_META_LABEL(ParticleFloatValueConstant, "Constant"); \
	ADD_COMBO_META_LABEL(ParticleFloatValueRandom, "Random Range");

#define ADD_PARTICLE_COLOR_OPTIONS() \
	ADD_COMBO_META_LABEL(ParticleColorValueConstant, "Constant"); \
	ADD_COMBO_META_LABEL(ParticleColorValueRandom, "Random Range"); \
	ADD_COMBO_META_LABEL(ParticleColorValueRandomFixed, "Random Fixed"); \
	ADD_COMBO_META_LABEL(ParticleColorValueRandomOffset, "Random Offset"); \
	

namespace cs
{
	CLASS_DEFINITION_REFLECT(ParticleBaseValue)
	public:

		virtual bool isConstant() const { return true; }
	};

	CLASS_DEFINITION_DERIVED_REFLECT(ParticleFloatValue, ParticleBaseValue)
	public:
		ParticleFloatValue() { }
		
		virtual float32 getValue() const { return 0.0f; }
		virtual float32 getMaxValue() const { return 0.0f; }

		Event onChanged;

		void onValueChanged()
		{
			this->onChanged.invoke();
		}

	};

	CLASS_DEFINITION_DERIVED_REFLECT(ParticleFloatValueConstant, ParticleFloatValue)
	public:

		ParticleFloatValueConstant()
			: ParticleFloatValue()
			, value(1.0f)
		{ }
		ParticleFloatValueConstant(const float32& val)
			: ParticleFloatValue()
			, value(val)
		{ }

		virtual float32 getValue() const { return this->value; }
		virtual float32 getMaxValue() const { return this->value; }
	
	private:

		float32 value;

	};

	CLASS_DEFINITION_DERIVED_REFLECT(ParticleFloatValueRandom, ParticleFloatValue)
	public:
		ParticleFloatValueRandom()
			: ParticleFloatValue()
			, range(vec2(0.0f, 1.0f))
		{ }

		ParticleFloatValueRandom(const vec2& r)
			: ParticleFloatValue()
			, range(r)
		{ }

		virtual float32 getValue() const { return randomRange<float32>(this->range.x, this->range.y); }
		virtual float32 getMaxValue() const { return this->range.y; }

		virtual bool isConstant() const { return false; }

	private:

		vec2 range;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(ParticleVec2Value, ParticleBaseValue)
	public:
		ParticleVec2Value() { }

		virtual vec2 getValue() const { return kZero2; }
		virtual vec2 getMaxValue() const { return kZero2; }
		virtual vec2 getMinValue() const { return kZero2; }

		Event onChanged;

		void onValueChanged()
		{
			this->onChanged.invoke();
		}
	};

	CLASS_DEFINITION_DERIVED_REFLECT(ParticleVec2ValueConstant, ParticleVec2Value)
	public:
		ParticleVec2ValueConstant()
			: ParticleVec2Value()
			, value(kZero2)
		{ }
		ParticleVec2ValueConstant(const vec2& val)
			: ParticleVec2Value()
			, value(val)
		{ }

		virtual vec2 getValue() const { return this->value; }
		virtual vec2 getMaxValue() const { return this->value; }
		virtual vec2 getMinValue() const { return this->value; }

	private:

		vec2 value;

	};

	CLASS_DEFINITION_DERIVED_REFLECT(ParticleVec2ValueRandomCubed, ParticleVec2Value)
	public:
		ParticleVec2ValueRandomCubed()
			: ParticleVec2Value()
			, range(kZero2)
		{ }
		ParticleVec2ValueRandomCubed(const vec2& val)
			: ParticleVec2Value()
			, range(val)
		{ }

		virtual vec2 getValue() const 
		{ 
			float32 val = randomRange<float32>(this->range.x, this->range.y);
			return vec2(val, val);
		}

		virtual vec2 getMaxValue() const { return vec2(range.x, range.x); }
		virtual vec2 getMinValue() const { return vec2(range.y, range.y); }

		virtual bool isConstant() const { return false; }

	private:

		vec2 range;

	};

	CLASS_DEFINITION_DERIVED_REFLECT(ParticleVec2ValueRandom, ParticleVec2Value)
		public:
			ParticleVec2ValueRandom()
				: min_value(kZero2)
				, max_value(kZero2)
			{ }

			ParticleVec2ValueRandom(const vec2& minv, const vec2& maxv)
				: min_value(minv)
				, max_value(maxv)
			{ }

			virtual vec2 getValue() const
			{
				return vec2(
					randomRange<float32>(this->min_value.x, this->max_value.x),
					randomRange<float32>(this->min_value.y, this->max_value.y));
			}

			virtual vec2 getMaxValue() const { return this->max_value; }
			virtual vec2 getMinValue() const { return this->min_value; }

			virtual bool isConstant() const { return false; }

		private:

			vec2 min_value;
			vec2 max_value;
	};


	CLASS_DEFINITION_DERIVED_REFLECT(ParticleVec3Value, ParticleBaseValue)
	public:
		ParticleVec3Value() { }

		virtual vec3 getValue() const { return kZero3; }
		virtual vec3 getMaxValue() const { return kZero3; }
		virtual vec3 getMinValue() const { return kZero3; }

		Event onChanged;

		void onValueChanged()
		{
			this->onChanged.invoke();
		}
	};

	CLASS_DEFINITION_DERIVED_REFLECT(ParticleVec3ValueConstant, ParticleVec3Value)
	public:
		ParticleVec3ValueConstant()
			: ParticleVec3Value()
			, value(kZero3)
		{ }
		ParticleVec3ValueConstant(const vec3& val)
			: ParticleVec3Value()
			, value(val)
		{ }

		virtual vec3 getValue() const { return this->value; }
		virtual vec3 getMaxValue() const { return this->value; }
		virtual vec3 getMinValue() const { return this->value; }

	private:

		vec3 value;

	};

	CLASS_DEFINITION_DERIVED_REFLECT(ParticleVec3ValueRandom, ParticleVec3Value)
		public:
			ParticleVec3ValueRandom()
				: min_value(kZero3)
				, max_value(kZero3)
			{ }

			ParticleVec3ValueRandom(const vec3& minv, const vec3& maxv)
				: min_value(minv)
				, max_value(maxv)
			{ }

			virtual vec3 getValue() const 
			{ 
				return vec3(
					randomRange<float32>(this->min_value.x, this->max_value.x),
					randomRange<float32>(this->min_value.y, this->max_value.y),
					randomRange<float32>(this->min_value.z, this->max_value.z)
				);
			}

			virtual vec3 getMaxValue() const { return this->max_value; }
			virtual vec3 getMinValue() const { return this->min_value; }

			virtual bool isConstant() const { return false; }

		private:

			vec3 min_value;
			vec3 max_value;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(ParticleVec3ValueRandomDirection, ParticleVec3Value)
		public:
			ParticleVec3ValueRandomDirection()
				: min_value(kZero3)
				, max_value(kZero3)
				, magnitude(CREATE_CLASS(ParticleFloatValueConstant, 1.0f))
			{ }

			ParticleVec3ValueRandomDirection(const vec3& minv, const vec3& maxv, float32 mag)
				: min_value(minv)
				, max_value(maxv)
				, magnitude(CREATE_CLASS(ParticleFloatValueConstant, mag))
			{ }

			virtual vec3 getValue() const
			{
				vec3 val(
					randomRange<float32>(this->min_value.x, this->max_value.x),
					randomRange<float32>(this->min_value.y, this->max_value.y),
					randomRange<float32>(this->min_value.z, this->max_value.z)
					);

				val = glm::normalize(val);
				return val * this->magnitude->getValue();
			}

			virtual vec3 getMaxValue() const { return this->max_value; }
			virtual vec3 getMinValue() const { return this->min_value; }

			virtual bool isConstant() const { return false; }

		private:

			vec3 min_value;
			vec3 max_value;
			ParticleFloatValuePtr magnitude;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(ParticleColorValue, ParticleBaseValue)
	public:
		ParticleColorValue() { }

		virtual ColorB getValue() const { return ColorB::White; }
		
		Event onChanged;

		void onValueChanged()
		{
			this->onChanged.invoke();
		}
	};

	CLASS_DEFINITION_DERIVED_REFLECT(ParticleColorValueConstant, ParticleColorValue)
	public:
		ParticleColorValueConstant()
			: color(ColorB::White)
		{ }
		ParticleColorValueConstant(const ColorB& col)
			: color(col)
		{ }

		virtual ColorB getValue() const { return this->color; }

	protected:

		ColorB color;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(ParticleColorValueRandom, ParticleColorValue)
	public:
		ParticleColorValueRandom()
			: min_value(ColorB::Black)
			, max_value(ColorB::White)
		{ }
		ParticleColorValueRandom(const ColorB& minv, const ColorB& maxv)
			: min_value(minv)
			, max_value(maxv)
		{ }

		virtual ColorB getValue() const 
		{ 
			ColorB color;
			color.r = randomRange<uchar>(min_value.r, max_value.r);
			color.g = randomRange<uchar>(min_value.g, max_value.g);
			color.b = randomRange<uchar>(min_value.b, max_value.b);
			color.a = randomRange<uchar>(min_value.a, max_value.a);
			return color;
		}

		virtual bool isConstant() const { return false; }

	protected:

		ColorB min_value;
		ColorB max_value;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(ParticleColorValueRandomOffset, ParticleColorValue)
	public:
		ParticleColorValueRandomOffset()
			: value(ColorB::White)
			, mask(kOne4)
			, offset(CREATE_CLASS(ParticleFloatValueConstant, 0.0f))
		{ }
		ParticleColorValueRandomOffset(const ColorB& v)
			: value(v)
			, mask(kOne4)
			, offset(CREATE_CLASS(ParticleFloatValueConstant, 0.0f))
		{ }

		virtual ColorB getValue() const
		{
			ColorB color;
			float32 offset_value = this->offset->getValue();
			color.r = uchar(std::min<uint32>(uint32(this->value.r) + static_cast<uint32>(offset_value * this->mask.x), 255));
			color.g = uchar(std::min<uint32>(uint32(this->value.g) + static_cast<uint32>(offset_value * this->mask.y), 255));
			color.b = uchar(std::min<uint32>(uint32(this->value.b) + static_cast<uint32>(offset_value * this->mask.z), 255));
			color.a = uchar(std::min<uint32>(uint32(this->value.a) + static_cast<uint32>(offset_value * this->mask.w), 255));
			return color;
		}

		virtual bool isConstant() const { return false; }

	protected:

		ColorB value;
		vec4 mask;
		ParticleFloatValuePtr offset;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(ParticleColorValueRandomFixed, ParticleColorValue)
	public:
		ParticleColorValueRandomFixed()
			: min_value(ColorB::Black)
			, max_value(ColorB::White)
		{ 
			this->onValueChanged();
		}

		ParticleColorValueRandomFixed(const ColorB& minv, const ColorB& maxv)
			: min_value(minv)
			, max_value(maxv)
		{ 
			this->onValueChanged();
		}

		virtual ColorB getValue() const
		{
			return this->init_value;
		}

		virtual bool isConstant() const { return true; }

		virtual void onValueChanged()
		{
			this->init_value.r = randomRange<uchar>(min_value.r, max_value.r);
			this->init_value.g = randomRange<uchar>(min_value.g, max_value.g);
			this->init_value.b = randomRange<uchar>(min_value.b, max_value.b);
			this->init_value.a = randomRange<uchar>(min_value.a, max_value.a);
			BASECLASS::onValueChanged();
		}

	protected:

		ColorB init_value;
		ColorB min_value;
		ColorB max_value;
	};
}
