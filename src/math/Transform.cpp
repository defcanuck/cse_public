#include "PCH.h"

#include "math/Transform.h"

namespace cs
{

	const vec3 Transform::kDefaultPosition = vec3(0.0f, 0.0f, 0.0f);
	const vec3 Transform::kDefaultScale = vec3(1.0f, 1.0f, 1.0f);
	const quat Transform::kDefaultRotation = quat(vec3(0.0f, 0.0f, 0.0f));

	BEGIN_META_CLASS(Transform)

		ADD_MEMBER(position);
	ADD_MEMBER(scale);
	ADD_MEMBER(rotation);

	END_META()

	Transform::Transform()
		: position(kDefaultPosition)
		, scale(kDefaultScale)
		, rotation(kDefaultRotation)
	{
		this->refresh();
	}

	Transform::Transform(const vec3& pos)
		: position(pos)
		, scale(kDefaultScale)
		, rotation(kDefaultRotation)
	{ 
		this->refresh();
	}

	Transform::Transform(const quat& rot)
		: position(kDefaultPosition)
		, scale(kDefaultScale)
		, rotation(rot)
	{ 
		this->refresh();
	}

	void Transform::onPostLoad(const LoadFlagMask& flags)
	{
		this->refresh();
	}

	void Transform::setPosition(const vec3& pos)
	{
		if (pos != this->position)
		{
			this->position = pos;
			this->refresh();
		}
	}

	void Transform::setRotation(const quat& rot)
	{
		if (rot != this->rotation)
		{
			this->rotation = rot;
			this->refresh();
		}
	}

	void Transform::setScale(const vec3& sca)
	{
		if (sca != this->scale)
		{
			this->scale = sca;
			this->refresh();
		}
	}

	const mat4& Transform::getCurrentMatrix() const
	{
		return this->curMatrix;
	}

	void Transform::refresh()
	{
		this->curMatrix = this->genMatrix();
	}

	void Transform::scaleTranslation(float32 s)
	{
		this->position = this->position * s;
		this->refresh();
	}

	vec3 Transform::translateScale(const vec3& vec) const
	{
		vec4 transVec(vec, 0.0f);
		vec4 result = this->curMatrix * transVec;
		return vec3(result.x, result.y, result.z);
	}

	vec3 Transform::translate(const vec3& vec) const
	{
		vec4 transVec(vec, 1.0f);
		vec4 result = this->curMatrix * transVec;
		return vec3(result.x, result.y, result.z);
	}

	vec2 Transform::translate(const vec2& vec) const
	{
		vec4 transVec(vec, 0.0f, 1.0f);
		vec4 result = this->curMatrix * transVec;
		return vec2(result.x, result.y);
	}

	quat Transform::rotate(const quat& rot) const
	{
		return this->rotation * rot;
	}

	vec3 Transform::rotate(const vec3& vec) const
	{
		mat4 rotate_mat = mat4_cast(this->rotation);
		vec4 result = rotate_mat * vec4(vec, 0.0f);
		return vec3(result.x, result.y, result.z);
	}

	vec2 Transform::rotate(const vec2& vec) const
	{
		mat4 rotate_mat = mat4_cast(this->rotation);
		vec4 result = rotate_mat * vec4(vec, 0.0f, 0.0f);
		return vec2(result.x, result.y);
	}
	
	vec3 Transform::rotateInverse(const vec3& vec)
	{
		mat4 rotate_mat = mat4_cast(this->rotation);
		vec4 result = glm::inverse(rotate_mat) * vec4(vec, 0.0f);
		return vec3(result.x, result.y, result.z);
	}

	Transform Transform::concatenate(const Transform& parent_transform) const
	{
		//this->curMatrix * parent_transform;
		Transform concat;
		concat.curMatrix = parent_transform.curMatrix * this->curMatrix;
		concat.rotation = parent_transform.rotation * this->rotation;
		concat.position = parent_transform.position + this->position;
		concat.scale = parent_transform.scale * this->scale;
		return concat;
	}

	mat4 Transform::genMatrix() const
	{
		mat4 scale_mat = glm::scale(scale);
		mat4 rotate_mat = mat4_cast(rotation);
		mat4 trans_mat = glm::translate(position);

		return trans_mat * rotate_mat * scale_mat;
	}
}