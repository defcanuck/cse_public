#pragma once

#include "ClassDef.h"

#include "geom/Volume.h"
#include "gfx/BatchDraw.h"
#include "gfx/Geometry.h"
#include "math/Transform.h"
#include "gfx/DisplayList.h"

namespace cs
{
	struct BatchRenderableParams
	{
		float32 dt;
		ColorB parentColor;
		Transform transform;
		RectF orthoRect;
		SortMethod sortMethod;
	};

	CLASS_DEFINITION_REFLECT(Renderable)
	public:

		Renderable();
		Renderable(GeometryPtr& geom);

		virtual ~Renderable();

		virtual void draw() const;
		virtual void process(float32 dt);
		
		virtual void batch(
			const std::string& tag,
			BatchDrawList& display_list,
			const BatchRenderableParams& params,
			uint32& numVertices,
			uint16& numIndices) { }

		virtual void refresh() { }
		virtual void getSelectableVolume(SelectableVolumeList& selectable_volumes) { }

		virtual bool batchable() const { return false; }

		virtual void queueGeometry(RenderTraversal traversal, DisplayListNode& display_node);
		virtual void addGeometry(const GeometryPtr& geom);

		virtual void adjustSize(const vec2& sz, AnimationScaleType type) { }
		virtual void adjustTexRect(const RectF& rect) { }
		
		bool isVisible() const { return this->visible; }
		virtual bool isValid() const { return true; }

		void setLayer(const int32& lr) { this->layer = lr; }
		const int32& getLayer() const { return this->layer; }

		Event onChanged;
		RenderTraversalMask traversalMask;

		virtual VolumePtr getVolume() { return nullptr; }
			
	protected:

		
		std::vector<GeometryPtr> geometry;
		vec4 color;
		bool visible;
		int32 layer;
	};
}
