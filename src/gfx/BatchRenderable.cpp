#include "PCH.h"

#include "gfx/BatchRenderable.h"
#include "global/ResourceFactory.h"
namespace cs
{

	const float32 kLayerDepthAdjust = 0.01f;
	
	BEGIN_META_CLASS(BatchRenderable)

		ADD_MEMBER_PTR(shape);
			ADD_COMBO_META_LABEL(QuadShape, "Quad");
			ADD_COMBO_META_LABEL(ParallelogramShape, "Parallelogram");
			ADD_COMBO_META_LABEL(GridShape, "Grid");
			ADD_COMBO_META_LABEL(CircleShape, "Circle");
			ADD_COMBO_META_LABEL(EllipseShape, "Ellipse");
			SET_MEMBER_CALLBACK_POST(&BatchRenderable::onShapeChanged);

		ADD_MEMBER_PTR(texture);
			SET_MEMBER_COLLAPSEABLE();
			SET_MEMBER_START_COLLAPSED();
			SET_MEMBER_CALLBACK_POST(&BatchRenderable::resetTexture);

		ADD_MEMBER_PTR(shader);
			SET_MEMBER_COLLAPSEABLE();
			SET_MEMBER_START_COLLAPSED();

		ADD_MEMBER(blend);
			SET_MEMBER_COLLAPSEABLE();
			SET_MEMBER_START_COLLAPSED();

		ADD_MEMBER(worldUV);
			SET_MEMBER_IGNORE_GUI();
		
		ADD_MEMBER(worldUVScale);
			SET_MEMBER_DEFAULT(kOne2);
		
		ADD_MEMBER(tint);
			SET_MEMBER_DEFAULT(ColorB::White);

		ADD_MEMBER(culling);
			SET_MEMBER_DEFAULT(true);

		ADD_META_FUNCTION("Set World UV", &BatchRenderable::setWorldUV);
		ADD_META_FUNCTION("Clear World UV", &BatchRenderable::clearWorldUV);
		ADD_META_FUNCTION("Match Texture Size", &BatchRenderable::mapToTexture);
		ADD_META_FUNCTION("Match Texture Size Half", &BatchRenderable::mapToTextureHalf);
		ADD_META_FUNCTION("Set Shadow Params", &BatchRenderable::setAsShadow);
	}

	BatchRenderable::BatchRenderable()
		: shape()
		, texture(CREATE_CLASS(TextureHandle, RenderInterface::kWhiteTexture))
		, animRect(nullptr)
		, shader(CREATE_CLASS(ShaderHandle, RenderInterface::kDefaultTextureShader))
		, worldUVScale(kOne2)
		, drawData(CREATE_CLASS(BatchDrawData))
		, tint(ColorB::White)
		, textureAtlas(nullptr)
		, culling(true)
		, flags(0)
	{
	
	}

	void BatchRenderable::onNew()
	{
		this->shape = CREATE_CLASS(QuadShape);
		if (this->shape)
		{
			this->shape->onChanged += createCallbackArg0(&BatchRenderable::refresh, this);
		}

		BASECLASS::onNew();
	}

	BatchRenderable::~BatchRenderable()
	{
		if (this->animRect)
		{
			delete this->animRect;
			this->animRect = nullptr;
		}
	}

	void BatchRenderable::onShapeChanged()
	{
		if (this->shape)
		{
			this->shape->onChanged += createCallbackArg0(&BatchRenderable::refresh, this);
		}
		this->refresh();
	}

	void BatchRenderable::refresh()
	{
		this->setGeometryImpl();
		this->setTextureImpl();
	}

	void BatchRenderable::setTextureAtlasImpl()
	{
	
		if (this->texture && this->texture->getIsAtlased())
		{
			if (this->textureAtlas.get())
				return;

			TextureHandlePtr copyTex = CREATE_CLASS(TextureHandle, this->texture);
			copyTex->setFlipHorizontal(false);
			copyTex->setFlipVertical(false);
			this->textureAtlas = TextureAtlasManager::getInstance()->getTextureAtlas(copyTex);
			assert(this->textureAtlas.get());

			// Set a callback for when the atlas completes
			if (!this->textureAtlas->resultHandle.get())
			{
				this->textureAtlas->data->onAtlased += createCallbackArg0(&BatchRenderable::refresh, this);
			}
		}
		else
		{
			this->textureAtlas = nullptr;
			this->atlasHandle = nullptr;
		}
	}

	void BatchRenderable::onPostLoad(const LoadFlagMask& flags)
	{
		if (this->texture)
		{
			this->texture->onPostLoad(flags);
			this->texture->onChanged += createCallbackArg0(&BatchRenderable::setTextureImpl, this);
		}
		else
		{
			this->texture = CREATE_CLASS(TextureHandle, RenderInterface::kErrorTexture);
		}

		if (this->shader)
		{
			this->shader->onPostLoad(flags);
			this->shader->onChanged += createCallbackArg0(&BatchRenderable::setShaderImpl, this);
		}
		else
		{
			// There was an error loading the shader - set to default
			this->shader = CREATE_CLASS(ShaderHandle, RenderInterface::kDefaultTextureShader);
		}

		if (this->shape)
		{
			this->shape->onChanged += createCallbackArg0(&BatchRenderable::setGeometryImpl, this);
		}

		this->refresh();
	}

	void BatchRenderable::getSelectableVolume(SelectableVolumeList& selectable_volumes)
	{
		SelectableVolume volume;
		volume.volume = this->volume;
		volume.type = SelectableVolumeTypeDraw;

		selectable_volumes.push_back(volume);
	}

	void BatchRenderable::setGeometryImpl()
	{
		this->drawData->positions.clear();
		this->drawData->indices.clear();

		if (this->animShape.get())
		{
			this->animShape->generatePositions(this->drawData->positions);
			this->animShape->generateIndices(this->drawData->indices);
		}
		else
		{
			this->shape->generatePositions(this->drawData->positions);
			this->shape->generateIndices(this->drawData->indices);
		}
		this->volume = this->shape->createVolume();
	}

	void BatchRenderable::adjustSize(const vec2& sz, AnimationScaleType type)
	{

		switch (type)
		{
			case AnimationScaleTypeCenter:
			{
				float32 x = 0.0f;
				float32 y = 0.0f;
				if (this->volume.get())
				{
					RectF rect = this->volume->getRect();
					PointF c = rect.getCenter();
					x = c.x;
					y = c.y;
				}

				x = (sz.x > 0) ? x - sz.x * 0.5f : x;
				y = (sz.y > 0) ? y - sz.y * 0.5f : y;
				this->animShape = CREATE_CLASS(QuadShape, x, y, fabs(sz.x), fabs(sz.y));
				break;
			}
			case AnimationScaleTypeLeft:
			{
				float32 x = 0.0f;
				float32 y = 0.0f;
				if (this->volume.get())
				{
					RectF rect = this->volume->getRect();
					x = rect.pos.x;
					y = rect.pos.y;
				}

				x = (sz.x > 0) ? x : x + sz.x;
				y = (sz.y > 0) ? y : y + sz.y;
				this->animShape = CREATE_CLASS(QuadShape, x, y, fabs(sz.x), fabs(sz.y));
				break;
			}
			default:
				assert(false);
				
		}
			

		this->setGeometryImpl();
	}

	void BatchRenderable::adjustTexRect(const RectF& rect)
	{
		if (this->texture.get())
		{
			if (!this->animRect)
			{
				this->animRect = new RectF();
			}
			*this->animRect = rect;
			this->setUVImpl();
		}
	}

	VolumePtr BatchRenderable::getVolume()
	{
		if (!this->volume.get())
		{
			if (this->shape.get())
			{
				this->volume = this->shape->createVolume();
			}
		}
		return this->volume;
	}

	void BatchRenderable::setTexture(const TextureHandlePtr& tex)
	{
		if (this->texture)
		{
			this->texture->onChanged.clear();
		}

		this->texture = tex;
		
		if (this->texture)
		{
			this->setTextureImpl();
			this->texture->onChanged += createCallbackArg0(&BatchRenderable::setTextureImpl, this);
		}
	}

	void BatchRenderable::setTexture(const std::string& textureName)
	{
		TextureHandlePtr texture_handle = CREATE_CLASS(TextureHandle, textureName);
		this->setTexture(texture_handle);
	}

	void BatchRenderable::setShaderImpl()
	{
		this->shader->ignoreTextureStage(TextureStageDiffuse);
		this->drawData->shader = this->shader;
	}


	void BatchRenderable::mapToTextureImpl(float32 scale)
	{
		if (this->texture.get())
		{
			uint32 w = uint32(this->texture->getWidth() * scale);
			uint32 h = uint32(this->texture->getHeight() * scale);

			RectF szRect(w * -0.5f, h * -0.5f, float32(w), float32(h));
			this->shape = CREATE_CLASS(QuadShape, szRect);
			this->onShapeChanged();
		}
	}

	void BatchRenderable::mapToTexture()
	{
		this->mapToTextureImpl(1.0f);
	}

	void BatchRenderable::mapToTextureHalf()
	{
		this->mapToTextureImpl(0.5f);
	}

	void BatchRenderable::resetTexture()
	{
		this->textureAtlas = nullptr;
		this->atlasHandle = nullptr;
		this->setTextureImpl();
	}

	void BatchRenderable::setTextureImpl()
	{
		this->setTextureAtlasImpl();
		if (this->worldUV.size() > 0)
		{
			this->drawData->uvs0 = this->worldUV;
		}
		else
		{
			this->setUVImpl();
		}
		this->drawData->texture[0] = this->getTextureHandleImpl();
	}

	TextureHandlePtr& BatchRenderable::getTextureHandleImpl()
	{
		if (this->textureAtlas.get() && this->textureAtlas->resultHandle.get())
		{
			// need to create a copy of this texture handle and apply any flips we may need
			if (!this->atlasHandle.get())
			{
				this->atlasHandle = CREATE_CLASS(TextureHandle, this->textureAtlas->resultHandle);
				this->atlasHandle->setFlipHorizontal(this->texture->getFlipHorizontal());
				this->atlasHandle->setFlipVertical(this->texture->getFlipVertical());
			}
			return this->atlasHandle;
		}
		return this->texture;
	}

	void BatchRenderable::setUVImpl(float32 dt)
	{
		if (this->shape)
		{
			static RectF kDefaultBoxRect(0.0f, 0.0f, 1.0f, 1.0f);

			this->drawData->uvs0.clear();
            RectF toRect = (this->animRect) ? *this->animRect : this->getTextureHandleImpl()->getUVRect(dt);
            
#if defined(CS_METAL)
            if (this->atlasHandle.get())
            {
                toRect.pos.y = 1.0f - toRect.pos.y;
                toRect.size.h = -toRect.size.h;
            }
#endif
            
			this->shape->generateUV(toRect, this->drawData->uvs0);
			this->shape->generateUV(kDefaultBoxRect, this->drawData->uvs1);

			this->shape->generateColors(this->drawData->vcolors, this->tint);
		}
	}

	void BatchRenderable::clearWorldUV()
	{
		this->worldUV.clear();
		this->setTextureImpl();
	}

	void BatchRenderable::setWorldUV()
	{
		this->worldUV.clear();
		for (auto& it : this->drawData->positions)
		{
			this->worldUV.push_back(vec2(it.x * this->worldUVScale.x, it.y * this->worldUVScale.y));
		}
		this->drawData->uvs0 = this->worldUV;
	}

	void BatchRenderable::setShader(const std::string& shaderName)
	{
		ShaderResourcePtr shaderResource = std::static_pointer_cast<ShaderResource>(
			ResourceFactory::getInstance()->loadResource<ShaderResource>(shaderName));
		if (shaderResource.get())
		{
			ShaderHandlePtr shaderHandle = CREATE_CLASS(ShaderHandle, shaderResource);
			this->setShader(shaderHandle);
		}
	}

	void BatchRenderable::setShader(const ShaderHandlePtr& shader)
	{
		this->shader = shader;
		this->drawData->shader = this->shader;

		if (this->shader)
		{
			this->shader->onChanged += createCallbackArg0(&BatchRenderable::setShaderImpl, this);
		}
	}


	float32 BatchRenderable::updateDepth(const mat4& objToWorld, const RectF& orthoRect, SortMethod sortMethod)
	{
		switch (sortMethod)
		{
			case SortMethodY:
			{
				vec4 bottom(0.0f, this->volume->getRect().getBottom(), 0.0f, 1.0f);
				vec4 worldBottom = objToWorld * bottom;
				float32 orthoBottom = orthoRect.getBottom();
				float32 pct = 1.0f - std::min(1.0f, (worldBottom.y - orthoBottom) / float32(orthoRect.size.h));
				return pct;
			}
			default:
				break;
		}

		return 0.0f;
	}

	void BatchRenderable::batch(
		const std::string& tag,
		BatchDrawList& display_list,
		const BatchRenderableParams& params,
		uint32& numVertices, 
		uint16& numIndices)
	{

		if (!this->isVisible() || this->tint.a == 0)
			return;

		if (this->texture.get() && this->texture->hasAnimation())
		{
			this->setUVImpl(params.dt);
		}

		mat4 objToWorld = params.transform.getCurrentMatrix();

		if (this->culling)
		{
			RectF volRect = this->volume->getRect();

			vec4 transVec(volRect.pos.x, volRect.pos.y, 0.0f, 1.0f);
			vec4 trans = objToWorld * transVec;
			volRect.pos.x = trans.x;
			volRect.pos.y = trans.y;

			if (!RectF::contains(params.orthoRect, volRect))
				return;
		}

		display_list.push_back(BatchDrawParams(this->drawData, numVertices));
		BatchDrawParams& drawParams = display_list.back();
		drawParams.transform = objToWorld;
		drawParams.tint = params.parentColor * this->tint;
		drawParams.layer = this->layer;
		drawParams.tag = tag;
		drawParams.bounds = this->volume->getRect();
		drawParams.depth = params.sortMethod == SortMethodY;
		drawParams.depthValue = this->updateDepth(objToWorld, params.orthoRect, params.sortMethod);
		drawParams.blend = this->blend;
		drawParams.flags = this->flags;

        //this->drawData->texture[0] = RenderInterface::kDefaultTexture;
		this->drawData->shader = this->shader;

		numVertices += static_cast<uint32>(this->drawData->positions.size());
		numIndices += static_cast<uint16>(this->drawData->indices.size());
	}


	void BatchRenderable::setTint(const ColorB& col)
	{
		this->tint = col;
	}

	const ColorB& BatchRenderable::getTint() const
	{
		return this->tint;
	}

	void BatchRenderable::setAsShadow()
	{
		const ColorB kShadowTint(128, 128, 128, 255);
		this->setShader("texColorAlpha");
		this->setTint(kShadowTint);
	}

	void BatchRenderable::setFlag(RenderableOptions flag, bool enabled) 
	{ 
		if (enabled)
		{
			this->flags |= (0x1 << int32(flag));
		}
		else
		{
			this->flags &= ~(0x1 << int32(flag));
		}
	}
}
