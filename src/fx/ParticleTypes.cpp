#include "PCH.h"

#include "fx/ParticleTypes.h"

namespace cs
{
	BEGIN_META_CLASS(ParticleBaseValue)
	END_META();

	BEGIN_META_CLASS(ParticleFloatValue)
	END_META();

	BEGIN_META_CLASS(ParticleFloatValueConstant)
		ADD_MEMBER(value);
			SET_MEMBER_NO_SLIDER();
			SET_MEMBER_CALLBACK_POST(&ParticleFloatValue::onValueChanged);
	END_META();

	BEGIN_META_CLASS(ParticleFloatValueRandom)
		SET_IGNORE_DERIVED_MEMBERS();
		ADD_MEMBER(range);
		SET_MEMBER_CALLBACK_POST(&ParticleFloatValue::onValueChanged);
	END_META();

	BEGIN_META_CLASS(ParticleVec2Value)
		END_META();

	BEGIN_META_CLASS(ParticleVec2ValueConstant)
		ADD_MEMBER(value);
		SET_MEMBER_CALLBACK_POST(&ParticleVec2Value::onValueChanged);
	END_META();

	BEGIN_META_CLASS(ParticleVec2ValueRandomCubed)
		ADD_MEMBER(range);
		SET_MEMBER_CALLBACK_POST(&ParticleVec2Value::onValueChanged);
	END_META();

	BEGIN_META_CLASS(ParticleVec2ValueRandom)
		ADD_MEMBER(min_value);
			SET_MEMBER_CALLBACK_POST(&ParticleVec2Value::onValueChanged);
		ADD_MEMBER(max_value);
			SET_MEMBER_CALLBACK_POST(&ParticleVec2Value::onValueChanged);
	END_META();

	BEGIN_META_CLASS(ParticleVec3Value)
	END_META();

	BEGIN_META_CLASS(ParticleVec3ValueConstant)
		ADD_MEMBER(value);
		SET_MEMBER_CALLBACK_POST(&ParticleVec3Value::onValueChanged);
	END_META();

	BEGIN_META_CLASS(ParticleVec3ValueRandom)
		ADD_MEMBER(min_value);
			SET_MEMBER_CALLBACK_POST(&ParticleVec3Value::onValueChanged);
		ADD_MEMBER(max_value);
			SET_MEMBER_CALLBACK_POST(&ParticleVec3Value::onValueChanged);
	END_META();

	BEGIN_META_CLASS(ParticleVec3ValueRandomDirection)
		ADD_MEMBER(min_value);
			SET_MEMBER_CALLBACK_POST(&ParticleVec3Value::onValueChanged);
		ADD_MEMBER(max_value);
			SET_MEMBER_CALLBACK_POST(&ParticleVec3Value::onValueChanged);
		ADD_MEMBER_PTR(magnitude);
			ADD_PARTICLE_FLOAT_OPTIONS();
	END_META();

	BEGIN_META_CLASS(ParticleColorValue)
	END_META();

	BEGIN_META_CLASS(ParticleColorValueConstant)
		ADD_MEMBER(color);
			SET_MEMBER_CALLBACK_POST(&ParticleColorValue::onValueChanged);
	END_META();

	BEGIN_META_CLASS(ParticleColorValueRandom)
		ADD_MEMBER(min_value);
			SET_MEMBER_CALLBACK_POST(&ParticleColorValue::onValueChanged);
		ADD_MEMBER(max_value);
			SET_MEMBER_CALLBACK_POST(&ParticleColorValue::onValueChanged);
	END_META();

	BEGIN_META_CLASS(ParticleColorValueRandomFixed)
		ADD_MEMBER(min_value);
			SET_MEMBER_CALLBACK_POST(&ParticleColorValueRandomFixed::onValueChanged);
		ADD_MEMBER(max_value);
			SET_MEMBER_CALLBACK_POST(&ParticleColorValueRandomFixed::onValueChanged);
	END_META();

	BEGIN_META_CLASS(ParticleColorValueRandomOffset)
		ADD_MEMBER(value);
			SET_MEMBER_CALLBACK_POST(&ParticleColorValue::onValueChanged);
		ADD_MEMBER(mask);
		ADD_MEMBER_PTR(offset);
			ADD_PARTICLE_FLOAT_OPTIONS();
	END_META();
}