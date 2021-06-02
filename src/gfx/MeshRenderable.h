#pragma once

#include "ClassDef.h"

#include "gfx/Renderable.h"
#include "gfx/Mesh.h"

namespace cs
{
	CLASS_DEFINITION_DERIVED_REFLECT(MeshRenderable, Renderable)
	public:

		MeshRenderable();
		virtual ~MeshRenderable();

		void setMesh(const std::string& fileName);
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);

		virtual void draw() const;
		virtual void queueGeometry(RenderTraversal traversal, DisplayListNode& display_node);

		virtual VolumePtr getVolume() { return std::static_pointer_cast<Volume>(this->polygonListVolume); }
		virtual void getSelectableVolume(SelectableVolumeList& selectable_volumes);

		MeshHandlePtr getMeshHandle() { return this->meshHandle; }

	private:

		void setBounds();

		MeshHandlePtr meshHandle;
		AABBVolumePtr polygonListVolume;
	};
}