#include "PCH.h"

#include "ecs/system/DrawableSystem.h"
#include "ecs/ECS_Utils.h"

#include "ecs/Entity.h"
#include "gfx/RenderInterface.h"
#include "gfx/Attribute.h"
#include "game/Context.h"
#include "math/Plane.h"

#include <functional>

namespace cs
{
	DrawableSystem::DrawableSystem(ECSContext* cxt) 
		: BaseSystem(cxt)
        , allocated(false)
	{
		this->subscribeForComponent<DrawableComponent>(this->parentContext);

		

		// Shadows are inverse additive to main pass
		//this->batch[RenderTraversalShadow]->options.blend.setDestBlend(BlendOne);
		//this->batch[RenderTraversalShadow]->options.blend.setSourceBlend(BlendOne);

	}

	DrawableSystem::~DrawableSystem()
	{
        this->removeComponentSubscription<DrawableComponent>(this->parentContext);
	}
    
    void DrawableSystem::allocGeometry()
    {
        GeometryDataPtr data = CREATE_CLASS(cs::GeometryData);
        
        data->decl.addAttrib(AttributeType::AttribPosition, { AttributeType::AttribPosition, TypeFloat, 3, 0 });
        data->decl.addAttrib(AttributeType::AttribTexCoord0, { AttributeType::AttribTexCoord0, TypeFloat, 2, sizeof(vec3) });
		data->decl.addAttrib(AttributeType::AttribTexCoord1, { AttributeType::AttribTexCoord1, TypeFloat, 2, sizeof(vec3) + sizeof(vec2) });
		data->decl.addAttrib(AttributeType::AttribColor, { AttributeType::AttribColor, TypeFloat, 4, sizeof(vec3) + sizeof(vec2) + sizeof(vec2) });
        
        data->vertexSize = kInitNumElements * 4;
        data->indexSize = kInitNumElements * 6;
        data->storage = BufferStorageDynamic;
        
        for (size_t i = 0; i < RenderTraversalMAX; ++i)
        {
            this->batch[i] = CREATE_CLASS(BatchDraw, data);
            this->batch[i]->sortMethod = SortMethodY;
            this->batch[i]->options.depth.setDepthTest(i == RenderTraversalMain);
        }
        
        this->allocated = true;
    }

	void DrawableSystem::addRenderTarget(RenderTexturePtr ptr)
	{

	}

	void DrawableSystem::processImpl(SystemUpdateParams* params)
	{
		BaseSystem::ComponentIdMap drawables;
		this->getEnabledComponents<DrawableComponent>(drawables);

		RectF orthoRect;
		if (this->cullCamera)
		{
			static vec3 kPlanePos(0.0, 0.0, 0.0);
			static vec3 kPlaneNorm(0.0, 0.0, -1.0);

			RectI view = this->cullCamera->getViewport();

			vec2 screen0(0.0f, 0.0f);
			vec2 screen1(float32(view.size.w), float32(view.size.h));

			Ray ray0 = this->cullCamera->getRay(screen0);
			Ray ray1 = this->cullCamera->getRay(screen1);

			Plane worldPlane(kPlanePos, kPlaneNorm);

			vec3 hit0(0.0f, 0.0f, 0.0f), hit1(0.0f, 0.0f, 0.0f);


			if (worldPlane.intersects(ray0, hit0) &&
				worldPlane.intersects(ray1, hit1))
			{
				orthoRect.pos.x = std::min<float32>(hit0.x, hit1.x);
				orthoRect.pos.y = std::min<float32>(hit0.y, hit1.y);
				orthoRect.size.w = abs(hit0.x - hit1.x);
				orthoRect.size.h = abs(hit0.y - hit1.y);

				// expand the ortho rect so it's bigger than the view
				//orthoRect.growY(2.0f);
			}
		}
		
		for (size_t i = 0; i < RenderTraversalMAX; ++i)
		{
			std::vector<DrawableComponentPtr> batchableComponents;
			RenderTraversal traversalType = static_cast<RenderTraversal>(i);
			ScopedAccumTimer timer(&Context::updateTimes[ContextSplineUpdate]);
            
            if (this->batch[i].get())
            {
                this->batch[i]->clear();
            }
            
			for (const auto& it : drawables)
			{
				const DrawableComponentPtr& drawable =
					std::static_pointer_cast<DrawableComponent>(it.second);

				if (!drawable->getParent()->getEnabled())
				{
					continue;
				}

				drawable->process(params->animationDt);

				if (drawable->isBatcheable())
				{
					batchableComponents.push_back(drawable);
				}
			
			}

			{
				ScopedAccumTimer timer(&Context::updateTimes[ContextSceneBatchTraverse]);
                if (batchableComponents.size() > 0 && !this->allocated)
                {
                    this->allocGeometry();
                }

                for (const auto& it : batchableComponents)
				{
					it->batch(
						this->batch[i]->sortMethod,
						orthoRect,
						params->animationDt,
						this->batch[i]->drawData, 
						this->batch[i]->numVertices, 
						this->batch[i]->numIndices, 
						traversalType
					);
				}
			}
		}
	}

	void DrawableSystem::flush(DisplayList& display_list)
	{
		for (size_t i = 0; i < RenderTraversalMAX; ++i)
		{
			RenderTraversal traversal_type = (RenderTraversal)i;
			DisplayListTraversal& traversal_list = display_list.traversals[traversal_type];
			if (!traversal_list.camera.get())
				continue;

			mat4 projection = traversal_list.camera->getCurrentProjection();
			mat4 view = traversal_list.camera->getCurrentView();

			BaseSystem::ComponentIdMap drawables;
			this->getEnabledComponents<DrawableComponent>(drawables);
			for (const auto& it : drawables)
			{
				const DrawableComponentPtr& drawable =
					std::static_pointer_cast<DrawableComponent>(it.second);

				if (!drawable->getParent()->getEnabled())
				{
					continue;
				}

				drawable->flush(traversal_list);
			}

			{
				ScopedAccumTimer timer(&Context::updateTimes[ContextSceneBatchBuffer]);
                if (this->batch[i].get())
                {
                    this->batch[i]->update();
                    this->batch[i]->flush(traversal_list);
                }
            }
		}
	}

	void DrawableSystem::onComponentSystemAdd(uint32 id, ComponentPtr& component)
	{

	}

	void DrawableSystem::onComponentSystemRemove(uint32 id, ComponentPtr& component)
	{

	}
}
