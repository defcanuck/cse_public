#include "PCH.h"

#include "gfx/MeshRenderable.h"
#include "global/ResourceFactory.h"

namespace cs
{
	BEGIN_META_CLASS(MeshRenderable)
		ADD_MEMBER_PTR(meshHandle);
	END_META();

	MeshRenderable::MeshRenderable()
		: meshHandle(CREATE_CLASS(MeshHandle, RenderInterface::kCubeMesh))
	{
		this->setBounds();
	}

	MeshRenderable::~MeshRenderable()
	{
		
	}

	void MeshRenderable::onPostLoad(const LoadFlagMask& flags)
	{
		log::info("MeshRenderable::onPostLoad");
		this->setBounds();

		this->meshHandle->onChanged += createCallbackArg0(&MeshRenderable::setBounds, this);

		if (this->meshHandle.get())
		{
			this->meshHandle->onMeshChanged();
		}
	}

	void MeshRenderable::setBounds()
	{
		if (this->meshHandle)
		{
			const MeshPtr& mesh = this->meshHandle->getMesh();
			assert(mesh);
			const MeshAABB& aabb = mesh->getAABB();
			this->polygonListVolume = CREATE_CLASS(AABBVolume, aabb.mmin, aabb.mmax);
		}
	}

	void MeshRenderable::draw() const
	{
		if (this->meshHandle)
		{
			this->meshHandle->draw();
		}
	}

	void MeshRenderable::setMesh(const std::string& fileName)
	{
		MeshPtr mesh = std::static_pointer_cast<Mesh>(ResourceFactory::getInstance()->loadResource<Mesh>(fileName));
		if (mesh.get())
		{
			this->meshHandle = CREATE_CLASS(MeshHandle, mesh);
			this->meshHandle->onChanged += createCallbackArg0(&MeshRenderable::setBounds, this);
		
			this->setBounds();
		}
	}

	void MeshRenderable::queueGeometry(RenderTraversal traversal, DisplayListNode& display_node)
	{
		if (this->meshHandle)
		{
			this->meshHandle->queueGeometry(traversal, display_node);
		}
	}

	void MeshRenderable::getSelectableVolume(SelectableVolumeList& selectable_volumes)
	{
		SelectableVolume volume;
		volume.volume = this->getVolume();
		volume.type = SelectableVolumeTypeDraw;

		selectable_volumes.push_back(volume);
	}
}