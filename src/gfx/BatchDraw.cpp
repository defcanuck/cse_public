#include "PCH.h"

#include "gfx/BatchDraw.h"
#include "gfx/Attribute.h"

namespace cs
{
	const float32 kLayerOffset = 1000.0f;

	void BatchDraw::init()
	{

		this->numIndices = 0;
		this->numVertices = 0;

		this->geom = CREATE_CLASS(DynamicGeometry, this->data);

		DynamicGeometry::VertexUpdateFunc vfunc;
		vfunc = std::bind(
			&BatchDraw::updateVertices,
			this,
			std::placeholders::_1,
			std::placeholders::_2,
			std::placeholders::_3);
		this->geom->setVertexUpdateFunc(vfunc);

		DynamicGeometry::IndexUpdateFunc ifunc;
		ifunc = std::bind(
			&BatchDraw::updateIndices,
			this,
			std::placeholders::_1,
			std::placeholders::_2);
		this->geom->setIndexUpdateFunc(ifunc);

		DynamicGeometry::AdjustDrawCallFunc dcfunc;
		dcfunc = std::bind(
			&BatchDraw::setDrawParams,
			this,
			std::placeholders::_1,
			std::placeholders::_2);
		this->geom->setDrawCallAdjustFunc(dcfunc);
		
		DynamicGeometry::GetVertexSizeFunc numVFunc = std::bind(&BatchDraw::getVertexBufferSize, this);
		this->geom->setVertexBufferSizeFunc(numVFunc);

		DynamicGeometry::GetIndexSizeFunc numIFunc = std::bind(&BatchDraw::getIndexBufferSize, this);
		this->geom->setIndexBufferSizeFunc(numIFunc);

	}

	size_t BatchDraw::updateVertices(uchar* data, size_t bufferSize, VertexDeclaration& decl)
	{
		size_t vertexCtr = 0;
		const float32 kAdjustYSort = -0.1f;

		char* pos_ptr = decl.getAttributePointerAtIndex<char>(data, AttributeType::AttribPosition, 0);
		char* uv0_ptr = decl.getAttributePointerAtIndex<char>(data, AttributeType::AttribTexCoord0, 0);
		char* uv1_ptr = decl.getAttributePointerAtIndex<char>(data, AttributeType::AttribTexCoord1, 0);

		char* col_ptr = decl.getAttributePointerAtIndex<char>(data, AttributeType::AttribColor, 0);
		size_t stride = decl.getStride();

		for (size_t i = 0; i < this->drawData.size(); i++)
		{
			BatchDrawParams& params = this->drawData[i];
			BatchDrawDataPtr& draw_data = params.data;

			for (size_t t = 0; t < draw_data->positions.size(); t++)
			{
				vec3* pos = reinterpret_cast<vec3*>(PTR_ADD(pos_ptr, vertexCtr * stride));
				glm::vec4 v(draw_data->positions[t], 1);
				v = params.transform * v;

				float32 z = (params.layer * -kAdjustYSort) + ((1.0f - params.depthValue) * kAdjustYSort);
				(*pos) = vec3(v.x, v.y, v.z + z);
                
				if (draw_data->uvs0.size() > t)
				{
					vec2* uv = reinterpret_cast<vec2*>(PTR_ADD(uv0_ptr, vertexCtr * stride));
					(*uv) = draw_data->uvs0[t];
				}
                else
                {
                    vec2* uv = reinterpret_cast<vec2*>(PTR_ADD(uv0_ptr, vertexCtr * stride));
                    (*uv) = vec2(0.0f, 0.0f);
                }

				if (uv1_ptr)
                {
                    vec2* uv = reinterpret_cast<vec2*>(PTR_ADD(uv1_ptr, vertexCtr * stride));
                    if (draw_data->uvs1.size() > t)
                    {
                        (*uv) = draw_data->uvs1[t];
                    }
                    else
                    {
                        (*uv) = vec2(0.0f, 0.0f);
                    }
                }

				if (col_ptr)
                {
                    if (draw_data->vcolors.size() > t)
                    {
                        ColorF* col = reinterpret_cast<ColorF*>(PTR_ADD(col_ptr, vertexCtr * stride));
                        ColorF fcol = toColorF(draw_data->vcolors[t]);
                        (*col) = fcol;
                    }
                    else
                    {
                        ColorF* col = reinterpret_cast<ColorF*>(PTR_ADD(col_ptr, vertexCtr * stride));
                        (*col) = ColorF::White;
                    }
                }
				vertexCtr++;
			}
		}

#ifdef DEBUG_BUFFER_FILL
		for (size_t i = 0; i < vertexCtr; i++)
		{
			vec3* pos = decl.getAttributePointerAtIndex<vec3>(data, AttribPosition, i);
			vec2* uv = decl.getAttributePointerAtIndex<vec2>(data, AttribTexCoord0, i);
			std::cout << *pos << std::endl;
			std::cout << *uv << std::endl;
		}
#endif

		return vertexCtr;
	}

	size_t BatchDraw::updateIndices(uchar* data, size_t bufferSize)
	{

		if (this->drawData.size() <= 0)
			return 0;

		this->draws.clear();

		const static DrawOptions kDefaultDrawOptions;
		struct DrawBatch
		{
			DrawBatch() 
				: drawType(DrawTriangles)
				, shader(RenderInterface::kDefaultColorShader)
				, color(255, 255, 255, 255) 
				, layer(0)
				, depth(true)
				, depthType(DepthLess)
				, flags(0)
			{ 
				for (int i = 0; i < BATCH_TEXTURE_STAGES; ++i)
					this->texture[i] = nullptr;
			}

			DrawBatch(const DrawBatch& rhs)
			{
				*this = rhs;
			}

			void operator=(const DrawBatch& rhs)
			{
				this->shader = rhs.shader;
				for (int i = 0; i < BATCH_TEXTURE_STAGES; ++i)
					this->texture[i] = rhs.texture[i];
				this->color = rhs.color;
				this->dataIndex = rhs.dataIndex;
				this->drawType = rhs.drawType;
				this->layer = rhs.layer;
				this->postCallback = rhs.postCallback;
				this->preCallback = rhs.preCallback;
				this->depth = rhs.depth;
				this->depthType = rhs.depthType;
				this->flags = rhs.flags;
			}

			DrawType drawType;
			ShaderHandlePtr shader;
			TextureHandlePtr texture[BATCH_TEXTURE_STAGES];
			ColorB color;
			std::deque<uint16> dataIndex;
			uint32 layer;
			DrawCall::PostDraw postCallback;
			DrawCall::PreDraw preCallback;
			bool depth = false;
			DepthType depthType;
			DrawOptionsBlend blend;
			int32 flags;

		};

		// Assume 1/2 efficient batching
		std::deque<DrawBatch> batchList;
		//batchList.reserve(this->drawData.size() * 2 / 3);
		
		batchList.push_back(DrawBatch());
		DrawBatch* batch = &batchList.back();
		
		batch->drawType = this->drawData[0].data->drawType;
		batch->color = this->drawData[0].tint;
		for (int i = 0; i < BATCH_TEXTURE_STAGES; ++i)
			batch->texture[i] = this->drawData[0].data->texture[i];

		batch->shader = this->drawData[0].data->shader;
		batch->layer = this->drawData[0].layer;
		batch->postCallback = this->drawData[0].postDrawCallback;
		batch->preCallback = this->drawData[0].preDrawCallback;
		batch->depth = this->drawData[0].depth;
		batch->depthType = this->drawData[0].depthType;
		batch->blend = this->drawData[0].blend;
		batch->flags = this->drawData[0].flags;

		batch->dataIndex.push_back(0);
		uint16 numDraws = uint16(this->drawData.size());
		for (uint16 i = 1; i < numDraws; i++)
		{
			const BatchDrawParams& params = this->drawData[i];
			const BatchDrawDataPtr& dd = params.data;

			bool colorMatch = batch->color != params.tint;			
			bool layerMatch = batch->layer != params.layer;
			
			bool textureMatch = false;
			for (int tex = 0; tex < BATCH_TEXTURE_STAGES; ++tex)
				textureMatch = textureMatch || (batch->texture[tex] && dd->texture[tex] && !batch->texture[tex]->equals(dd->texture[tex]));

			bool shaderMatch = !batch->shader->equals(dd->shader);
			bool callbackExists = params.postDrawCallback != nullptr || batch->postCallback != nullptr || params.preDrawCallback != nullptr || batch->preCallback != nullptr;
			bool depthMatch = params.depth != batch->depth || params.depthType != batch->depthType;
			bool blendMatch = batch->blend != params.blend;
			bool flagsMatch = batch->flags != params.flags;

			bool resetBatch =
				callbackExists ||
				(batch->drawType == DrawTriangleStrip) ||
				(batch->drawType != dd->drawType) ||
				colorMatch ||
				textureMatch ||
				shaderMatch ||
				layerMatch ||
				depthMatch ||
				blendMatch || 
				flagsMatch;
			
			if (resetBatch)
			{
				batchList.push_back(DrawBatch());
				batch = &batchList.back();

				batch->dataIndex.clear();
				batch->color = params.tint;
				for (int tex = 0; tex < BATCH_TEXTURE_STAGES; ++tex)
					batch->texture[tex] = dd->texture[tex];
				batch->shader = dd->shader;
				batch->drawType = dd->drawType;
				batch->layer = params.layer;
				batch->postCallback = params.postDrawCallback;
				batch->preCallback = params.preDrawCallback;
				batch->depth = params.depth;
				batch->depthType = params.depthType;
				batch->blend = params.blend;
				batch->flags = params.flags;
			}

			if (this->sortMethod == SortMethodY)
				batch->dataIndex.push_front(i);
			else
				batch->dataIndex.push_back(i);
		}

		// std::sort(batchList.begin(), batchList.end(), DrawBatchSort);

		uint16 indexCtr = 0;
		uint16* indices = reinterpret_cast<uint16*>(data);

		for (size_t i = 0; i < batchList.size(); i++)
		{
			DrawBatch& draw_batch = batchList[i];
			uint16 batchIndexCtr = 0;
 			DrawCallPtr dc = CREATE_CLASS(DrawCall);

			std::string tagName = "BatchDraw";
			if (draw_batch.texture[0].get() != nullptr)
			{
				tagName = draw_batch.texture[0]->getTextureName();
				assert(tagName.length() > 0);
			}

			dc->tag = tagName;
			dc->offset = indexCtr;
			dc->type = draw_batch.drawType;
			dc->indexType = TypeUnsignedShort;
			dc->shaderHandle = draw_batch.shader;
			for (int tex = 0; tex < BATCH_TEXTURE_STAGES; ++tex)
				dc->textures[tex] = draw_batch.texture[tex];
			dc->layer = draw_batch.layer;
			dc->srcBlend = BlendSrcAlpha;
			dc->dstBlend = BlendOne;
			dc->depthTest = draw_batch.depth && (draw_batch.blend.getSourceBlend() != BlendOne && draw_batch.blend.getDestBlend() != BlendOne);
			dc->depthFunc = draw_batch.depthType;
			dc->blend = draw_batch.blend.getBlendEnabled();
			dc->srcBlend = draw_batch.blend.getSourceBlend();
			dc->dstBlend = draw_batch.blend.getDestBlend();

			dc->color = draw_batch.color;

			for (auto const& idx : draw_batch.dataIndex)
			{
				const BatchDrawParams& params = this->drawData[idx];
				const BatchDrawDataPtr& dd = params.data;

				for (auto const& index : dd->indices)
				{
					uint16 adjIndex = params.vertexOffset + index;
					indices[indexCtr++] = adjIndex;
					++batchIndexCtr;
				}
			}

			dc->count = batchIndexCtr;
			dc->postCallback = draw_batch.postCallback;
			dc->preCallback = draw_batch.preCallback;
			
			if (this->forceBlend)
			{
				this->options.populateBlend(dc);
			}

			this->draws.push_back(dc);
			this->flags.push_back(draw_batch.flags);

		}

#ifdef DEBUG_BUFFER_FILL
		for (size_t i = 0; i < indexCtr; i++)
		{
			std::cout << indices[i] << std::endl;
		}
#endif

		return indexCtr;
	}

	void BatchDraw::clear()
	{
		this->numVertices = 0;
		this->numIndices = 0;
		this->drawData.clear();
		this->draws.clear();
		this->flags.clear();
	}

	void BatchDraw::update()
	{
        if (this->drawData.size() == 0)
            return;
        
		if (this->sortMethod == SortMethodY)
		{

			// sort first by later
			this->sort(this->drawData.begin(), this->drawData.end(), SortMethodLayer);

			// sort last by draw method
			BatchDrawList::iterator it = this->drawData.begin();
			if (it != this->drawData.end())
			{
				int32 curLayer = (*it).layer;
				while (it != this->drawData.end())
				{
					BatchDrawList::iterator next = it + 1;
					BatchDrawList::iterator last = it;
					while (next != this->drawData.end() && (*next).layer == curLayer)
					{
						next++;
						last++;
					}

                    BatchDrawList::iterator start = it;
                    it = next;
					this->sort(start, last, this->sortMethod);
					
					if (it != this->drawData.end())
						curLayer = (*it).layer;
				}
			}
		}
		else
		{
			this->sort(this->drawData.begin(), this->drawData.end(), this->sortMethod);
		}

		this->geom->update();
	}

	void BatchDraw::draw()
	{
		this->geom->draw(nullptr);
	}

	size_t BatchDraw::getVertexBufferSize()
	{
		size_t stride = this->geom->getGeometryData()->decl.getStride();
		return this->numVertices * stride;
	}

	size_t BatchDraw::getIndexBufferSize()
	{
		return this->numIndices * sizeof(uint16);
	}

	void BatchDraw::setDrawParams(int32 index, std::vector<DrawCallPtr>& dcs)
	{
		if (index == -1)
		{
			dcs = this->draws;
		}
		else
		{
			assert(size_t(index) < this->draws.size());
			dcs.push_back(this->draws[index]);
		}
	}

	static float32 getValueSortY(const BatchDrawParams& params)
	{
		return params.depthValue;
	}

	static float32 getValueSortZ(const BatchDrawParams& params)
	{
		return (params.transform * vec4(0.0f, 0.0, 0.0f, 1.0f)).z;
	}

	void BatchDraw::sort(BatchDrawList::iterator start, BatchDrawList::iterator end, SortMethod sort)
	{
		struct
		{
			bool operator()(const BatchDrawParams& a, const BatchDrawParams& b)
			{
				return a.layer < b.layer;
			}
		} BatchDrawSortLayer;

		struct
		{
			bool operator()(const BatchDrawParams& a, const BatchDrawParams& b)
			{
				return a.sortValue > b.sortValue;
			}
		} BatchDrawSortValue;

		struct local
		{
			static void updateVertexOffset(BatchDrawList& drawData)
			{
				uint16 vertexOffset = 0;
				for (BatchDrawList::iterator it = drawData.begin(); it != drawData.end(); ++it)
				{
					BatchDrawParams& params = (*it);
					params.vertexOffset = vertexOffset;
					vertexOffset += static_cast<uint16>(params.data->positions.size());
				}
			}
		};

		switch (sort)
		{
			case SortMethodLayer:
				std::sort(start, end, BatchDrawSortLayer);
				local::updateVertexOffset(this->drawData);
			case SortMethodY:
				std::for_each(
					start, 
					end, 
					[](BatchDrawParams& param) 
					{ 
						param.sortValue = getValueSortY(param);
					}
				);
				std::sort(start, end, BatchDrawSortValue);
				local::updateVertexOffset(this->drawData);
			case SortMethodZ:
				std::for_each(
					start,
					end,
					[](BatchDrawParams& param)
					{
						param.sortValue = getValueSortZ(param);
					}
				);
				std::sort(start, end, BatchDrawSortValue);
				local::updateVertexOffset(this->drawData);
			default:
				break;
		}
	}


	void BatchDraw::flush(DisplayListTraversal& traversal_list)
	{
		mat4 projection = traversal_list.camera->getCurrentProjection();
		mat4 view = traversal_list.camera->getCurrentView();

		mat4 mvp = projection * view;
		for (int32 i = int32(this->draws.size()) - 1; i >= 0; --i)
		{
			DisplayListNode node;
			DrawCallPtr& drawPtr = this->draws[i];
			node.tag = drawPtr->tag;
			node.mvp = mvp;
			node.geomList.push_back(DisplayListGeom(std::static_pointer_cast<Geometry>(this->geom), i));
			node.layer = drawPtr->layer;
			node.flags = this->flags[i];

			traversal_list.nodes.push_sort(node, &DisplayListNode::sort);
		}
	}
}
