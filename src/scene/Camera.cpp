#include "PCH.h"

#include "scene/Camera.h"
#include "gfx/RenderInterface.h"

namespace cs
{

	Camera::Camera() :
		projection(ProjectionOrthographic),
		zNear(0.0f),
		zFar(100.0f),
		viewport(0, 0, 1, 1),
		fieldOfView(60.0f),
		aspectRatio(1.0f),
		orthographicDepth(10.0f)
	{
		this->updateView();
		this->updateProjection();
	}

	Camera::~Camera()
	{
		this->updateView();
		this->updateProjection();
	}


	void Camera::operator=(const Camera& rhs)
	{
		this->viewport = rhs.viewport;

		this->projection = rhs.projection;
		this->zNear = rhs.zNear;
		this->zFar = rhs.zFar;
		this->fieldOfView = rhs.fieldOfView;
		this->aspectRatio = rhs.aspectRatio;
		this->transform = rhs.transform;

		this->updateView();
		this->updateProjection();
	}

	void Camera::setViewport(const RectI& view)
	{ 
		this->viewport = view;

		if (this->projection == ProjectionOrthographic)
			this->updateProjection();
	}

	void Camera::bind()
	{
		RenderInterface* ri = RenderInterface::getInstance();
        ri->setZ(this->zNear, this->zFar);
        ri->setViewport(this->viewport);
    }

	void Camera::setOrthographic(float32 x0, float32 y0, float32 x1, float32 y1, float32 zNear, float32 zFar)
	{
		this->projection = ProjectionOrthographic;
		this->zNear = zNear;
		this->zFar = zFar;
		this->currentProjection = glm::ortho(x0, x1, y0, y1, zNear, zFar);
		this->updateView();
	}

	void Camera::setPerspective(float32 fov, float32 aspect, float32 zNear, float32 zFar)
	{
		this->projection = ProjectionPerspective;
		this->zNear = zNear;
		this->zFar = zFar;
		this->aspectRatio = aspect;
		this->fieldOfView = fov;

		this->currentProjection = glm::perspective(fov, aspect, zNear, zFar);
		this->updateView();

	}

	void Camera::updateProjection()
	{
		switch (this->projection)
		{
			case ProjectionOrthographic:
			{
				float32 x0 = (float32) this->viewport.pos.x;
				float32 y0 = (float32) this->viewport.pos.y;
				float32 x1 = x0 + (float32) this->viewport.size.w;
				float32 y1 = y0 + (float32) this->viewport.size.h;
				this->currentProjection = glm::ortho(x0, x1, y0, y1, this->zNear, this->zFar);
#if defined(CS_METAL)
                mat4 adjustProjection(
                    1.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 0.5f, 0.0f,
                    0.0f, 0.0f, 0.5f, 1.0f);
                this->currentProjection = adjustProjection * this->currentProjection;
#endif
			} break;
			
			case ProjectionPerspective:
			{
				this->currentProjection = glm::perspective(this->fieldOfView, this->aspectRatio, this->zNear, this->zFar);
			} break;
		}
	}

	void Camera::translate(const vec3& vec)
	{
		if (this->transform.getPosition() == vec)
			return;

		this->transform.setPosition(vec);
		this->updateView();
	}

	void Camera::translate(float32 x, float32 y, float32 z)
	{
		vec3 vec(x, y, z);
		this->translate(vec);
	}

	vec3 Camera::getTranslation() const
	{
		return this->transform.getPosition();
	}

	vec3 Camera::getCenter() const
	{
		switch (this->projection)
		{
			case ProjectionOrthographic:
			{
				vec3 off = this->getTranslation();
				return vec3(off.x - this->viewport.size.w / 2.0f, off.y - this->viewport.size.h / 2.0f, off.z);
			}
			case ProjectionPerspective:
			default:
				return this->getTranslation();
		}
	}

	void Camera::scale(const vec3& vec)
	{
		if (this->transform.getScale() == vec)
			return;

		this->transform.setScale(vec);
		this->updateView();
	}

	void Camera::scale(float32 x, float32 y, float32 z)
	{
		vec3 vec(x, y, z);
		this->scale(vec);
	}

	vec3 Camera::getScale() const
	{
		return this->transform.getScale();
	}

	void Camera::updateView()
	{
		this->currentView = this->transform.getCurrentMatrix();
		this->currentMatrix = this->currentProjection * this->currentView;
		this->currentMatrixInv = glm::inverse(this->currentMatrix);
	}

	void Camera::getRay(const PointI point, Ray& ray)
	{
		float32 x = (float32(point.x) - this->viewport.pos.x) / float32(this->viewport.size.w);
		float32 y = (float32(point.y) - this->viewport.pos.y) / float32(this->viewport.size.h);

		return this->getRay(vec2(x, y), ray);
	}

	bool Camera::getScreenPosition(const vec3& pos, vec2& screen_pos)
	{
		vec4 trans_pos(pos, 1.0f);
		trans_pos = this->currentMatrix * trans_pos;
		if (trans_pos.w != 0.0f)
			trans_pos.w = 1.0f / trans_pos.w;
		else
			return false;

		vec3 out = vec3(trans_pos.x, trans_pos.y, trans_pos.z) * trans_pos.w;

		// map to range [0 - 1]
		vec2 pt;
		screen_pos.x = (out.x * 0.5f) + 0.5f;
		screen_pos.y = (out.y * 0.5f) + 0.5f;

		screen_pos.x *= float32(this->viewport.size.w);
		screen_pos.y *= float32(this->viewport.size.h);

		return trans_pos.w > 0.0f;
	}

	void Camera::getRay(const vec2& screen_pct, Ray& ray)
	{
		// transform to normalized coordinates
		float32 x = (2.0f * screen_pct.x) - 1.0f;
		float32 y = (2.0f * screen_pct.y) - 1.0f;
		float32 z = 1.0f;

		vec3 ray_nds = vec3(x, y, z);
		vec4 ray_near(ray_nds.x, ray_nds.y, 0.0f, 1.0f);
		vec4 ray_far = ray_near;
		ray_far.z = 1.0f;

		struct local
		{
			static void unproject(vec3& out, const vec4& in, const mat4& mat)
			{
				vec4 ray_eye = mat * in;
				if (ray_eye.w != 0.0f)
					ray_eye.w = 1.0f / ray_eye.w;

				out = vec3(ray_eye.x, ray_eye.y, ray_eye.z) * ray_eye.w;
			}
		};

		vec3 near_pos, far_pos;
		local::unproject(near_pos, ray_near, this->currentMatrixInv);
		local::unproject(far_pos, ray_far, this->currentMatrixInv);
		vec3 dir = glm::normalize(far_pos - near_pos);

		ray = Ray(near_pos, dir);
	}

	Ray Camera::getRay(const vec2& pos)
	{
		Ray ray;
		float32 x = (float32(pos.x) - this->viewport.pos.x) / float32(this->viewport.size.w);
		float32 y = (float32(pos.y) - this->viewport.pos.y) / float32(this->viewport.size.h);
		this->getRay(vec2(x, y), ray);
		return ray;
	}

}
