#pragma once

#include "ClassDef.h"
#include "gfx/Types.h"
#include "math/GLM.h"
#include "math/Transform.h"
#include "math/Ray.h"
#include "math/Rect.h"

namespace cs
{
	CLASS_DEFINITION(Camera)

	public:
		Camera();
		~Camera();

		void operator=(const Camera& rhs);

		void setViewport(const RectI& view);
		void setOrthographic(float32 x0, float32 y0, float32 x1, float32 y1, float32 zNear, float32 zFar);
		void setPerspective(float32 fov, float32 aspect, float32 zNear, float32 zFar);
		void setView(const mat4& view);

		void translate(const vec3& vec);
		void translate(float32 x, float32 y, float32 z);
		vec3 getTranslation() const;
		vec3 getCenter() const;

		void scale(const vec3& vec);
		void scale(float32 x, float32 y, float32 z);
		vec3 getScale() const;

		float32 getNear() const { return this->zNear; }
		float32 getFar() const { return this->zFar; }
		float32 getOrthographicDepth() const { return this->orthographicDepth; }

		ProjectionType getProjectionType() const { return this->projection; }
		mat4 getCurrentMatrix() const { return currentMatrix; }
		mat4 getCurrentProjection() const { return currentProjection; }
		mat4 getCurrentView() const { return currentView; }

		void getRay(const vec2& screen_pct, Ray& ray);
		void getRay(const PointI point, Ray& ray);
		Ray getRay(const vec2& pos);

		bool getScreenPosition(const vec3& pos, vec2& screen_pos);

		void setNear(float32 fnear) 
		{
			this->zNear = fnear; 
			this->updateProjection();
		}

		void setFar(float32 ffar) 
		{ 
			this->zFar = ffar; 
			this->updateProjection();
		}

		void bind();

		const RectI& getViewport() const { return this->viewport; }
		const Transform& getTransform() const { return this->transform; }

	private:

		void updateView();
		void updateProjection();

		RectI viewport;

		ProjectionType projection;
		float32 zNear;
		float32 zFar;
		float32 fieldOfView;
		float32 aspectRatio;
		float32 orthographicDepth;

		mat4 currentProjection;
		mat4 currentView;
		mat4 currentMatrix;
		mat4 currentMatrixInv;

		Transform transform;

	};


}


