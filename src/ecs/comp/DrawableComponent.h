#pragma once

#include "ClassDef.h"

#include "gfx/Geometry.h"
#include "gfx/BatchDraw.h"
#include "gfx/Renderable.h"

#include "ecs/comp/Component.h"

#include "geom/Volume.h"

#include <vector>

namespace cs
{
	CLASS_DEFINITION_DERIVED_REFLECT(DrawableComponent, Component)
	
	public:
		DrawableComponent() :
			renderable(nullptr) { }
		DrawableComponent(GeometryPtr& geom);

		virtual ~DrawableComponent() { }

		static const std::type_index type_index_impl;

		virtual void draw() const;
		virtual void flush(DisplayListTraversal& traversal_list);

		virtual void process(float32 dt);
		virtual void batch(
			SortMethod sortMethod,
			const RectF& orthoRect,
			float32 dt,
			BatchDrawList& batch_list, 
			uint32& numVertices, 
			uint16& numIndices,
			RenderTraversal traversalType = RenderTraversalMain);
		
		template <class T>
		void setRenderable(std::shared_ptr<T>& rend) 
		{ 
			this->renderable = std::static_pointer_cast<Renderable>(rend); 
		}

		RenderablePtr& getRenderable() { return this->renderable; }

		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);

		virtual void getSelectableVolume(SelectableVolumeList& selectable_volumes);
		virtual void onRenderableUpdated();

		bool isBatcheable() const;

		Event onChanged;

	private:

		void onRenderableChanged();

		RenderablePtr renderable;
		
	};
}
