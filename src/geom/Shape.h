#pragma once

#include "ClassDef.h"

#include "math/Rect.h"
#include "gfx/Types.h"
#include "geom/Volume.h"
#include "global/Event.h"

namespace cs
{
	CLASS_DEFINITION_REFLECT(Shape)
	public:
		Shape() { }

		virtual size_t generatePositions(std::vector<vec3>& pos, float32 z = 0.0f) { return 0; }
		virtual size_t generateUV(const RectF& uvRect, std::vector<vec2>& uvs) { return 0; }
		virtual size_t generateIndices(std::vector<uint16>& indices) { return 0; }
		virtual size_t generateWireframe(std::vector<uint16>& indices) { return 0; }
		virtual size_t generateColors(std::vector<ColorB>& colors, ColorB& color) { return 0; }
		virtual VolumePtr createVolume() { return nullptr; }

		void onGeomChanged() { this->onChanged.invoke(); }
		Event onChanged;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(QuadShape, Shape)

		static const RectF kDefaultRect;

	public:
		QuadShape() : rect(kDefaultRect) { }
		QuadShape(float32 w, float32 h) : rect(w * -0.5f, h * -0.5f, w, h) { }
		QuadShape(float32 x, float32 y, float32 w, float32 h) : rect(x, y, w, h) { }
		QuadShape(const RectF& rt) : rect(rt) { }

		virtual size_t generatePositions(std::vector<vec3>& pos, float32 z = 0.0f);
		virtual size_t generateUV(const RectF& uvRect, std::vector<vec2>& uvs);
		virtual size_t generateIndices(std::vector<uint16>& indices);
		virtual size_t generateWireframe(std::vector<uint16>& indices);

		static size_t generateWireframeImpl(std::vector<uint16>& indices);
		static size_t generateIndicesImpl(std::vector<uint16>& indices);
		static size_t generateUVImpl(const RectF& uvRect, std::vector<vec2>& uvs);
		static size_t generatePositionsImpl(const RectF& rect, std::vector<vec3>& pos, float32 z = 0.0f, float32 angle = 0.0f);

		virtual VolumePtr createVolume();

		RectF rect;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(ParallelogramShape, QuadShape)
	public:
		ParallelogramShape()
			: QuadShape()
			, skew(0.0f)
		{ }
		ParallelogramShape(float32 w, float32 h, float32 sk)
			: QuadShape(w, h)
			, skew(sk)
		{ }

		ParallelogramShape(const RectF& rt, float32 sk)
			: QuadShape(rt)
			, skew(sk)
		{ }

		virtual size_t generatePositions(std::vector<vec3>& pos, float32 z = 0.0f);

		float32 skew;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(GridShape, QuadShape)
	public:
		static const size_t kDefaultGridWidth;
		static const size_t kDefaultGridHeight;

		GridShape()
			: QuadShape()
			, gridWidth(kDefaultGridWidth)
			, gridHeight(kDefaultGridHeight)
		{ }

		size_t gridWidth;
		size_t gridHeight;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(CircleShape, Shape)
	public:
		CircleShape()
			: Shape()
			, radius(1.0f)
			, granularity(32)
		{ }

		virtual size_t generatePositions(std::vector<vec3>& pos, float32 z = 0.0f);
		virtual size_t generateUV(const RectF& uvRect, std::vector<vec2>& uvs);
		virtual size_t generateIndices(std::vector<uint16>& indices);
		virtual size_t generateWireframe(std::vector<uint16>& indices);

		virtual VolumePtr createVolume();

		float32 radius;
		int32 granularity;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(EllipseShape, Shape)
	public:
		EllipseShape()
			: Shape()
			, width(1.0f)
			, height(1.0f)
			, granularity(32)
			, alphaBoarder(0.0f)
		{ }

		virtual size_t generatePositions(std::vector<vec3>& pos, float32 z = 0.0f);
		virtual size_t generateUV(const RectF& uvRect, std::vector<vec2>& uvs);
		virtual size_t generateIndices(std::vector<uint16>& indices);
		virtual size_t generateWireframe(std::vector<uint16>& indices);
		virtual size_t generateColors(std::vector<ColorB>& colors, ColorB& color);

		virtual VolumePtr createVolume();

		float32 width;
		float32 height;
		int32 granularity;
		float32 alphaBoarder;
	};
}