#pragma once


#include "scene/Camera.h"
#include "gfx/DynamicGeometry.h"
#include "geom/Volume.h"

namespace cs
{
	CLASS_DEFINITION(VolumeDraw)
	public:
		VolumeDraw(const VolumePtr& ptr, const ColorB& c = ColorB::White);

		size_t updateVertices(uchar* data, size_t bufferSize, VertexDeclaration& decl);
		size_t updateIndices(uchar* data, size_t bufferSize);

		void setDrawParams(int32 index, std::vector<DrawCallPtr>& dcs);

		void draw(cs::CameraPtr& camera, const Transform& transform);
		void setColor(const ColorB& col) { this->color = col; }

		size_t getVertexBufferSize();
		size_t getIndexBufferSize();

		VolumePtr& getVolume() { return volume; }

		void update();

	protected:

		VolumeDraw()
			: volume(nullptr)
			, color(ColorB::White)
		{

		}
		
	private:

		ColorB color;
		VolumePtr volume;
		DynamicGeometryPtr geom;

	};
}