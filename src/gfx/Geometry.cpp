#include "PCH.h"

#include "gfx/Geometry.h"
#include "gfx/Types.h"
#include "gfx/gl/OpenGL.h"
#include "gfx/RenderInterface.h"

#include <algorithm>

namespace cs
{
	Geometry::Geometry(const GeometryDataPtr& init_data, bool dblBuf)
		: data(init_data)
		, doubleBuffered(dblBuf)
		, index(0)
		, vbStagingData(nullptr)
		, ibStagingData(nullptr)
	{
		this->init(dblBuf);
	}

	Geometry::Geometry(const GeometryDataPtr& init_data, uchar* vdata, uchar* idata, bool dblBuf)
		: data(init_data)
		, doubleBuffered(dblBuf)
		, index(0)
		, vbStagingData(vdata)
		, ibStagingData(idata)
	{
		this->init(dblBuf);
	}

	Geometry::~Geometry()
	{
		if (this->vbStagingData) delete[] this->vbStagingData;
		if (this->ibStagingData) delete[] this->ibStagingData;
	}

	void Geometry::bindAll()
	{
		VertexArrayObjectPtr& vaoObj = this->vao[this->index];
		if (vaoObj)
		{
			vaoObj->bind();
		}
		else
		{
			vaoObj = RenderInterface::getInstance()->createVertexArrayObject();
			RenderInterface::getInstance()->clearBuffer(BufferTypeVertex);
			
		}

		this->bindVertices();
		this->bindIndices();
	}

	void Geometry::bindVertices()
	{
		if (this->vbo[this->index])
        {
            BufferObjectPtr buffer = std::static_pointer_cast<BufferObject>(this->vbo[this->index]);
			RenderInterface::getInstance()->setBuffer(buffer);
        }
		if (this->vbo[this->index])
			this->vbo[this->index]->bindAttributes(this->data->decl);
	}

	void Geometry::bindIndices()
	{
		if (this->ibo[this->index])
			RenderInterface::getInstance()->setBuffer(this->ibo[this->index]);
	}
    
    void Geometry::draw(ColorF tint)
    {
        this->bindAll();
        for (auto& it : this->data->drawCalls)
        {
            DrawCallPtr& dc = it;
            
            dc->setUniforms(tint);
            dc->bind(this, nullptr);
            
            RenderInterface::getInstance()->draw(this, dc);
        }
    }
	void Geometry::draw(DrawCallOverrides* overrides, int32 index, ColorF tint)
	{
		this->bindAll();

		if (index == -1)
		{
			for (auto& it : this->data->drawCalls)
			{
				DrawCallPtr& dc = it;

				dc->setUniforms(tint);
				dc->bind(this, overrides);
				
				RenderInterface::getInstance()->draw(this, dc, overrides);
			}
		}
		else
		{
			assert(size_t(index) < this->data->drawCalls.size());
			DrawCallPtr& dc = this->data->drawCalls[index];

			dc->setUniforms(tint);
			dc->bind(this, overrides);
			
			RenderInterface::getInstance()->draw(this, dc, overrides);
		}
	}

	void Geometry::swap()
	{
		if (this->doubleBuffered)
		{
			this->index = (this->index + 1) % kNumBuffers;
		}
	}

	void Geometry::draw(std::vector<DrawCallPtr>& drawCalls, DrawCallOverrides* overrides, ColorF tint)
	{
		this->bindAll();
		for (auto& it : drawCalls)
		{
			DrawCallPtr& dc = it;

			dc->setUniforms(tint);
			dc->bind(this, overrides);
			RenderInterface::getInstance()->draw(this, dc);
		}
	}

	void Geometry::init(bool dblBuf)
	{
		size_t numBuffers = (dblBuf) ? kNumBuffers : 1;

		for (size_t i = 0; i < numBuffers; i++)
		{
			if (this->data->vertexSize > 0)
			{
				BufferObjectPtr vbuffer = RenderInterface::getInstance()->createBufferObject(BufferTypeVertex);
                if (vbuffer.get())
                {
                    RenderInterface::getInstance()->setBuffer(vbuffer);
                    void* vertex_data =(this->vbStagingData) ? (void*) this->vbStagingData : ((this->data->vertexData.size() > 0) ? &this->data->vertexData[0] : nullptr);
                    
                    vbuffer->alloc(this->data->vertexSize * this->data->decl.getStride(), vertex_data, this->data->storage);
                    RenderInterface::getInstance()->clearBuffer(BufferTypeVertex);

                    this->vbo.push_back(vbuffer);
                }
            }

			if (this->data->indexSize > 0)
			{
				BufferObjectPtr ibuffer = RenderInterface::getInstance()->createBufferObject(BufferTypeIndex);
				if (ibuffer.get())
                {
                    RenderInterface::getInstance()->setBuffer(ibuffer);
                    void* index_data = (void*)(this->ibStagingData) ? (void*) this->ibStagingData : ((this->data->indexData.size() > 0) ? &this->data->indexData[0] : nullptr);

                    ibuffer->alloc(this->data->indexSize * sizeof(uint16), index_data, this->data->storage);
                    RenderInterface::getInstance()->clearBuffer(BufferTypeIndex);

                    this->ibo.push_back(ibuffer);
                }
            }
			this->vao.push_back(nullptr);
		}
	}
}
