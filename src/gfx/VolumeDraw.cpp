#include "PCH.h"

#include "gfx/VolumeDraw.h"

#include "gfx/RenderInterface.h"
#include "gfx/Renderer.h"

namespace cs
{
	VolumeDraw::VolumeDraw(const VolumePtr& ptr, const ColorB& c) :
		color(c)
	{
		if (!ptr.get() || ptr->getNumPositions() == 0)
		{
			log::error("No volume specified for draw!");
			return;
		}
			
		this->volume = ptr;
		cs::GeometryDataPtr data = CREATE_CLASS(GeometryData);
        data->decl.addAttrib(AttributeType::AttribPosition, { AttributeType::AttribPosition, TypeFloat, 3, 0 });
		data->decl.addAttrib(AttributeType::AttribColor, { AttributeType::AttribColor, TypeFloat, 4, sizeof(vec3) });

		data->vertexSize = this->volume->getNumPositions();
		data->indexSize = this->volume->getNumEdges();
		data->storage = BufferStorageDynamic;

		this->geom = CREATE_CLASS(DynamicGeometry, data);

		DynamicGeometry::VertexUpdateFunc vfunc;
		vfunc = std::bind(&VolumeDraw::updateVertices,
			this,
			std::placeholders::_1,
			std::placeholders::_2,
			std::placeholders::_3);
		this->geom->setVertexUpdateFunc(vfunc);

		DynamicGeometry::IndexUpdateFunc ifunc;
		ifunc = std::bind(&VolumeDraw::updateIndices,
			this,
			std::placeholders::_1,
			std::placeholders::_2);
		this->geom->setIndexUpdateFunc(ifunc);

		DynamicGeometry::AdjustDrawCallFunc dcfunc;
		dcfunc = std::bind(&VolumeDraw::setDrawParams,
			this,
			std::placeholders::_1,
			std::placeholders::_2);
		this->geom->setDrawCallAdjustFunc(dcfunc);


		DynamicGeometry::GetVertexSizeFunc numVFunc = std::bind(&VolumeDraw::getVertexBufferSize, this);
		this->geom->setVertexBufferSizeFunc(numVFunc);

		DynamicGeometry::GetIndexSizeFunc numIFunc = std::bind(&VolumeDraw::getIndexBufferSize, this);
		this->geom->setIndexBufferSizeFunc(numIFunc);

		this->geom->update();
	}

	size_t VolumeDraw::getVertexBufferSize()
	{
		size_t stride = this->geom->getGeometryData()->decl.getStride();
		return this->volume->getNumPositions() * stride;
	}

	size_t VolumeDraw::getIndexBufferSize()
	{
		return this->volume->getNumEdges() * sizeof(uint16);
	}

	size_t VolumeDraw::updateVertices(uchar* data, size_t bufferSize, VertexDeclaration& decl)
	{
		uint32 vertexCtr = 0;
		size_t nv = this->volume->getNumPositions();

		std::vector<vec3> positions;
		this->volume->getPositions(positions);
		for (size_t i = 0; i < nv; i++)
		{
			vec3* pos = decl.getAttributePointerAtIndex<vec3>(data, AttribPosition, vertexCtr);
			assert(size_t(pos) - size_t(data) < bufferSize);
			*pos = positions[i];

			ColorF* col = decl.getAttributePointerAtIndex<ColorF>(data, AttribColor, vertexCtr);
			*col = toColorF(this->color);
			vertexCtr++;
		}
		return vertexCtr;
	}

	size_t VolumeDraw::updateIndices(uchar* data, size_t bufferSize)
	{
		uint32 indexCtr = 0;
		uint16* indices = reinterpret_cast<uint16*>(data);

		std::vector<uint16> wireframe;
		size_t ni = this->volume->getEdges(wireframe);
		for (size_t i = 0; i < ni; i++)
		{
			indices[indexCtr++] = wireframe[i];
		}
		return indexCtr;
	}

	void VolumeDraw::setDrawParams(int32 index, std::vector<DrawCallPtr>& dcs)
	{
		DrawCallPtr dc = CREATE_CLASS(DrawCall);
		dc->type = this->volume->getDrawType();
		dc->indexType = TypeUnsignedShort;
		dc->count = static_cast<uint32>(this->volume->getNumEdges());
		dc->offset = 0;
		dc->shaderHandle = RenderInterface::kDefaultColorShader;
		dc->depthTest = false;
		dc->color = this->color;
		dcs.push_back(dc);
	}

	void VolumeDraw::update()
	{
		if (!this->geom)
			return;

		this->geom->update();
	}

	void VolumeDraw::draw(cs::CameraPtr& camera, const Transform& transform)
	{
		if (!this->geom)
		{
			log::error("No geometry defined! Abort!");
			return;
		}
			
		mat4 projection = camera->getCurrentProjection();
		mat4 view = camera->getCurrentView();
		mat4 model = transform.getCurrentMatrix();
		mat4 mvp = projection * view * model;

		RenderInterface::getInstance()->setLineWidth(1.0f);
        GeometryPtr geomPtr = std::static_pointer_cast<Geometry>(this->geom);
		renderer::draw(mvp, this->color, geomPtr);
	}
}