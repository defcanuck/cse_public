#include "PCH.h"

#include "ecs/system/ParticleSystem.h"
#include "ecs/comp/ParticleComponent.h"
#include "ecs/ECS_Utils.h"

namespace cs
{
	ParticleSystem::ParticleSystem(ECSContext* cxt)
		: BaseSystem(cxt)
		, heaps(CREATE_CLASS(ParticleHeapCollection))
	{
		this->subscribeForComponent<ParticleComponent>(this->parentContext);
	}

	ParticleSystem::~ParticleSystem()
	{
        this->removeComponentSubscription<ParticleComponent>(this->parentContext);
	}
    
    void ParticleSystem::clear()
    {
        
    }

	void ParticleSystem::processImpl(SystemUpdateParams* params)
	{
		BaseSystem::ComponentIdMap anims;
		this->getEnabledComponents<ParticleComponent>(anims);
		for (const auto it : anims)
		{
			const ParticleComponentPtr& fx =
				std::static_pointer_cast<ParticleComponent>(it.second);

			fx->process(params->animationDt);
		}

		for (int32 traversal = 0; traversal < RenderTraversalMAX; ++traversal)
		{
			for (auto& it : this->heaps->buffers[traversal])
			{
				ParticleHeapPtr& heap = it.second;
				heap->process(params->animationDt);
			}
		}
	}

	void ParticleSystem::flush(DisplayList& display_list)
	{
		for (int traversal = 0; traversal < RenderTraversalMAX; ++traversal)
		{
			DisplayListTraversal& traversal_list = display_list.traversals[traversal];
			if (!traversal_list.camera.get())
				continue;

			RenderInterface::getInstance()->pushDebugScope("ParticleSystem");
			for (auto& it : this->heaps->buffers[traversal])
			{
				ParticleHeapPtr& heap = it.second;
				heap->flush(traversal_list);
			}
			RenderInterface::getInstance()->popDebugScope();
		}
	}

}
