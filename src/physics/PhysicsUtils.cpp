#include "PCH.h"

#include "physics/PhysicsUtils.h"


namespace cs
{
	PhysicsShapePtr PhysicsUtils::createShapeFromRenderable(RenderablePtr& renderable)
	{
		// dirty kludge
		if (renderable.get())
		{
			if (renderable->getMetaData()->getName() == "BatchRenderable")
			{
				return createShape(std::static_pointer_cast<BatchRenderable>(renderable));
			}
			else if (renderable->getMetaData()->getName() == "EnvironmentRenderable")
			{
				return createShape(std::static_pointer_cast<EnvironmentRenderable>(renderable));
			}
			else if (renderable->getMetaData()->getName() == "SpineRenderable")
			{
				return createShape(std::static_pointer_cast<SpineRenderable>(renderable));
			}
		}

		return PhysicsShapePtr();
	}

	template <>
	PhysicsShapePtr PhysicsUtils::createShape(std::shared_ptr<SpineRenderable> renderable)
	{
		QuadVolumePtr volPtr = std::static_pointer_cast<QuadVolume>(renderable->getVolume());
		if (volPtr.get())
		{
			return CREATE_CLASS(PhysicsShapeBox, volPtr);
		}
		assert(false);
		log::error("createShape needs a shape yo");
		return CREATE_CLASS(PhysicsShapeBox);
	}

	template <>
	PhysicsShapePtr PhysicsUtils::createShape(std::shared_ptr<BatchRenderable> renderable)
	{
		SelectableVolumeList selectable_volumes;
		renderable->getSelectableVolume(selectable_volumes);

		assert(selectable_volumes.size() == 1);
		SelectableVolume& volume = selectable_volumes[0];

		QuadVolumePtr volPtr = std::static_pointer_cast<QuadVolume>(volume.volume);
		if (volPtr.get())
		{
			return CREATE_CLASS(PhysicsShapeBox, volPtr);
		}
		
		assert(false);
		log::error("createShape needs a shape yo");
		return CREATE_CLASS(PhysicsShapeBox);
	}

	template <>
	PhysicsShapePtr PhysicsUtils::createShape(std::shared_ptr<EnvironmentRenderable> renderable)
	{
		renderable->refreshBoundaryVertices();

		Vec3List pos_list = renderable->getBoundaryVertices();
		ChainVolumePtr poly_volume = CREATE_CLASS(ChainVolume, pos_list);

		return CREATE_CLASS(PhysicsShapeChain, poly_volume);
	}
}