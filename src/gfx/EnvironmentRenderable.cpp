#include "PCH.h"

#include "gfx/EnvironmentRenderable.h"


namespace cs
{

	const vec2 kEdgeVertexUV = kZero2;
	const vec2 kCenterVertexUV = kOne2;

	const float32 kOffsetRange = 0.0f;

	typedef BitMask<EnvironmentModificationType, EnvironmentModifyTypeMAX> EnvModifyMask;
	struct EnvCommand
	{
		char key;
		const char* desc;
		EnvModifyMask flags;
		bool(EnvironmentRenderable::*func_ptr)(size_t);
	};

	const std::vector<EnvironmentModificationType> kEnvironmentModifyTypeTriangle = { EnvironmentModifyTypeTriangle };
	const std::vector<EnvironmentModificationType> kEnvironmentModifyTypeFace = { EnvironmentModifyTypeFace };

	const EnvCommand kEnvCommand[] =
	{
		{
			's',
			"Subdivide a face, preserving edge information, on the selected face",
			EnvModifyMask(kEnvironmentModifyTypeFace),
			&EnvironmentRenderable::subdivideFace
		},
		{
			'r',
			"Remove a face (including all edge information) on the selected face",
			EnvModifyMask(kEnvironmentModifyTypeFace),
			&EnvironmentRenderable::removeFace
		},
		{
			'r',
			"Remove the selected triangle",
			EnvModifyMask(kEnvironmentModifyTypeTriangle),
			&EnvironmentRenderable::removeTriangle
		},
	};

	DEFINE_META_PRIMITIVE(EnvVertex, EnvVertex);
	DEFINE_META_PRIMITIVE(EnvTriangle, EnvTriangle);
	DEFINE_META_PRIMITIVE(EnvQuad, EnvQuad);

	DEFINE_META_VECTOR(EnvVertexList, EnvVertex, EnvVertexList);
	DEFINE_META_VECTOR(EnvTriangleList, EnvTriangle, EnvTriangleList);
	DEFINE_META_VECTOR(EnvEdgeList, EnvQuad, EnvEdgeList);
	DEFINE_META_VECTOR(EnvBoundaryVertexIndexSerialize, uint16, EnvBoundaryVertexIndexSerialize);

	BEGIN_META_CLASS(EnvironmentModificationTypeDrop)
	END_META();

	BEGIN_META_CLASS(EnvironmentModificationTypeDropVertex)
	END_META();

	BEGIN_META_CLASS(EnvironmentModificationTypeDropFace)
	END_META();

	BEGIN_META_CLASS(EnvironmentModificationTypeDropTriangle)
	END_META();

	BEGIN_META_CLASS(EnvironmentRenderable)
		
		ADD_MEMBER(positions);
			SET_MEMBER_IGNORE_GUI();
		
		ADD_MEMBER(faces);
			SET_MEMBER_IGNORE_GUI();
		
		ADD_MEMBER(stitches);
			SET_MEMBER_IGNORE_GUI();
		
		ADD_MEMBER(edges);
			SET_MEMBER_IGNORE_GUI();

		ADD_MEMBER(boundaryIndexSerialize);
			SET_MEMBER_IGNORE_GUI();

		ADD_MEMBER_PTR(mainTextureHandle);
			SET_MEMBER_COLLAPSEABLE();
			SET_MEMBER_START_COLLAPSED();
			
		ADD_MEMBER_PTR(edgeTextureHandle);
			SET_MEMBER_COLLAPSEABLE();
			SET_MEMBER_START_COLLAPSED();

		ADD_MEMBER(centerColor);
			SET_MEMBER_CALLBACK_POST(&EnvironmentRenderable::updateCenterParams);
		ADD_MEMBER(edgeColor);
			SET_MEMBER_CALLBACK_POST(&EnvironmentRenderable::updateEdgeParams);

		ADD_MEMBER_PTR(shaderHandle);
			SET_MEMBER_COLLAPSEABLE();
			SET_MEMBER_START_COLLAPSED();

		ADD_MEMBER(size);
			SET_MEMBER_CALLBACK_POST(&EnvironmentRenderable::refresh);

		ADD_MEMBER(subdivisions);
			SET_MEMBER_CALLBACK_POST(&EnvironmentRenderable::refresh);
		
		ADD_MEMBER(stretch);
			SET_MEMBER_CALLBACK_POST(&EnvironmentRenderable::refresh);

		ADD_MEMBER(borderDimm);
			SET_MEMBER_CALLBACK_POST(&EnvironmentRenderable::refresh);
			SET_MEMBER_DEFAULT(0.0f);
			SET_MEMBER_MIN(0.0f);
			SET_MEMBER_MAX(200.0f);
		
		ADD_MEMBER_PTR(modifyType);
			ADD_COMBO_META_LABEL(EnvironmentModificationTypeDrop, "None");
			ADD_COMBO_META_LABEL(EnvironmentModificationTypeDropVertex, "Vertex");
			ADD_COMBO_META_LABEL(EnvironmentModificationTypeDropFace, "Face");
	
			ADD_COMBO_META_LABEL(EnvironmentModificationTypeDropTriangle, "Triangle");
			SET_MEMBER_CALLBACK_POST(&EnvironmentRenderable::onModificationTypeChanged);
			SET_MEMBER_IGNORE_SERIALIZATION();
	END_META();

	EnvironmentRenderable::EnvironmentRenderable()
		: geometry(nullptr)
		, subdivisions(PointI(5, 4))
		, stretch(PointF(100.0f, 100.0f))
		, size(RectF(-250.0f, -200.0f, 500.0f, 400.0f))
		, selectSize(RectF(-250.0f, -200.0f, 500.0f, 400.0f))
		, borderDimm(0.0f)
		, shaderHandle(CREATE_CLASS(ShaderHandle, RenderInterface::kDefaultTextureColorShader))
		, mainTextureHandle(CREATE_CLASS(TextureHandle, RenderInterface::kDefaultTexture))
		, edgeTextureHandle(CREATE_CLASS(TextureHandle, "gradient.png"))
		, edgeColor(ColorB::White)
		, centerColor(ColorB::White)
		, volume(nullptr)
		, modifyType(CREATE_CLASS(EnvironmentModificationTypeDrop))
		, dirty(false)
		, visibleIndices(0)
	{

	}

	void EnvironmentRenderable::setEdgeParams(EnvVertex& vertex)
	{
		vertex.uv1 = kEdgeVertexUV;
		vertex.color = this->edgeColor;
	}

	EnvironmentRenderable::~EnvironmentRenderable()
	{

	}

	size_t EnvironmentRenderable::getNumBaseVertices() const
	{
		return (this->subdivisions.x + 1) * (this->subdivisions.y + 1);
	}

	size_t EnvironmentRenderable::getNumBaseIndices() const
	{
		return this->subdivisions.x * this->subdivisions.y * 6;
	}

	size_t EnvironmentRenderable::getNumVertices() const
	{
		return this->positions.size();
	}

	size_t EnvironmentRenderable::getNumIndices() const
	{
		return this->faces.size() * 3;
	}

	void EnvironmentRenderable::refresh()
	{
		this->volume = nullptr;
		this->onNew();
	}

	void EnvironmentRenderable::onNew()
	{
		this->populateGeometry(); 
		this->initGeometry();
	}

	void EnvironmentRenderable::onPostLoad(const LoadFlagMask& flags)
	{
		this->dirty = true;
		this->initGeometry();

		if (this->shaderHandle)
		{
			this->shaderHandle->onPostLoad(flags);
			this->shaderHandle->onChanged += createCallbackArg0(&EnvironmentRenderable::setShaderImpl, this);
		}

		this->refreshBoundaryVertices();
	}

	void EnvironmentRenderable::setShaderImpl()
	{
		this->shaderHandle->ignoreTextureStage(TextureStageDiffuse);
	}

	size_t EnvironmentRenderable::getVertexBufferSize()
	{
		size_t stride = this->geometry->getGeometryData()->decl.getStride();
		return this->positions.size() * stride;
	}

	size_t EnvironmentRenderable::getIndexBufferSize()
	{
		return (this->faces.size() + this->stitches.size()) * sizeof(EnvTriangle);
	}

	void EnvironmentRenderable::onModificationTypeChanged()
	{
		this->onChanged.invoke();
	}

	void EnvironmentRenderable::updateCenterParams()
	{
		for (uint16 i = 0; i < uint16(this->positions.size()); i++)
		{
			EnvVertex& vertex = this->positions[i];
			if (!this->boundaryIndex.count(i))
			{
				vertex.color = this->centerColor;
			}
		}
		this->dirty = true;
	}

	void EnvironmentRenderable::updateEdgeParams()
	{
		for (auto& it : this->boundaryIndex)
		{
			EnvVertex& vertex = this->positions[it];
			this->setEdgeParams(vertex);
		}
		this->dirty = true;
	}

	void EnvironmentRenderable::initGeometry()
	{

		cs::GeometryDataPtr data = CREATE_CLASS(GeometryData);
		data->decl.addAttrib(AttributeType::AttribPosition, { AttributeType::AttribPosition, TypeFloat, 3, 0 });
		data->decl.addAttrib(AttributeType::AttribTexCoord0, { AttributeType::AttribTexCoord0, TypeFloat, 2, sizeof(vec3) });
		data->decl.addAttrib(AttributeType::AttribTexCoord1, { AttributeType::AttribTexCoord1, TypeFloat, 2, sizeof(vec3) + sizeof(vec2) });
		data->decl.addAttrib(AttributeType::AttribColor, { AttributeType::AttribColor, TypeUnsignedByte, 4, sizeof(vec3) + sizeof(vec2) + sizeof(vec2) });

		data->vertexSize = this->getNumVertices();
		data->indexSize = this->getNumIndices();
		data->storage = BufferStorageDynamic;

		this->geometry = CREATE_CLASS(DynamicGeometry, data);

		DynamicGeometry::VertexUpdateFunc vfunc;
		vfunc = std::bind(&EnvironmentRenderable::updateVertices,
			this,
			std::placeholders::_1,
			std::placeholders::_2,
			std::placeholders::_3);
		this->geometry->setVertexUpdateFunc(vfunc);

		DynamicGeometry::IndexUpdateFunc ifunc;
		ifunc = std::bind(&EnvironmentRenderable::updateIndices,
			this,
			std::placeholders::_1,
			std::placeholders::_2);
		this->geometry->setIndexUpdateFunc(ifunc);

		GeometryPtr geoptr = std::static_pointer_cast<Geometry>(this->geometry);


		DynamicGeometry::AdjustDrawCallFunc dcfunc;
		dcfunc = std::bind(&EnvironmentRenderable::setDrawParams,
			this,
			std::placeholders::_1,
			std::placeholders::_2);
		this->geometry->setDrawCallAdjustFunc(dcfunc);

		DynamicGeometry::GetVertexSizeFunc numVFunc = std::bind(&EnvironmentRenderable::getVertexBufferSize, this);
		this->geometry->setVertexBufferSizeFunc(numVFunc);

		DynamicGeometry::GetIndexSizeFunc numIFunc = std::bind(&EnvironmentRenderable::getIndexBufferSize, this);
		this->geometry->setIndexBufferSizeFunc(numIFunc);

		this->geometry->update();
	}

	void EnvironmentRenderable::addFace(size_t top_left, size_t top_right, size_t bot_left, size_t bot_right, bool flip)
	{
		if (flip)
		{
			this->faces.push_back({ uint16(bot_left), uint16(top_left), uint16(bot_right) });
			this->faces.push_back({ uint16(bot_right), uint16(top_left), uint16(top_right), });
		}
		else
		{
			this->faces.push_back({ uint16(bot_left), uint16(top_left), uint16(top_right) });
			this->faces.push_back({ uint16(bot_left), uint16(top_right), uint16(bot_right) });
		}
	}

	void EnvironmentRenderable::populateGeometry()
	{
		this->positions.clear();

		this->size.pos.x = -this->size.size.w / 2.0f;
		this->size.pos.y = -this->size.size.h / 2.0f;

		float32 startX = this->size.pos.x;
		float32 startY = this->size.pos.y;

		bool hasBorder = this->borderDimm > 0.0f && this->subdivisions.w > 2 && this->subdivisions.h > 2;
		
		float32 incX = (hasBorder) ?
			std::max<float32>(0.0f, (this->size.size.w - (2.0f * this->borderDimm)) / float32(this->subdivisions.w - 2)) :
			this->size.size.w / float32(this->subdivisions.w);

		float32 incY = (hasBorder) ?
			std::max<float32>(0.0f, (this->size.size.h  - (2.0f * this->borderDimm)) / float32(this->subdivisions.h - 2)) :
			this->size.size.h / float32(this->subdivisions.h);

		for (size_t h = 0; h < size_t(this->subdivisions.h + 1); h++)
		{
			float32 offset_y = 0.0f;

			if (hasBorder)
			{
				offset_y = (h == 0 || h == (this->subdivisions.h - 1)) ? this->borderDimm : incY;
			}
			else
			{
				offset_y = incY + randomRange<float32>(-kOffsetRange, kOffsetRange);
			}

			for (size_t w = 0; w < size_t(this->subdivisions.w + 1); w++)
			{

				float32 offset_x = 0.0f;
				
				if (hasBorder)
				{
					offset_x = (w == 0 || w == (this->subdivisions.w - 1)) ? this->borderDimm : incX;
				}
				else
				{
					offset_x = incX + randomRange<float32>(-kOffsetRange, kOffsetRange);
				}

				float32 u = startX / this->stretch.w;
				float32 v = startY / this->stretch.h;

				this->positions.push_back({ vec3(startX, startY, 0.0f), vec2(u, v), kCenterVertexUV, this->centerColor });
			
				startX += offset_x;
			}

			startY += offset_y;
			startX = this->size.pos.x;
		}

		this->faces.clear();
		this->edges.clear();

		for (size_t h = 0; h < (size_t) this->subdivisions.h; h++)
		{
			for (size_t w = 0; w < (size_t) this->subdivisions.w; w++)
			{
				size_t top_left = w + (h * (this->subdivisions.w + 1));
				size_t top_right = (w + 1) + (h * (this->subdivisions.w + 1));
				size_t bot_left = w + ((h + 1) * (this->subdivisions.w + 1));
				size_t bot_right = (w + 1) + ((h + 1) * (this->subdivisions.w + 1));


				bool shouldFlip = (h == 0 && w == 0) || (h == this->subdivisions.h - 1 && w == this->subdivisions.w - 1);
				this->addFace(top_left, top_right, bot_left, bot_right, shouldFlip);

				EnvQuad edge_quad;
				edge_quad.botLeft = uint16(bot_left);
				edge_quad.botRight = uint16(bot_right);
				edge_quad.topLeft = uint16(top_left);
				edge_quad.topRight = uint16(top_right);

				edge_quad.t0 = int32(this->faces.size()) - 2;
				edge_quad.t1 = int32(this->faces.size()) - 1;
				edge_quad.isVisible = true;
				edge_quad.isFlipped = shouldFlip;

				this->edges.push_back(edge_quad);

			}
		}

		uint16 realW = this->subdivisions.w;
		uint16 realH = this->subdivisions.h;
		uint16 adjW = this->subdivisions.w + 1;
		uint16 adjH = this->subdivisions.h + 1;

		this->boundaryIndex.clear();

		// top row
		for (uint16 w = 0; w < adjW - 1; w++)
		{
			uint16 index = w;
			assert(index < this->positions.size());
			this->boundaryIndex.insert(index);
		}

		// right row
		for (uint16 h = 0; h < adjH - 1; h++)
		{
			uint16 index = realW + (h * adjW);
			assert(index < this->positions.size());
			this->boundaryIndex.insert(index);
		}
			
		// bottom row
		for (uint16 w = 0; w < adjW - 1; w++)
		{
			uint16 index = (adjW * adjH) - w - 1;
			assert(index < this->positions.size());
			this->boundaryIndex.insert(index);
		}
			
		// left row
		for (uint16 h = 0; h < adjH - 1; h++)
		{
			uint16 index = (realH - h) * adjW;
			assert(index < this->positions.size());
			this->boundaryIndex.insert(index);
		}

		this->boundaryIndexSerialize.clear();
		for (auto& index : this->boundaryIndex)
		{
			this->boundaryIndexSerialize.push_back(index);
		}

		
		this->updateEdgeParams();
		this->refreshBoundaryVertices();
		
		log::info("Faces: ", this->faces.size());
		log::info("Positions: ", this->positions.size());
	}

	void EnvironmentRenderable::queueGeometry(RenderTraversal traversal, DisplayListNode& display_node)
	{
		if (!this->isVisible())
			return;


		if (traversal == RenderTraversalMain && this->getNumVertices() > 0 && this->getNumIndices() > 0)
		{
			if (this->dirty)
			{
				this->geometry->update();
				this->dirty = false;

				FloatExtentCalculator xExt, yExt;
				for (auto& it : this->positions)
				{
					xExt.evaluate(it.position.x);
					yExt.evaluate(it.position.y);
				}

				this->selectSize = createRectFromExtents(xExt, yExt);
			}

			assert(this->geometry != nullptr);
			
			display_node.geomList.push_back(std::static_pointer_cast<Geometry>(this->geometry));
		}
	}

	void EnvironmentRenderable::draw() const
	{

	}

	size_t EnvironmentRenderable::updateVertices(uchar* data, size_t bufferSize, VertexDeclaration& decl)
	{
		if (this->positions.size() == 0)
			return 0;

		assert(this->getNumVertices() <= (bufferSize / sizeof(EnvVertex)));
		memcpy(data, (void*)&this->positions[0], this->positions.size() * sizeof(EnvVertex));
		return this->getNumVertices();
	}

	size_t EnvironmentRenderable::updateIndices(uchar* data, size_t bufferSize)
	{
		if (this->faces.size() == 0)
			return 0;

		this->visibleIndices = 0;
		for (auto& it : this->edges)
		{
			EnvQuad& quad = it;
			if (!quad.isVisible)
				continue;

			assert(quad.t0 >= 0 && quad.t0 < int32(this->faces.size()));
			memcpy(data + (this->visibleIndices * sizeof(uint16)), (void*) &this->faces[quad.t0], sizeof(EnvTriangle));
			this->visibleIndices += 3;
			
			assert(quad.t1 >= 0 && quad.t1 < int32(this->faces.size()));
			memcpy(data + (this->visibleIndices * sizeof(uint16)), (void*)&this->faces[quad.t1], sizeof(EnvTriangle));
			this->visibleIndices += 3;
			
		}

		if (this->stitches.size() > 0)
		{
			memcpy(data + (this->visibleIndices * sizeof(uint16)), (void*)&this->stitches[0], this->stitches.size() * sizeof(EnvTriangle));
			this->visibleIndices += this->stitches.size() * 3;
		}

		return this->visibleIndices;
	}

	void EnvironmentRenderable::setDrawParams(int32 index, std::vector<DrawCallPtr>& dcs)
	{
		DrawCallPtr dc = CREATE_CLASS(DrawCall);
		dc->tag = "EnvironmentRenderable";
		dc->type = DrawTriangles;
		dc->indexType = TypeUnsignedShort;
		dc->count = static_cast<uint32>(this->visibleIndices);
		dc->offset = 0;
		dc->shaderHandle = this->shaderHandle;
		dc->textures[0] = this->mainTextureHandle;
		dc->textures[1] = this->edgeTextureHandle;
		dc->color = ColorB::White;
		dcs.push_back(dc);
	}

	void EnvironmentRenderable::onVertexChanged(vec3 new_pos, size_t index)
	{
		if (index >= this->positions.size())
		{
			log::error("Invalid index ", index);
			return;
		}

		EnvVertex& vertex = this->positions[index];

		vertex.position.x = new_pos.x;
		vertex.position.y = new_pos.y;
		vertex.uv0.x = new_pos.x / this->stretch.w;
		vertex.uv0.y = new_pos.y / this->stretch.h;

		this->dirty = true;

	}

	bool EnvironmentRenderable::subdivideFace(size_t index)
	{
		enum EdgeType { Left, Top, Right, Bottom, EdgeMAX };
		typedef BitMask<EdgeType, EdgeMAX> StitchMask;
		struct local
		{
			static void getStitches(int32 index, EnvEdgeList& edge_list, SizeI& subdivisions, StitchMask& mask)
			{
				int32 parent_index = index;
				while (index != -1)
				{
					parent_index = index;
					EnvQuad& quad = edge_list[index];
					index = quad.parent;
				}

				mask.setAll();
				if (parent_index < subdivisions.w)
					mask.unset(Top);
				if (parent_index >= (subdivisions.w * (subdivisions.h - 1)))
					mask.unset(Bottom);
				if (parent_index % subdivisions.w == 0)
					mask.unset(Left);
				if (parent_index % subdivisions.w == (subdivisions.w - 1))
					mask.unset(Right);
			}
		};

		log::info("subdivide face at index ", index);
		EnvQuad& quad = this->edges[index];
		if (quad.t0 == -1 || quad.t1 == -1)
		{
			log::error("Cannot subdivide an ampty or partial quad");
			return false;
		}

		// create new positions from the quadrant formed by the quad
		vec3& top_left = this->positions[quad.topLeft].position;
		vec3& bot_left = this->positions[quad.botLeft].position;
		vec3& top_right = this->positions[quad.topRight].position;
		vec3& bot_right = this->positions[quad.botRight].position;
		

		vec3 center = (top_left + bot_left + top_right + bot_right) / 4.0f;
		vec3 top_edge = (top_left + top_right) / 2.0f;
		vec3 bot_edge = (bot_left + bot_right) / 2.0f;
		vec3 left_edge = (top_left + bot_left) / 2.0f;
		vec3 right_edge = (top_right + bot_right) / 2.0f;

		enum { SubCenter, SubLeft, SubTop, SubRight, SubBot };
		const vec3 new_positions[] = 
		{ 
			center, 
			left_edge, 
			top_edge, 
			right_edge, 
			bot_edge 
		};
		
		uint16 posSize = static_cast<uint16>(this->positions.size());
        uint16 idx[5];
		
        idx[0] = posSize + uint16(SubCenter);
        idx[1] = posSize + uint16(SubLeft);
        idx[2] = posSize + uint16(SubTop);
        idx[3] = posSize + uint16(SubRight);
        idx[4] = posSize + uint16(SubBot);
		

		// push back new vertices
		for (auto& it : new_positions)
		{
			float32 u = it.x / this->stretch.w;
			float32 v = it.y / this->stretch.h;

			this->positions.push_back({ it, vec2(u, v) });
		}
		
		// create new faces
		const uint16 new_indices[4][4] = 
		{
			{ idx[SubLeft], quad.topLeft, idx[SubTop], idx[SubCenter] },
			{ idx[SubCenter], idx[SubTop], quad.topRight, idx[SubRight] },
			{ quad.botLeft, idx[SubLeft], idx[SubCenter], idx[SubBot] },
			{ idx[SubBot], idx[SubCenter], idx[SubRight], quad.botRight }
		};

		for (size_t i = 0; i < 4; i++)
		{
			const uint16* cur_quad = new_indices[i];
			
			// size_t top_left, size_t top_right, size_t bot_left, size_t bot_right
			// this->faces.push_back({ bot_left, top_left, top_right });
			// this->faces.push_back({ bot_left, top_right, bot_right });

			this->addFace(cur_quad[1], cur_quad[2], cur_quad[0], cur_quad[3], quad.isFlipped);
			//this->faces.push_back({ cur_quad[0], cur_quad[1], cur_quad[2] });
			//this->faces.push_back({ cur_quad[0], cur_quad[2], cur_quad[3] });

			EnvQuad edge_quad;
			edge_quad.botLeft = cur_quad[0];
			edge_quad.botRight = cur_quad[3];
			edge_quad.topLeft = cur_quad[1];
			edge_quad.topRight = cur_quad[2];

			edge_quad.t0 = static_cast<int32>(this->faces.size()) - 2;
			edge_quad.t1 = static_cast<int32>(this->faces.size()) - 1;
			edge_quad.isVisible = quad.isVisible;
			edge_quad.parent = static_cast<int32>(index);
			edge_quad.isFlipped = quad.isFlipped;
		
			this->edges.push_back(edge_quad);
		}

		StitchMask mask;
		local::getStitches(int32(index), this->edges, this->subdivisions, mask);
		if (mask.test(Left))
		{
			log::info("Adding stich on the left");
			this->stitches.push_back({ quad.topLeft, idx[SubLeft], quad.botLeft });
		}

		if (mask.test(Right))
		{
			log::info("Adding stich on the right");
			this->stitches.push_back({ idx[SubRight], quad.topRight, quad.botRight });
		}

		if (mask.test(Top))
		{
			log::info("Adding stich on the top");
			this->stitches.push_back({ quad.topRight, idx[SubTop], quad.topLeft });
		}

		if (mask.test(Bottom))
		{
			log::info("Adding stich on the bottom");
			this->stitches.push_back({ quad.botRight, quad.botLeft, idx[SubBot] });
		}

		quad.isVisible = false;

		return true;
	}

	bool EnvironmentRenderable::indexIsEdge(uint16 index) const
	{
		return this->boundaryIndex.find(index) != this->boundaryIndex.end();
	}

	bool EnvironmentRenderable::indexIsInVisibleQuad(uint16 index) const
	{
		for (size_t i = 0; i < this->edges.size(); i++)
		{
			const EnvQuad& quad = this->edges[i];
			if (quad.containsVisibleIndex(index))
				return true;
		}
		return false;
	}

	void EnvironmentRenderable::adjustEdgeForDuplicates()
	{
		for (EnvBoundaryVertexIndex::iterator it = this->boundaryIndex.begin(); it != this->boundaryIndex.end(); it++)
		{
			if (!this->indexIsInVisibleQuad(*it))
			{
				log::info("Erasing orphaned position value");
				it = this->boundaryIndex.erase(it);
				if (it == this->boundaryIndex.end())
				{
					break;
				}

			}
		}

		EnvBoundaryVertexIndex::iterator last_it = this->boundaryIndex.begin();
		for (EnvBoundaryVertexIndex::iterator it = this->boundaryIndex.begin(); it != this->boundaryIndex.end(); it++)
		{
			if (last_it != it && it != this->boundaryIndex.end() && last_it != this->boundaryIndex.end() && *last_it == *it)
			{
				log::info("Erasing duplicate adjacent position");
				it = this->boundaryIndex.erase(it);
				if (it == this->boundaryIndex.end())
				{
					break;
				}
			}
			last_it = it;
		}

		this->refreshBoundaryVertices();
	}

	void EnvironmentRenderable::refreshBoundaryVertices()
	{
		this->boundaryIndex.clear();
		for (auto& index : this->boundaryIndexSerialize)
		{
			this->boundaryIndex.insert(index);
		}

		this->boundary.clear();
		for (auto& index : this->boundaryIndex)
		{
			assert(index < this->positions.size());
			this->boundary.push_back(this->positions[index].position);
		}

	}

	void EnvironmentRenderable::adjustEdgeForRemovedFace(size_t face_index)
	{
		
		assert(face_index >= 0 && face_index < this->edges.size());
		EnvQuad& quad = this->edges[face_index];

		std::vector<uint16> edge_face_indices;
		std::vector<uint16> remainder_indices;

		uint16 indices_to_test[] =
		{
			quad.botLeft,
			quad.topLeft,
			quad.topRight,
			quad.botRight
		};

		for (uint16 i = 0; i < 4; i++)
		{
			uint16 index_to_test = indices_to_test[i];
			if (this->indexIsEdge(index_to_test))
				edge_face_indices.push_back(index_to_test);
			else
				remainder_indices.push_back(index_to_test);
		}

		// corners an edge
		if (edge_face_indices.size() == 3)
		{
			// check for any edges that are no longer visible anywhere
			for (std::vector<uint16>::iterator it = edge_face_indices.begin(); it != edge_face_indices.end(); it++)
			{
				uint16 index_to_test = *it;
				if (!this->indexIsInVisibleQuad(index_to_test))
				{
					it = edge_face_indices.erase(it);
					if (it == edge_face_indices.end())
						break;
				}
			}
		}

		// borders an edge
		if (edge_face_indices.size() == 2)
		{
			uint16 start_index = edge_face_indices[0] < edge_face_indices[1] ? edge_face_indices[0] : edge_face_indices[1];
			
			// edge case handling - if the start index is actually the last and we're looping
			if (edge_face_indices[0] == this->boundaryIndex.size() - 1 && edge_face_indices[1] == 1)
				start_index = edge_face_indices[0];

			vec3 start_position = this->positions[start_index].position;

			std::vector<uint16> index_ordered;

			if (remainder_indices.size() == 2)
			{
				vec3 p0 = this->positions[remainder_indices[0]].position;
				vec3 p1 = this->positions[remainder_indices[1]].position;

				float d0 = glm::distance(p0, start_position);
				float d1 = glm::distance(p1, start_position);

				if (d0 < d1)
				{
					index_ordered.push_back(remainder_indices[0]);
					index_ordered.push_back(remainder_indices[1]);
				}
				else
				{
					index_ordered.push_back(remainder_indices[1]);
					index_ordered.push_back(remainder_indices[0]);
				}
			}
			else if(remainder_indices.size() == 1)
			{
				index_ordered.push_back(remainder_indices[0]);
			}
			

			if (index_ordered.size() > 0)
			{
				this->boundaryIndex.insert(index_ordered.begin(), index_ordered.end());
				this->boundaryIndexSerialize.insert(this->boundaryIndexSerialize.end(), index_ordered.begin(), index_ordered.end());
			}
		}

		this->adjustEdgeForDuplicates();
	}

	bool EnvironmentRenderable::removeFace(size_t index)
	{
		log::info("Removing face at index ", index);
		assert(index >= 0 && index < this->edges.size());
		EnvQuad& quad = this->edges[index];

		if (quad.isVisible)
		{
			quad.isVisible = false;
			this->dirty = true;

			this->adjustEdgeForRemovedFace(index);
			return true;
		}
		return false;
	}

	bool EnvironmentRenderable::removeTriangle(size_t index)
	{
		log::info("Removing triangle at index ", index);
		assert(index >= 0 && index < this->faces.size());
		for (size_t i = 0; i < this->edges.size(); ++i)
		{
			EnvQuad& quad = this->edges[i]; 
			if (quad.t0 == index)
			{
				quad.t0 = -1;
			}
			if (quad.t1 == index)
			{
				quad.t1 = -1;
			}		
		}

		this->dirty = true;

		return true;
	}

	void EnvironmentRenderable::addVolumeCallbacks(size_t index, SelectableVolume& volume)
	{
		for (auto& it : kEnvCommand)
		{
			const EnvCommand& command = it;
			if (!command.flags.test(this->modifyType->getType()))
				continue;

			CallbackPtr onClick = createCallbackArg1(command.func_ptr, this, index);
			volume.onKeyPressCallbacks[command.key] = SelectableVolumeCallbackInfo(onClick, command.desc);
		}
	}

	void EnvironmentRenderable::getSelectableVolume(SelectableVolumeList& selectable_volumes)
	{

		if (!this->volume)
			this->volume = CREATE_CLASS(QuadVolume, this->selectSize);

		SelectableVolume volume;
		volume.volume = std::static_pointer_cast<Volume>(this->volume);
		volume.type = (this->modifyType->getType() == EnvironmentModifyTypeNone) ? SelectableVolumeTypeDraw : SelectableVolumeTypeNone;

		selectable_volumes.push_back(volume);
		EnvironmentModificationType modType = this->modifyType->getType();
		switch (modType)
		{
			case EnvironmentModifyTypeVertex:
			{
				for (size_t i = 0; i < this->positions.size(); i++)
				{
					EnvVertex& vertex = this->positions[i];
					SelectableVolume volume;
					volume.volume = CREATE_CLASS(CircleVolume, vertex.position, 5.0f);
					volume.type = SelectableVolumeTypeVertex;
					volume.indexData = i;
					volume.positionWatcher = (vec3*) &this->positions[i].position;

					CallbackArg2<void, vec3, size_t>::CallbackFunc onFunc = std::bind(&EnvironmentRenderable::onVertexChanged, this, std::placeholders::_1, std::placeholders::_2);
					volume.onChangedCallback = CallbackArg2<void, vec3, size_t>(onFunc);

					selectable_volumes.push_back(volume);
				}
			} break;
			case EnvironmentModifyTypeFace:
			{
				for (size_t i = 0; i < this->edges.size(); ++i)
				{
					EnvQuad& quad = this->edges[i];
					if (!quad.isVisible)
						continue;

					FloatExtentCalculator xExtent;
					FloatExtentCalculator yExtent;

					std::vector<uint16> index;
					quad.getUniqueIndices(index);
					if (index.size() == 0)
						continue;

					std::vector<vec3> pos;
					for (auto& it : index)
					{
						vec3& vertex = this->positions[it].position;
						pos.push_back(vertex);
					}

					SelectableVolume volume;
					volume.volume = CREATE_CLASS(PolygonVolume, pos);
					volume.type = SelectableVolumeTypeFace;
					volume.indexData = i;

					this->addVolumeCallbacks(i, volume);
					selectable_volumes.push_back(volume);
				}

			} break;
			case EnvironmentModifyTypeTriangle:
			{
				for (size_t i = 0; i < this->faces.size(); i++)
				{
					EnvTriangle& triangle = this->faces[i];
					const vec3& p0 = this->positions[triangle.f0].position;
					const vec3& p1 = this->positions[triangle.f1].position;
					const vec3& p2 = this->positions[triangle.f2].position;

					SelectableVolume volume;
					volume.volume = CREATE_CLASS(TriangleVolume, p0, p1, p2);
					volume.type = SelectableVolumeTypeTriangle;
					volume.indexData = i;

					this->addVolumeCallbacks(i, volume);
					selectable_volumes.push_back(volume);
				}
			} break;
			default:
				break;
		}
	}

	namespace text
	{
		template <>
		void serializePrim<EnvVertex>(std::ostream& oss, RefVariant prim)
		{
			EnvVertex& vertex = *reinterpret_cast<EnvVertex*>(prim.getData());
			oss << "[" 
				<< vertex.position.x << ", " 
				<< vertex.position.y << ", "
				<< vertex.position.z << ", "
				<< vertex.uv0.x << ", "
				<< vertex.uv0.y << ", "
				<< vertex.uv1.x << ", "
				<< vertex.uv1.y << ", "
				<< (uint16) vertex.color.r << ", "
				<< (uint16) vertex.color.g << ", "
				<< (uint16) vertex.color.b << ", "
				<< (uint16) vertex.color.a << "]";
		}

		template <>
		void serializePrim<EnvTriangle>(std::ostream& oss, RefVariant prim)
		{
			EnvTriangle& triangle = *reinterpret_cast<EnvTriangle*>(prim.getData());
			oss << "["
				<< triangle.f0 << ", "
				<< triangle.f1 << ", "
				<< triangle.f2 << "]";
		}

		template <>
		void serializePrim<EnvQuad>(std::ostream& oss, RefVariant prim)
		{
			EnvQuad& edge = *reinterpret_cast<EnvQuad*>(prim.getData());
			oss << "["
				<< edge.botLeft << ", "
				<< edge.topLeft << ", "
				<< edge.topRight << ", "
				<< edge.botRight << ", "
				<< edge.t0 << ", "
				<< edge.t1 << ", "
				<< edge.parent << ", "
				<< (edge.isVisible ? 1 : 0) << ", "
				<< (edge.isFlipped ? 1 : 0) << "]";

		}

		template <>
		void deserializePrim<EnvVertex>(JsonValue value, RefVariant prim)
		{
			EnvVertex& vertex = *reinterpret_cast<EnvVertex*>(prim.getData());
			JsonIterator it = begin(value);
			setAndInc<float32>(it, vertex.position.x);
			setAndInc<float32>(it, vertex.position.y);
			setAndInc<float32>(it, vertex.position.z);
			setAndInc<float32>(it, vertex.uv0.x);
			setAndInc<float32>(it, vertex.uv0.y);
			setAndInc<float32>(it, vertex.uv1.x);
			setAndInc<float32>(it, vertex.uv1.y);
			setAndInc<uchar>(it, vertex.color.r);
			setAndInc<uchar>(it, vertex.color.g);
			setAndInc<uchar>(it, vertex.color.b);
			setAndInc<uchar>(it, vertex.color.a);
		}

		template <>
		void deserializePrim<EnvTriangle>(JsonValue value, RefVariant prim)
		{
			EnvTriangle& triangle = *reinterpret_cast<EnvTriangle*>(prim.getData());
			JsonIterator it = begin(value);
			setAndInc<uint16>(it, triangle.f0);
			setAndInc<uint16>(it, triangle.f1);
			setAndInc<uint16>(it, triangle.f2);
		}

		template <>
		void deserializePrim<EnvQuad>(JsonValue value, RefVariant prim)
		{
			EnvQuad& edge = *reinterpret_cast<EnvQuad*>(prim.getData());
			JsonIterator it = begin(value);
			setAndInc<uint16>(it, edge.botLeft);
			setAndInc<uint16>(it, edge.topLeft);
			setAndInc<uint16>(it, edge.topRight);
			setAndInc<uint16>(it, edge.botRight);
			setAndInc<int32>(it, edge.t0);
			setAndInc<int32>(it, edge.t1);
			setAndInc<int32>(it, edge.parent);
			setAndIncBool(it, edge.isVisible);
			setAndIncBool(it, edge.isFlipped);
		}
	}
}
