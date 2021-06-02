#pragma once

#include "ClassDef.h"

#include "gfx/Color.h"
#include "gfx/TextureHandle.h"
#include "gfx/ShaderResource.h"
#include "gfx/RenderInterface.h"
#include "gfx/DynamicGeometry.h"
#include "gfx/DisplayList.h"
#include "gfx/DrawOptions.h"

#define BATCH_TEXTURE_STAGES 2
#define BATCH_DEFAULT_STAGE 0

namespace cs
{

	enum SortMethod
	{
		SortMethodNone = -1,
		SortMethodLayer,
		SortMethodY, 
		SortMethodZ,
		SortMethodMAX
	};

	struct BatchDrawData
	{
		
		BatchDrawData()
			: drawType(DrawTriangles)
			, positions()
			, uvs0()
			, uvs1()
			, indices()
			, shader(RenderInterface::kDefaultColorShader)
		{
			this->texture[BATCH_DEFAULT_STAGE] = RenderInterface::kWhiteTexture;
			for (int i = BATCH_DEFAULT_STAGE + 1; i < BATCH_TEXTURE_STAGES; ++i)
				this->texture[i] = nullptr;
		}

		BatchDrawData(const BatchDrawData& rhs)
		{
			*this = rhs;
		}

		void operator=(const BatchDrawData& rhs)
		{
			this->positions = rhs.positions;
			this->uvs0 = rhs.uvs0;
			this->uvs1 = rhs.uvs1;
			this->indices = rhs.indices;
			this->vcolors = rhs.vcolors;
			for (int i = 0; i < BATCH_TEXTURE_STAGES; ++i)
				this->texture[i] = rhs.texture[i];

			this->shader = rhs.shader;
			this->drawType = rhs.drawType;
			this->postDrawCallback = rhs.postDrawCallback;
		}

		std::vector<vec3> positions;
		std::vector<vec2> uvs0;
		std::vector<vec2> uvs1;
		std::vector<uint16> indices;
		std::vector<ColorB> vcolors;

		DrawType drawType;
		TextureHandlePtr texture[BATCH_TEXTURE_STAGES];
		ShaderHandlePtr shader;
		DrawCall::PostDraw postDrawCallback;

	};

	typedef std::shared_ptr<BatchDrawData> BatchDrawDataPtr;

	struct BatchDrawParams
	{
		
		BatchDrawParams(BatchDrawDataPtr data_ptr, uint32 vert_offset)
			: data(data_ptr)
			, vertexOffset(vert_offset)
			, transform()
			, bounds(RectF())
			, tint(ColorB::White)
			, sortValue(0.0f)
			, depthValue(0.0f)
			, layer(0)
			, depth(true)
			, depthType(DepthLess)
			, blend()
			, flags(0)
		{ 
			assert(data.get() != nullptr);
		}

		BatchDrawParams(const BatchDrawParams& rhs)
			: data(rhs.data)
			, vertexOffset(rhs.vertexOffset)
			, transform(rhs.transform)
			, bounds(rhs.bounds)
			, tint(rhs.tint)
			, sortValue(rhs.sortValue)
			, depthValue(rhs.depthValue)
			, layer(rhs.layer)
			, postDrawCallback(rhs.postDrawCallback)
			, preDrawCallback(rhs.preDrawCallback)
			, depth(rhs.depth)
			, depthType(rhs.depthType)
			, blend(rhs.blend)
			, flags(rhs.flags)
		{ }

		void operator=(const BatchDrawParams& rhs)
		{
			this->data = rhs.data;
			this->vertexOffset = rhs.vertexOffset;
			this->transform = rhs.transform;
			this->bounds = rhs.bounds;
			this->tint = rhs.tint;
			this->sortValue = rhs.sortValue;
			this->depthValue = rhs.depthValue;
			this->layer = rhs.layer;
			this->postDrawCallback = rhs.postDrawCallback;
			this->preDrawCallback = rhs.preDrawCallback;
			this->depth = rhs.depth;
			this->depthType = rhs.depthType;
			this->blend = rhs.blend;
			this->flags = rhs.flags;
		}

		BatchDrawDataPtr data;
		uint32 vertexOffset;
		mat4 transform;
		RectF bounds;
		ColorB tint;
		std::string tag;
		float32 sortValue;
		float32 depthValue;
		int32 layer;
		DrawCall::PreDraw preDrawCallback;
		DrawCall::PostDraw postDrawCallback;
		bool depth;
		DepthType depthType;
		DrawOptionsBlend blend;
		int32 flags;
	};

	typedef std::vector<BatchDrawParams> BatchDrawList;
	
	struct BatchDisplayList
	{
		BatchDrawList drawList;
		uint32 numVertices;
		uint32 numIndices;
	};
	
	CLASS_DEFINITION(BatchDraw)
	public:

		enum BatchType
		{
			BatchTypeStatic,
			BatchTypeDynamic,
			//...
			BatchTypeMAX
		};

		BatchDraw(GeometryDataPtr& ptr) 
			: data(ptr)
			, geom(nullptr)
			, sortMethod(SortMethodNone)
			, forceBlend(false)
		{ 
			this->init();
		}

		size_t updateVertices(uchar* data, size_t bufferSize, VertexDeclaration& decl);
		size_t updateIndices(uchar* data, size_t bufferSize);
		void setDrawParams(int32 index, std::vector<DrawCallPtr>& dcs);

		size_t getVertexBufferSize();
		size_t getIndexBufferSize();

		void clear();
		void update();
		void draw();
		void sort(BatchDrawList::iterator start, BatchDrawList::iterator end, SortMethod sort);

		void flush(DisplayListTraversal& traversal_list);

		uint32 numVertices;
		uint16 numIndices;
		BatchDrawList drawData;
		SortMethod sortMethod;
		DrawOptions options;
		bool forceBlend;

	private:

		void init();

		GeometryDataPtr& data;
		DynamicGeometryPtr geom;

		std::vector<DrawCallPtr> draws;
		std::vector<int32> flags;
		
	};
}
