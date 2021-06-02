#include "PCH.h"

#include "ecs/comp/DrawableComponent.h"
#include "ecs/Entity.h"
#include "global/Utils.h"

#include "gfx/BatchRenderable.h"
#include "gfx/MeshRenderable.h"
#include "gfx/EnvironmentRenderable.h"
#include "gfx/TrailRenderable.h"
#include "gfx/SplineRenderable.h"
#include "animation/spine/SpineRenderable.h"

namespace cs
{
	BEGIN_META_CLASS(DrawableComponent)
		ADD_MEMBER_PTR(renderable);
			ADD_COMBO_META(BatchRenderable);
			ADD_COMBO_META(MeshRenderable);
			ADD_COMBO_META(EnvironmentRenderable);
			ADD_COMBO_META(SpineRenderable);
			ADD_COMBO_META(SplineRenderable);
			ADD_COMBO_META(TrailRenderable);
			SET_MEMBER_CALLBACK_POST(&DrawableComponent::onRenderableChanged);
	END_META()

	DrawableComponent::DrawableComponent(GeometryPtr& geom)
	{
		RenderablePtr rend = CREATE_CLASS(Renderable);
		rend->addGeometry(geom);
		this->setRenderable(rend);
	}

	void DrawableComponent::draw() const
	{ 
		if (this->renderable)
			this->renderable->draw();
	}

	void DrawableComponent::flush(DisplayListTraversal& traversal_list)
	{
		if (!this->getEnabled() || 
			!this->renderable ||
			this->renderable->batchable() ||
			!this->renderable->isValid() || 
			!this->renderable->traversalMask.test(traversal_list.type))
		{
			return;
		}

		mat4 projection = traversal_list.camera->getCurrentProjection();
		mat4 view = traversal_list.camera->getCurrentView();

		DisplayListNode node; 
		mat4 model(1.0f);

		Entity* parent = this->getParent();
		if (parent)
		{
			model = parent->getWorldTransform().getCurrentMatrix();
			node.color = toVec4(parent->getColor());
			node.tag = parent->getName();
		}

		this->renderable->queueGeometry(traversal_list.type, node);
		
		node.objectToWorld = model;
		node.mvp = projection * view * model;
		node.layer = this->renderable->getLayer();

		traversal_list.nodes.push_sort(node, &DisplayListNode::sort);
		//traversal_list.nodes.push_back(node);
	}

	void DrawableComponent::process(float32 dt) 
	{ 
		if (!this->getEnabled() ||
			!this->renderable ||
			this->isBatcheable())
		{
			return;
		}

		this->renderable->process(dt);
	}

	bool DrawableComponent::isBatcheable() const
	{
		return this->renderable.get() && this->renderable->batchable();
	}

	void DrawableComponent::batch(
		SortMethod sortMethod,
		const RectF& orthoRect,
		float32 dt, 
		BatchDrawList& batchDrawList, 
		uint32& numVertices, 
		uint16& numIndices, 
		RenderTraversal traversalType)
	{
		if (!this->getEnabled() || 
			!this->renderable || 
			!this->renderable->batchable() ||
			!this->renderable->traversalMask.test(traversalType))
			return;

		Entity* parent = this->getParent();
		if (!parent)
			return;

		const std::string tag = parent->getName();
		size_t batch_start = batchDrawList.size();

		BatchRenderableParams params;
		params.parentColor = parent->getColor();
		params.transform = parent->getWorldTransform();
		params.dt = dt;
		params.orthoRect = orthoRect;
		params.sortMethod = sortMethod;

		this->renderable->batch(tag, batchDrawList, params, numVertices, numIndices);

		/*
		std::string tag = this->getParent()->getName();
		for (size_t i = batch_start; i < batchDrawList.size(); i++)
		{
			batchDrawList[i].tag = tag;
		}
		*/
	}
	
	void DrawableComponent::onPostLoad(const LoadFlagMask& flags)
	{
		if (!flags.test(LoadFlagsRenderable))
		{
			return;
		}

		if (this->renderable)
		{
			this->renderable->onPostLoad(flags);	
			this->renderable->onChanged += createCallbackArg0(&DrawableComponent::onRenderableUpdated, this);
		}
	}

	void DrawableComponent::getSelectableVolume(SelectableVolumeList& selectable_volumes)
	{
		if (this->renderable)
		{
			this->renderable->getSelectableVolume(selectable_volumes);
		}
	}

	void DrawableComponent::onRenderableChanged()
	{
		if (this->renderable)
		{
			this->renderable->refresh();
			this->renderable->onChanged += createCallbackArg0(&DrawableComponent::onRenderableUpdated, this);
		}
		this->onChanged.invoke();
		
	}

	void DrawableComponent::onRenderableUpdated()
	{
		// flag the selection as dirty if the renderable changed - need to re-select it
		Entity* parent = this->getParent();
		if (parent)
		{
			parent->setSelectionDirty(true);
		}
	}
}
