#pragma once

#include "ClassDef.h"

#include "gfx/Renderable.h"

namespace cs
{

	enum SplineType
	{
		SplineTypeHermite,
		SplineTypeBezier,
		SplineTypeMAX
	};


	CLASS_DEFINITION_DERIVED_REFLECT(SplineRenderable, Renderable)
	
		const static float32 kDefaultSplineWidth;
		const static float32 kDefaultSplineSmooth;
		const static float32 kDefaultSplineControl;
		const static size_t kDefaultSplineSize;
		const static ColorB kDefaultSplineColor;

		const static vec3 kDefaultP0;
		const static vec3 kDefaultP1;
		const static vec3 kDefaultP2;
		const static vec3 kDefaultP3;

	public:

		SplineRenderable()
			: Renderable()
			, type(SplineTypeHermite)
			, p0(kDefaultP0)
			, p1(kDefaultP1)
			, p2(kDefaultP2)
			, p3(kDefaultP3)
			, splineColor(kDefaultSplineColor)
			, maxSize(kDefaultSplineSize)
			, width(kDefaultSplineWidth)
			, smooth(kDefaultSplineSmooth)
			, control(kDefaultSplineControl)
			, texture(CREATE_CLASS(TextureHandle, RenderInterface::kWhiteTexture))
		{ 
			this->initGeometry();
			this->updateVolume();
		}

		SplineRenderable(int32 maxNodes)
			: Renderable()
			, type(SplineTypeHermite)
			, p0(kDefaultP0)
			, p1(kDefaultP1)
			, p2(kDefaultP2)
			, p3(kDefaultP3)
			, splineColor(kDefaultSplineColor)
			, maxSize(maxNodes)
			, width(kDefaultSplineWidth)
			, smooth(kDefaultSplineSmooth)
			, control(kDefaultSplineControl)
			, texture(CREATE_CLASS(TextureHandle, RenderInterface::kWhiteTexture))
		{
			this->initGeometry();
			this->updateVolume();
		}

		size_t updateVertices(uchar* data, size_t bufferSize, VertexDeclaration& decl);
		size_t updateIndices(uchar* data, size_t bufferSize);
		void setDrawParams(int32 index, std::vector<DrawCallPtr>& dcs);
		void setControlPoint(int32 idx, vec3 pos);

		size_t getVertexBufferSize();
		size_t getIndexBufferSize();

		void setWidth(float32 w) { this->width = w; }
		void setSmooth(float32 s) { this->smooth = s; }
		void setControl(float32 c) { this->control = c; }
		void setColor(const ColorB& col) { this->splineColor = col; }

		virtual void getSelectableVolume(SelectableVolumeList& selectable_volumes);

		void setTexture(const std::string& name);
		void setTexture(const TextureHandlePtr& tex);
		TextureHandlePtr& getTextureHandle() { return this->texture; }

		DrawOptions options;

	private:

		void initGeometry();
		void updateVolume();

		SplineType type;
		vec3 p0;
		vec3 p1;
		vec3 p2;
		vec3 p3;

		ColorB splineColor;
		size_t maxSize;
		
		DynamicGeometryPtr splineGeom;
		VolumePtr volume;

		float32 width;
		float32 smooth;
		float32 control;

		TextureHandlePtr texture;

	};
}