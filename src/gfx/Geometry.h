#pragma once

#include "ClassDef.h"
#include "gfx/Attribute.h"
#include "gfx/Types.h"
#include "gfx/GeometryData.h"
#include "gfx/BufferObject.h"
#include "gfx/VertexArrayObject.h"

#include <functional>

namespace cs
{
	CLASS_DEFINITION(Geometry)
	
#if defined(CS_METAL)
    const static uint32 kNumBuffers = 3;
#else
    const static uint32 kNumBuffers = 2;
#endif

	public:

		Geometry(const GeometryDataPtr& init_data, bool dblBuf = false);
		Geometry(const GeometryDataPtr& init_data, uchar* vdata, uchar* idata, bool dblBuf = false);
		virtual ~Geometry();

		virtual void bindAll();
		virtual void bindVertices();
		virtual void bindIndices();

        virtual void draw(ColorF tint = ColorF::White);
		virtual void draw(DrawCallOverrides* overrides, int32 index = -1, ColorF tint = ColorF::White);
		virtual void draw(std::vector<DrawCallPtr>& drawCalls, DrawCallOverrides* overrides, ColorF tint = ColorF::White);
		virtual void update() { }
		virtual void swap();

		const GeometryDataPtr& getGeometryData() const { return data; }

		BufferObjectPtr& getVertexBuffer() { return this->vbo[this->index]; }
		BufferObjectPtr& getIndexBuffer() { return this->ibo[this->index]; }
		size_t getVertexBufferSize() { return this->vbo[this->index]->getSize(); }
		size_t getIndexBufferSize() { return this->ibo[this->index]->getSize(); }

		size_t getNumVertices() { return this->getVertexBufferSize() / this->data->decl.getStride(); }
		size_t getNumIndices() { return this->getIndexBufferSize() / sizeof(uint16); }

		uchar* getVertexBufferStagingData() { return this->vbStagingData; }
		uchar* getIndexBufferStagingData() { return this->ibStagingData; }

	protected:

		void init(bool dblBuf = false);

		GeometryDataPtr data;
		std::vector<BufferObjectPtr> vbo;
		std::vector<BufferObjectPtr> ibo;
		std::vector<VertexArrayObjectPtr> vao;

		bool doubleBuffered;
		uint16 index;

		uchar* vbStagingData;
		uchar* ibStagingData;
	};

}

