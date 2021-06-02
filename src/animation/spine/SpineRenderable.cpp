#include "PCH.h"

#include "animation/spine/SpineRenderable.h"

#
namespace cs
{
	namespace SpineUtils
	{
		static ColorB convertToColorB(spine::Color& color)
		{
			return ColorB(uchar(color.r) * 255, uchar(color.g) * 255, uchar(color.b) * 255, uchar(color.a) * 255);
		}

		static void applySkew(float& x, float y, float skew, QuadVolumePtr& volume)
		{
			if (skew != 0.0f && volume.get())
			{
				RectF rect = volume->getRect();
				vec2 pct_pos(x, y);
				vec2 pct = rectPercent<float32, vec2>(rect, pct_pos);
				x += skew * pct.y;
			}
		}

		static void applyScale(float& x, float& y, float sx, float sy)
		{
			x = x * sx;
			y = y * sy;
		}
	}

	const float32 kSplineExtentsDim = 10000.0f;

	DEFINE_META_VECTOR_NEW(SpineRenderable::AtlasTextureHandleList, TextureHandleConst, AtlasTextureHandleList);

	BEGIN_META_CLASS(SpineRenderable)
		ADD_MEMBER_PTR(skeletonHandle);
		ADD_MEMBER_PTR(textureAtlas);
		ADD_MEMBER(atlasPageTextures);
			SET_MEMBER_IGNORE_SERIALIZATION();
			SET_MEMBER_CONST();

		ADD_MEMBER(options);
			SET_MEMBER_COLLAPSEABLE();
			SET_MEMBER_START_COLLAPSED();

		ADD_MEMBER_PTR(shader);
			SET_MEMBER_COLLAPSEABLE();
			SET_MEMBER_START_COLLAPSED();

		ADD_MEMBER_STRING(skinName);
			SET_MEMBER_CALLBACK_POST(&SpineRenderable::onSkinChanged);
		
		ADD_MEMBER(animSpeed);
			SET_MEMBER_MIN(0.0f);
			SET_MEMBER_MAX(2.0f);
			SET_MEMBER_CALLBACK_POST(&SpineRenderable::onAnimationChanged);

		ADD_MEMBER_STRING(animName);
			SET_MEMBER_CALLBACK_POST(&SpineRenderable::onAnimationChanged);

		ADD_MEMBER(flipHorizontal);
			SET_MEMBER_CALLBACK_POST(&SpineRenderable::refreshBoundingVolume);
		
		ADD_MEMBER(flipVertical);
			SET_MEMBER_CALLBACK_POST(&SpineRenderable::refreshBoundingVolume);

		ADD_MEMBER(tint);
			SET_MEMBER_DEFAULT(ColorB::White);

		ADD_MEMBER(skew);
			SET_MEMBER_MIN(-500.0f);
			SET_MEMBER_MAX(500.0f);

		ADD_MEMBER(scaleX);
			SET_MEMBER_MIN(0.0f);
			SET_MEMBER_MAX(2.0f);

		ADD_MEMBER(scaleY);
			SET_MEMBER_MIN(0.0f);
			SET_MEMBER_MAX(2.0f);

	END_META();

	SpineRenderable::SpineRenderable()
		: skeletonHandle(CREATE_CLASS(SpineSkeletonHandle))
		, textureAtlas(CREATE_CLASS(SpineAtlasHandle))
		, instanceLoaded(false)
		, volume(nullptr)
		, numVertices(0)
		, numIndices(0)
		, numActiveSprites(0)
		, maxActiveSprites(0)
		, flipHorizontal(false)
		, flipVertical(false)
		, tint(ColorB::White)
		, skew(0.0f)
		, scaleX(1.0f)
		, scaleY(1.0f)
		, shader(CREATE_CLASS(ShaderHandle, RenderInterface::kDefaultTextureShader))
		, animSpeed(1.0f)
	{
		this->options.depth.setDepthWrite(false);
	}
    
    SpineRenderable::~SpineRenderable()
    {
        //log::info("Bye bye spine!");
    }

	void SpineRenderable::onNew()
	{
		this->skeletonHandle->onChanged += createCallbackArg0(&SpineRenderable::onSkeletonChanged, this);
		this->textureAtlas->onChanged += createCallbackArg0(&SpineRenderable::onAtlasChanged, this);
	}

	void SpineRenderable::onPostLoad(const LoadFlagMask& flags)
	{
		this->skeletonHandle->onChanged += createCallbackArg0(&SpineRenderable::onSkeletonChanged, this);
		this->textureAtlas->onChanged += createCallbackArg0(&SpineRenderable::onAtlasChanged, this);

		this->skeletonHandle->onPostLoad(flags);
		this->textureAtlas->onPostLoad(flags);

		this->onAtlasChanged();

		if (this->instanceLoaded)
		{
			this->instance.playAnimation(this->animName, this->animSpeed, AnimationTypeLoop);
		}
	}

	void SpineRenderable::onAnimationChanged()
	{
		if (this->instanceLoaded)
		{
			this->instance.playAnimation(this->animName, this->animSpeed, AnimationTypeLoop);
		}
	}

	void SpineRenderable::onSkeletonChanged()
	{
		if (this->textureAtlas.get())
		{
			this->instance = SpineAnimationInstance();
			if (this->skeletonHandle.get())
			{
				SpineSkeletonDataPtr& skel_data = this->skeletonHandle->getSpineSkeletonData();
				if (skel_data.get())
				{
					this->instanceLoaded =
						skel_data->getAnimationInstance(this->textureAtlas->getSpineAtlas(), this->instance);

					if (this->instanceLoaded)
					{
						spine::Skeleton* skl = this->instance.skeleton.get()->value;
						spSkeleton_setToSetupPose(skl);
						spSkeleton_updateWorldTransform(skl);
						this->refreshBoundingVolume();
						this->onSkinChanged();
					}
				}
			}
		}

		this->refreshGeometry();
	}

	void SpineRenderable::onAtlasChanged()
	{
		assert(this->textureAtlas.get());
		SpineTextureAtlasPtr& atlas_ptr = this->textureAtlas->getSpineAtlas();
		if (!atlas_ptr.get())
			return;

		this->atlasPageTextures.clear();
		std::vector<TexturePtr> textures;
		size_t num_textures = atlas_ptr->getTextures(textures);

		for (size_t i = 0; i < num_textures; i++)
		{
			TexturePtr& tex_ptr = textures[i];
			TextureHandleConstPtr textureHandle = CREATE_CLASS(TextureHandleConst, tex_ptr);
			this->atlasPageTextures.push_back(textureHandle);
			
			TextureHandlePtr renderTextureHandle = CREATE_CLASS(TextureHandle, tex_ptr);
			this->atlasPageRenderTextures.push_back(renderTextureHandle);
		}

		this->onSkeletonChanged();
	}

	void SpineRenderable::process(float32 dt)
	{
		if (this->instanceLoaded)
		{
			this->instance.applyAnimation(dt);
		}

		if (this->geometry.get())
			this->geometry->update();
	}

	void SpineRenderable::getSelectableVolume(SelectableVolumeList& selectable_volumes)
	{
		if (!this->volume.get())
			this->refreshBoundingVolume();

		SelectableVolume volume;
		volume.volume = this->volume;
		volume.type = SelectableVolumeTypeDraw;

		selectable_volumes.push_back(volume);
	}

	void SpineRenderable::refreshBoundingVolume()
	{
		if (this->numActiveSprites <= 0)
			return;

		FloatExtentCalculator xExtent;
		FloatExtentCalculator yExtent;

		for (size_t i = 0; i < this->instance.getNumSprites(); i++)
		{
			spine::Slot* slot = this->instance.skeleton->value->drawOrder[i];
			if (!slot || !slot->attachment)
			{
				continue;
			}
				
			float32* tmpVerts = nullptr;
			size_t numVerts = 0;
			switch (slot->attachment->type)
			{
				case SP_ATTACHMENT_REGION:
				{
					spine::RegionAttachment* attachment = reinterpret_cast<spine::RegionAttachment*>(slot->attachment);
					if (!attachment)
						continue;

					numVerts = 4;
					tmpVerts = new float32[numVerts * 2];
					spRegionAttachment_computeWorldVertices(attachment, slot->bone, tmpVerts, 0, 2);
					break;
				}
				case SP_ATTACHMENT_MESH:
				{
					spine::MeshAttachment* attachment = reinterpret_cast<spine::MeshAttachment*>(slot->attachment);
					numVerts = attachment->super.worldVerticesLength / 2;
					tmpVerts = new float32[numVerts * 2];
					spVertexAttachment_computeWorldVertices(&attachment->super, slot, 0, numVerts * 2, tmpVerts, 0, 2);
					
					break;
				}
                default:
                    log::info("Unknown attachment type");
                    break;
			}
			

			if (tmpVerts)
			{
				
				for (size_t t = 0; t < numVerts; t++)
				{
					float32 x = tmpVerts[(t * 2) + 0];
					float32 y = tmpVerts[(t * 2) + 1];

					x = (this->flipHorizontal) ? -x : x;
					y = (this->flipVertical) ? -y : y;

					if (x >= -kSplineExtentsDim && x <= kSplineExtentsDim)
						xExtent.evaluate(x);

					if (y >= -kSplineExtentsDim && y <= kSplineExtentsDim)
						yExtent.evaluate(y);
				}
				delete[] tmpVerts;
			}
		}

		RectF sz = createRectFromExtents(xExtent, yExtent);
		this->volume = CREATE_CLASS(QuadVolume, sz);
	}

	void SpineRenderable::queueGeometry(RenderTraversal traversal, DisplayListNode& display_node)
	{
		if (this->numActiveSprites > 0 && this->getNumVertices() > 0 && this->getNumIndices() > 0)
		{
			assert(this->geometry != nullptr);
			display_node.geomList.push_back(std::static_pointer_cast<Geometry>(this->geometry));
		}
	}

	size_t SpineRenderable::getNumIndices()
	{
		return this->numIndices;
	}

	size_t SpineRenderable::getNumVertices()
	{
		return this->numVertices;
	}

	void SpineRenderable::refreshGeometry()
	{
        
		if (!this->instanceLoaded)
		{
			log::info("No instance yet to draw!");
			return;
		}

		int32 num_vertices = 0, num_indices = 0;
		this->instance.getGeometryInfo(num_vertices, num_indices, this->numActiveSprites);
		this->maxActiveSprites = std::max<int32>(this->numActiveSprites, (int32) this->instance.getNumSprites());
        
		if (num_vertices <= 0 || num_indices <= 0)
		{
			log::error("No vertices/indices to draw - skipping geometry creation");
			return;
		}

		this->numIndices = num_indices;

		cs::GeometryDataPtr data = CREATE_CLASS(GeometryData);
		data->decl.addAttrib(AttributeType::AttribPosition, { AttributeType::AttribPosition, TypeFloat, 3, 0 });
		data->decl.addAttrib(AttributeType::AttribTexCoord0, { AttributeType::AttribTexCoord0, TypeFloat, 2, sizeof(vec3) });
		data->decl.addAttrib(AttributeType::AttribColor, { AttributeType::AttribColor, TypeFloat, 4, sizeof(vec3) + sizeof(vec2) });

		data->vertexSize = num_vertices;
		data->indexSize = num_indices;
		data->storage = BufferStorageDynamic;

		uint16 indexCtr = 0;
		int32 useIndexSize = std::max<int32>(this->maxActiveSprites * 6, num_indices);

		data->indexData.resize(useIndexSize);
		for (int32 i = 0; i < this->maxActiveSprites; i++)
		{
			uint16 offset = i * 4;
			data->indexData[indexCtr++] = offset + kStaticQuadIndices[0];
			data->indexData[indexCtr++] = offset + kStaticQuadIndices[1];
			data->indexData[indexCtr++] = offset + kStaticQuadIndices[2];

			data->indexData[indexCtr++] = offset + kStaticQuadIndices[3];
			data->indexData[indexCtr++] = offset + kStaticQuadIndices[4];
			data->indexData[indexCtr++] = offset + kStaticQuadIndices[5];
		}

		this->geometry = CREATE_CLASS(DynamicGeometry, data);

		DynamicGeometry::VertexUpdateFunc vfunc;
		vfunc = std::bind(&SpineRenderable::updateVertices,
			this,
			std::placeholders::_1,
			std::placeholders::_2,
			std::placeholders::_3);
		this->geometry->setVertexUpdateFunc(vfunc);

		DynamicGeometry::IndexUpdateFunc ifunc;
		ifunc = std::bind(&SpineRenderable::updateIndices,
			this,
			std::placeholders::_1,
			std::placeholders::_2);
		this->geometry->setIndexUpdateFunc(ifunc);

		GeometryPtr geoptr = std::static_pointer_cast<Geometry>(this->geometry);

		DynamicGeometry::AdjustDrawCallFunc dcfunc;
		dcfunc = std::bind(&SpineRenderable::setDrawParams,
			this,
			std::placeholders::_1,
			std::placeholders::_2);
		this->geometry->setDrawCallAdjustFunc(dcfunc);

		DynamicGeometry::GetVertexSizeFunc numVFunc = std::bind(&SpineRenderable::getVertexBufferSize, this);
		this->geometry->setVertexBufferSizeFunc(numVFunc);

		DynamicGeometry::GetIndexSizeFunc numIFunc = std::bind(&SpineRenderable::getIndexBufferSize, this);
		this->geometry->setIndexBufferSizeFunc(numIFunc);

		this->geometry->update();
	}

	size_t SpineRenderable::getVertexBufferSize()
	{
		assert(this->geometry.get());
		size_t stride = this->geometry->getGeometryData()->decl.getStride();
		return this->getNumVertices() * stride;
	}

	size_t SpineRenderable::getIndexBufferSize()
	{
		return this->getNumIndices() * sizeof(uint16);
	}

	size_t SpineRenderable::updateVertices(uchar* data, size_t bufferSize, VertexDeclaration& decl)
	{

		uint32 vertex_ctr = 0;
		this->numVertices = 0;

		char* pos_ptr = decl.getAttributePointerAtIndex<char>(data, AttributeType::AttribPosition, 0);
		char* uv_ptr = decl.getAttributePointerAtIndex<char>(data, AttributeType::AttribTexCoord0, 0);
		char* color_ptr = decl.getAttributePointerAtIndex<char>(data, AttributeType::AttribColor, 0);

		size_t stride = decl.getStride();

		memset(data, 0, bufferSize);

		for (size_t i = 0; i < this->instance.getNumSprites(); i++)
		{
			spine::Slot* slot = this->instance.skeleton->value->drawOrder[i];
			if (!slot || !slot->attachment)
			{
				continue;
			}
				
			uint32 vertexCount = 0;
			switch (slot->attachment->type)
			{
				case SP_ATTACHMENT_REGION:
					vertexCount = 4;
					break;
				case SP_ATTACHMENT_MESH:
				{
					spine::MeshAttachment* attachment = reinterpret_cast<spine::MeshAttachment*>(slot->attachment);
					vertexCount = attachment->super.worldVerticesLength / 2;
					break;
				}
				default: 
					assert(false);
			}

			AnimationVertices::iterator it = this->vertexMap.find(slot->data->name);
			if (it == this->vertexMap.end())
			{
				it = this->vertexMap.insert(std::pair<std::string, AnimVertexData>(std::string(slot->data->name), AnimVertexData())).first;
			}
			if (it->second.vertexCount != vertexCount)
			{
				delete[] it->second.vertices;
				it->second.vertices = nullptr;
			}
			if (!it->second.vertices)
			{
				it->second.vertices = new float[vertexCount * 2];
				it->second.vertexCount = vertexCount;
			}

			it->second.offset = vertex_ctr;

			switch (slot->attachment->type)
			{
				case SP_ATTACHMENT_REGION:
				{
					spine::RegionAttachment* attachment = reinterpret_cast<spine::RegionAttachment*>(slot->attachment);

					float32 verts[8];
					spRegionAttachment_computeWorldVertices(attachment, slot->bone, verts, 0, 2);
					
					for (size_t t = 0; t < 4; t++)
					{
						vec3* pos = reinterpret_cast<vec3*>(PTR_ADD(pos_ptr, vertex_ctr * stride));
						*pos = vec3(verts[(t * 2) + 0], verts[(t * 2) + 1], 0.0f);
						
						SpineUtils::applySkew(pos->x, pos->y, this->skew, this->volume);
						SpineUtils::applyScale(pos->x, pos->y, this->scaleX, this->scaleY);

						pos->x = (this->flipHorizontal) ? -pos->x : pos->x;
						pos->y = (this->flipVertical) ? -pos->y : pos->y;

						vec2* uv = reinterpret_cast<vec2*>(PTR_ADD(uv_ptr, vertex_ctr * stride));
						*uv = vec2(attachment->uvs[(t * 2) + 0], attachment->uvs[(t * 2) + 1]);

						memcpy(PTR_ADD(color_ptr, vertex_ctr * stride), (void*)&slot->color, sizeof(vec4));
						vertex_ctr++;
					}

					this->numVertices += 4;
					break;
				}
				case SP_ATTACHMENT_MESH:
				{
					spine::MeshAttachment* attachment = reinterpret_cast<spine::MeshAttachment*>(slot->attachment);
					
					spVertexAttachment_computeWorldVertices(&attachment->super, slot, 0, vertexCount * 2, it->second.vertices, 0, 2);
					for (size_t t = 0; t < vertexCount; t++)
					{
						float32* verts = it->second.vertices;
						float32 x = verts[(t * 2) + 0];
						float32 y = verts[(t * 2) + 1];
						float32 u = attachment->uvs[(t * 2) + 0];
						float32 v = attachment->uvs[(t * 2) + 1];

						SpineUtils::applySkew(x, y, this->skew, this->volume);
						SpineUtils::applyScale(x, y, this->scaleX, this->scaleY);

						vec3* pos = reinterpret_cast<vec3*>(PTR_ADD(pos_ptr, vertex_ctr * stride));
						*pos = vec3(x, y, 0.0f);
						pos->x = (this->flipHorizontal) ? -pos->x : pos->x;
						pos->y = (this->flipVertical) ? -pos->y : pos->y;

						vec2* uv = reinterpret_cast<vec2*>(PTR_ADD(uv_ptr, vertex_ctr * stride));
						*uv = vec2(u, v);
						
						memcpy(PTR_ADD(color_ptr, vertex_ctr * stride), (void*)&slot->color, sizeof(vec4));
						vertex_ctr++;
					}
					this->numVertices += vertexCount;
                    break;
				}
                default:
                    log::info("Unknown attachment type");
                    break;
			}
		}
		
        return vertex_ctr;
	}

	size_t SpineRenderable::updateIndices(uchar* data, size_t bufferSize)
	{
        uint16 indexCtr = 0;
		uint16* indexData = reinterpret_cast<uint16*>(data);
		for (uint16 i = 0; i < this->instance.getNumSprites(); i++)
		{
			spine::Slot* slot = this->instance.skeleton->value->drawOrder[i];
			if (!slot || !slot->attachment)
			{
				continue;
			}
				
			AnimationVertices::iterator it = this->vertexMap.find(slot->data->name);
			assert(it != this->vertexMap.end());

			uint16 offset = it->second.offset;
			switch (slot->attachment->type)
			{
				case SP_ATTACHMENT_REGION:
				{
					assert((offset + kStaticQuadIndices[0]) < uint16(this->getNumVertices()));
					indexData[indexCtr++] = offset + kStaticQuadIndices[0];
					assert((offset + kStaticQuadIndices[1]) < uint16(this->getNumVertices()));
                    indexData[indexCtr++] = offset + kStaticQuadIndices[1];
					assert((offset + kStaticQuadIndices[2]) < uint16(this->getNumVertices()));
                    indexData[indexCtr++] = offset + kStaticQuadIndices[2];

					assert((offset + kStaticQuadIndices[3]) < uint16(this->getNumVertices()));
                    indexData[indexCtr++] = offset + kStaticQuadIndices[3];
					assert((offset + kStaticQuadIndices[4]) < uint16(this->getNumVertices()));
                    indexData[indexCtr++] = offset + kStaticQuadIndices[4];
					assert((offset + kStaticQuadIndices[5]) < uint16(this->getNumVertices()));
                    indexData[indexCtr++] = offset + kStaticQuadIndices[5];
					break;
				}
				case SP_ATTACHMENT_MESH:
				{
					spine::MeshAttachment* attachment = reinterpret_cast<spine::MeshAttachment*>(slot->attachment);
					for (int32 i = 0; i < attachment->trianglesCount; i++)
					{
						uint16 t0 = (uint16) attachment->triangles[i];
						assert(t0 < it->second.vertexCount);
                        assert(size_t(t0 + offset) < this->getNumVertices());
                        
						indexData[indexCtr++] = offset + t0;
					}
                    break;
				}
                default:
                    log::info("Unknown attachment type");
                    break;
			}
		}

		assert(indexCtr <= this->numIndices);
		
		return indexCtr;
	}

	void SpineRenderable::setDrawParams(int32 index, std::vector<DrawCallPtr>& dcs)
	{

		DrawCallPtr dc = CREATE_CLASS(DrawCall);
		dc->tag = this->instance.name;
		dc->type = DrawTriangles;
		dc->indexType = TypeUnsignedShort;
		dc->count = this->numIndices;
		dc->offset = 0;
		dc->shaderHandle = this->shader;
		dc->textures[TextureStageDiffuse] = this->atlasPageRenderTextures[0];  // Kludge!
		dc->color = this->tint;
		dc->cullFace = CullNone;
	
#if !defined(CS_WINDOWS)
		this->options.populate(dc);
#endif

		dcs.push_back(dc);
	}

	void SpineRenderable::onSkinChanged()
	{
		if (!instance.skeletonData || instance.skeletonData->value->skinsCount <= 0)
		{
			return;
		}

		for (int32 i = 0; i < instance.skeletonData->value->skinsCount; ++i)
		{
			spine::Skin* skin = instance.skeletonData->value->skins[i];
			if (skin->name == this->skinName)
			{
				spSkeleton_setSkin(instance.skeleton->value, skin);
				this->refreshGeometry();
				this->instance.playAnimation(this->animName, this->animSpeed, AnimationTypeLoop);
				break;
			}
		}
	}
}
