#include "PCH.h"

#include "gfx/DynamicGeometry.h"
#include "gfx/RenderInterface.h"
#include "gfx/Uniform.h"
#include "global/Utils.h"

namespace cs
{

	DynamicGeometry::DynamicGeometry(const GeometryDataPtr& init_data) :
		Geometry(init_data, true),
		vertexUpdate(nullptr),
		indexUpdate(nullptr),
		adjustDraw(nullptr),
		vertexBufSize(nullptr),
		indexBufSize(nullptr)
	{ 
		this->vertexBufSize = std::bind(&DynamicGeometry::getVertexBufferSize, this);
		this->indexBufSize = std::bind(&DynamicGeometry::getIndexBufferSize, this);
	}

	void DynamicGeometry::update()
	{

		if (this->doubleBuffered)
			this->swap();

		if (this->vertexUpdate)
		{
			VertexDeclaration& decl = this->getGeometryData()->decl;

			size_t vbSize = this->vertexBufSize();
			BufferObjectPtr& vbuffer = this->getVertexBuffer();
			if (vbuffer->getSize() < vbSize)
			{
				log::print(LogInfo, "Resizing Vertex Buffers to ", vbSize, " bytes");
				for (auto& it : this->vbo)
					it->resize(vbSize);
			}

			RenderInterface::getInstance()->setBuffer(vbuffer);
			void* ptr = vbuffer->lock(BufferAccessWrite);
			if (ptr)
			{
				size_t sz = vbuffer->getSize();
				uchar* data = reinterpret_cast<uchar*>(ptr);
				this->vertexUpdate(data, sz, decl);
				vbuffer->unlock();
			}
			RenderInterface::getInstance()->clearBuffer(BufferTypeVertex);
		}

		if (this->indexUpdate)
		{
			size_t ibSize = this->indexBufSize();
			BufferObjectPtr& ibuffer = this->getIndexBuffer();
			if (ibuffer->getSize() < ibSize)
			{
				log::print(LogInfo, "Resizing Index Buffer to ", ibSize, ", bytes");
				for (auto& it : this->ibo)
					it->resize(ibSize * 2);
			}

			RenderInterface::getInstance()->setBuffer(ibuffer);
			void* ptr = ibuffer->lock(BufferAccessWrite);
			if (ptr)
			{
				size_t sz = ibuffer->getSize();
				uchar* data = reinterpret_cast<uchar*>(ptr);

				this->indexUpdate(data, sz);
				ibuffer->unlock();
			}
			RenderInterface::getInstance()->clearBuffer(BufferTypeIndex);
		}
	}

	void DynamicGeometry::draw(DrawCallOverrides* overrides, int32 drawIndex, ColorF tint)
	{
		std::vector<DrawCallPtr> drawCalls;
		if (this->adjustDraw)
			this->adjustDraw(drawIndex, drawCalls);
		

		this->bindAll();

		for (auto& it : drawCalls)
		{
			DrawCallPtr drawCall = it;
			if (drawCall->preCallback)
			{
				drawCall->preCallback();
				this->bindAll();
			}

			if (drawCall->count != 0)
			{
				drawCall->setUniforms(tint);
				drawCall->bind((Geometry*) this, overrides);

				RenderInterface::getInstance()->draw((Geometry*) this, drawCall);
			}

			if (drawCall->postCallback)
			{
				drawCall->postCallback();
				this->bindAll();
			}
		}
	}

	void DynamicGeometry::setVertexUpdateFunc(VertexUpdateFunc& func)
	{
		this->vertexUpdate = func;
	}

	void DynamicGeometry::setVertexUpdateFunc(vertexUpdateFunc func)
	{
		VertexUpdateFunc vf = std::bind(func, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		this->setVertexUpdateFunc(vf);
	}

	void DynamicGeometry::setIndexUpdateFunc(IndexUpdateFunc& func)
	{
		this->indexUpdate = func;
	}

	void DynamicGeometry::setIndexUpdateFunc(indexUpdateFunc func)
	{
		IndexUpdateFunc iuf = std::bind(func, std::placeholders::_1, std::placeholders::_2);
		this->setIndexUpdateFunc(iuf);
	}

	void DynamicGeometry::setDrawCallAdjustFunc(AdjustDrawCallFunc& func)
	{
		this->adjustDraw = func;
	}

	void DynamicGeometry::setDrawCallAdjustFunc(adjustDrawCallFunc func)
	{
		AdjustDrawCallFunc adcf = std::bind(func, std::placeholders::_1, std::placeholders::_2);
		this->setDrawCallAdjustFunc(adcf);
	}

	void DynamicGeometry::setVertexBufferSizeFunc(GetVertexSizeFunc& func)
	{
		this->vertexBufSize = func;
	}

	void DynamicGeometry::setVertexBufferSizeFunc(getVertexSizeFunc func)
	{
		GetVertexSizeFunc adcf = std::bind(func);
		this->setVertexBufferSizeFunc(adcf);
	}

	void DynamicGeometry::setIndexBufferSizeFunc(GetIndexSizeFunc& func)
	{
		this->indexBufSize = func;
	}

	void DynamicGeometry::setIndexBufferSizeFunc(getIndexSizeFunc func)
	{
		GetIndexSizeFunc adcf = std::bind(func);
		this->setIndexBufferSizeFunc(adcf);
	}

}
