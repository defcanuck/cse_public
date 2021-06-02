#pragma once

#include "ClassDef.h"

#include "gfx/Renderable.h"

namespace cs
{
	CLASS_DEFINITION_DERIVED_REFLECT(TrailRenderable, Renderable)
	
		const static float32 kDefaultSplineWidth;
		const static float32 kDefaultSplineSmooth;
		const static float32 kDefaultSplineControl;
		const static vec3 kDefaultWidthDir;
		const static float32 kDefaultWidthDirMag;
		const static size_t kDefaultSplineSize;
		const static ColorB kDefaultSplineColor;

	public:

		TrailRenderable()
			: Renderable()
			, splineColor(kDefaultSplineColor)
			, curSize(0)
			, maxSize(kDefaultSplineSize)
			, width(kDefaultSplineWidth)
			, smooth(kDefaultSplineSmooth)
			, control(kDefaultSplineControl)
			, dirWidth(kDefaultWidthDir)
			, dirWidthMag(kDefaultWidthDirMag)
			, depthWidth(0.0f)
			, depthFade(false)
			, restrictDirection(true)
			, age(0.0f)
			, pointMaxAge(-1.0f)
		{ 
			this->initGeometry();
		}

		TrailRenderable(int32 maxNodes)
			: Renderable()
			, splineColor(kDefaultSplineColor)
			, curSize(0)
			, maxSize(maxNodes)
			, width(kDefaultSplineWidth)
			, smooth(kDefaultSplineSmooth)
			, control(kDefaultSplineControl)
			, dirWidth(kDefaultWidthDir)
			, dirWidthMag(kDefaultWidthDirMag)
			, depthWidth(0.0f)
			, depthFade(false)
			, restrictDirection(true)
			, age(0.0f)
			, pointMaxAge(-1.0f)
		{
			this->initGeometry();
		}

		size_t updateVertices(uchar* data, size_t bufferSize, VertexDeclaration& decl);
		size_t updateIndices(uchar* data, size_t bufferSize);
		void setDrawParams(int32 index, std::vector<DrawCallPtr>& dcs);
		
		size_t getVertexBufferSize();
		size_t getIndexBufferSize();

		void update(const vec3& position, const vec3& direction);
		void push(const vec3& position, const vec3& direction);
		void top(const vec3& position, const vec3& direction);
		void adjust(const vec3& offset);

		void clear();

		void setWidth(float32 w) { this->width = w; }
		void setDirectionWidth(const vec3& vec, float32 w);

		void setSmooth(float32 s) { this->smooth = s; }
		void setControl(float32 c) { this->control = c; }
		void setColor(const ColorB& col) { this->splineColor = col; }
		void setDepthWidth(float32 dw) { this->depthWidth = dw; }
		void setDepthFade(bool df) { this->depthFade = df; }
		void setPointMaxAge(float32 a) { this->pointMaxAge = a; }
		void setRestrictDirection(bool rd) { this->restrictDirection = rd; }

		virtual bool isValid() const { return this->curSize > 0; }
		virtual void process(float32 dt);

		DrawOptions options;

	private:

		void initGeometry();

		std::pair<vec3, vec3> getPositionInternal(const vec3& position, const vec3& direction);
		void pushInternal(const vec3& position, const vec3& direction, const ColorB& color = ColorB::White);
		float32 getWidthInternal(const vec3& direction);

		struct TrailPointParams
		{
			TrailPointParams()
				: color(ColorB::White)
				, age(0.0f)
			{ }

			vec3 center;
			std::pair<vec3, vec3> positions;
			vec3 direction;
			vec3 perp;
			ColorB color;
			float32 age;
		};

		ColorB splineColor;
		size_t maxSize;
		size_t curSize;
		std::deque<TrailPointParams> params;
		
		DynamicGeometryPtr splineGeom;

		float32 width;
		float32 depthWidth;
		float32 pointMaxAge;
		bool depthFade;
		bool restrictDirection;
		
		vec3 dirWidth;
		float32 dirWidthMag;

		float32 smooth;
		float32 control;
		float32 age;

	};
}