#pragma once

#include "ClassDef.h"
#include "math/GLM.h"

#include <iostream>

namespace cs
{
	enum AnimationScaleType
	{
		AnimationScaleTypeNone = -1,
		AnimationScaleTypeCenter,
		AnimationScaleTypeLeft,
		//...
		AnimationScaleTypeMAX
	};

	CLASS_DEFINITION_REFLECT(Transform)
	public:

		const static vec3 kDefaultPosition;
		const static vec3 kDefaultScale;
		const static quat kDefaultRotation;

		Transform();
		Transform(const Transform& rhs)
			: position(rhs.position)
			, rotation(rhs.rotation)
			, scale(rhs.scale)
			, curMatrix(rhs.curMatrix)
		{ }

		Transform(const vec3& pos);
		Transform(const quat& rot);

		void operator=(const Transform& rhs)
		{
			this->position = rhs.position;
			this->rotation = rhs.rotation;
			this->scale = rhs.scale;
			this->curMatrix = rhs.curMatrix;
		}

		bool operator==(const Transform& rhs) const
		{
			return
				this->position == rhs.position &&
				this->rotation == rhs.rotation &&
				this->scale == rhs.scale;
		}

		void scaleTranslation(float32 s);

		const vec3& getPosition() const { return this->position; }
		const quat& getRotation() const { return this->rotation; }
		const vec3& getScale() const { return this->scale; }

		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);

		void setPosition(const vec3& pos);
		void setRotation(const quat& rot);
		void setScale(const vec3& sca);

		const mat4& getCurrentMatrix() const;

		void refresh();
		vec3 translateScale(const vec3& vec) const;
		vec3 translate(const vec3& vec) const;
		vec2 translate(const vec2& vec) const;
		
		quat rotate(const quat& rot) const;
		vec3 rotate(const vec3& vec) const;
		vec2 rotate(const vec2& vec) const;
		vec3 rotateInverse(const vec3& vec);

		Transform concatenate(const Transform& parent_transform) const;
		mat4 genMatrix() const;

		vec3 position;
		vec3 scale;
		quat rotation;

		mat4 curMatrix;
	};

}
