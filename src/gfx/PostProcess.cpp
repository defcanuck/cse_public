#include "PCH.h"

#include "gfx/PostProcess.h"
#include "geom/Shape.h"
#include "gfx/Renderer.h"

namespace cs
{
	PostProcess::PostProcess(const std::string& name, const PostProcessParams& params)
		: postName(name)
		, postParams(params)
		, textureOutput(nullptr)
		, orthoOffset(0.0f, 0.0f)
		, orthoScale(1.0f, 1.0f)
		, topSkew(1.0f)
		, botSkew(1.0f)
		, tint(ColorB::White)
	{

	}

	void PostProcess::mapInputs(RenderTextureMap& inputs)
	{
		for (auto input : this->postParams.inputMap)
		{
			TextureStage stage = input.targetStage;
			RenderTextureMap::iterator it = inputs.find(input.targetBuffer);
			if (it != inputs.end())
			{
				if (input.depth)
				{
					this->textureInputs[stage] = CREATE_CLASS(TextureHandle, it->second->getDepthTexture());
				}
				else
				{
					this->textureInputs[stage] = CREATE_CLASS(TextureHandle, it->second->getTexture());
				}

				if (this->postParams.flipHorizontal)
				{
					this->textureInputs[stage]->setFlipHorizontal(true);
				}

				if (this->postParams.flipVertical)
				{
					this->textureInputs[stage]->setFlipVertical(true);
				}

				if (this->postParams.fillRect.isZero() && input.targetStage == TexturePostColor)
				{
					this->postParams.fillRect = this->textureInputs[TexturePostColor]->getUVRect();
				}
			}
			else
			{
				log::error("Cannot find input for target buffer ", input.targetBuffer, " at stage ", input.targetStage);
			}
		}
	}

	void PostProcess::setTint(const ColorB& tint)
	{
		this->tint = tint;
		this->resolveDraw->color = this->tint;
	}

	void PostProcess::setShader(const std::string& shaderName)
	{
        if (!this->postParams.shader.get() ||
            this->postParams.shader->getName() != shaderName)
        {
            this->postParams.shader = CREATE_CLASS(ShaderHandle, shaderName);
            if (this->resolveDraw.get())
            {
                this->resolveDraw->shaderHandle = this->postParams.shader;
            }
        }
	}

	void PostProcess::mapOutputs(RenderTextureMap& outputs)
	{
		TextureStage stage = postParams.outputMap.targetStage;
		RenderTextureMap::iterator it = outputs.find(postParams.outputMap.targetBuffer);
		if (it != outputs.end())
		{
			this->textureOutput = it->second;
		}
	}

	void PostProcess::resolve(DisplayList& display_list)
	{
		if (!this->resolveGeom)
		{
			this->initGeometry();
		}


		DisplayListPassPtr post_pass = display_list.addPass(this->postName, this->postParams.traversal);
		post_pass->target = this->textureOutput;
		post_pass->targetViewport = this->postParams.viewport;

		post_pass->clearColor = this->postParams.clearColor;
		post_pass->clearModes = this->postParams.clearModes;

		DisplayListNode node;
		node.tag = "postprocess-" + this->postName;
		node.mvp = glm::ortho(
			float32(this->postParams.viewport.pos.x),
			float32(this->postParams.viewport.pos.x + this->postParams.viewport.size.w),
			float32(this->postParams.viewport.pos.y),
			float32(this->postParams.viewport.pos.y + this->postParams.viewport.size.h),
			-1.0f, 1.0f);

		this->resolveDraw->shaderHandle = this->postParams.shader;

		node.geomList.push_back(this->resolveGeom);
		node.color = toVec4(this->tint);

		post_pass->nodes.push_back(node);

		if (this->postParams.callback)
		{
			this->postParams.callback(post_pass->targetViewport, post_pass->target);
		}
	}

	void PostProcess::update()
	{
		if (this->postParams.isDynamic)
		{
			this->resolveGeom->update();
		}
	}

	void PostProcess::draw(const RectI& viewport)
	{
		if (!this->resolveGeom)
		{
			this->initGeometry();
		}

		RenderInterface* rend = RenderInterface::getInstance();

		mat4 mvp;
        rend->setZ(-1.0f, 1.0f);
        if (this->textureOutput.get())
		{
			rend->setViewport(this->postParams.viewport);
			mvp = glm::ortho(
				float32(this->postParams.viewport.pos.x),
				float32(this->postParams.viewport.pos.x + this->postParams.viewport.size.w),
				float32(this->postParams.viewport.pos.y),
				float32(this->postParams.viewport.pos.y + this->postParams.viewport.size.h),
				-1.0f, 1.0f);
		}
		else
		{
			rend->setViewport(viewport);
			mvp = glm::ortho(
				float32(viewport.pos.x),
				float32(viewport.pos.x + viewport.size.w),
				float32(viewport.pos.y),
				float32(viewport.pos.y + viewport.size.h),
				-1.0f, 1.0f);
		}

		renderer::draw(mvp, ColorB::White, this->resolveGeom);
	}

	size_t PostProcess::updateVertices(uchar* data, size_t bufferSize, VertexDeclaration& decl)
	{
		size_t stride = this->resolveGeom->getGeometryData()->decl.getStride();

		{
			void* pos_ptr =
				this->resolveGeom->getGeometryData()->decl.getAttributePointerAtIndex<char>(data, AttributeType::AttribPosition, 0);

			RectF copyViewport;
			copyViewport.size.w = float32(this->postParams.viewport.size.w);
			copyViewport.size.h = float32(this->postParams.viewport.size.h);
			copyViewport.pos.x = float32(this->postParams.viewport.pos.x);
			copyViewport.pos.y = float32(this->postParams.viewport.pos.y);

			PointF scaleFactor(this->orthoScale.x, this->orthoScale.y);
			PointF scaleCenter(
				float32(this->postParams.viewport.size.w), 
				float32(this->postParams.viewport.size.h));

			copyViewport.scaleCenterRelative(scaleFactor, scaleCenter);
			copyViewport.offset(PointF(this->orthoOffset.x * copyViewport.size.w, this->orthoOffset.y * copyViewport.size.h));
            
            // Clamp Left
            if (copyViewport.pos.x > this->postParams.viewport.pos.x)
            {
                float xOff = this->postParams.viewport.pos.x - copyViewport.pos.x;
                copyViewport.offset(PointF(xOff, 0.0f));
            }
            
            // Clamp Right
            if ((copyViewport.pos.x + copyViewport.size.w) < (this->postParams.viewport.pos.x + this->postParams.viewport.size.w))
            {
                float xOff =
                    (this->postParams.viewport.pos.x + this->postParams.viewport.size.w) -
                    (copyViewport.pos.x + copyViewport.size.w);
                copyViewport.offset(PointF(xOff, 0.0f));
            }
            
			vec2 bl_pos = toVec2(copyViewport.getBL());
			vec2 tl_pos = toVec2(copyViewport.getTL());
			vec2 tr_pos = toVec2(copyViewport.getTR());
			vec2 br_pos = toVec2(copyViewport.getBR());

			if (!fuzzyCompare(this->topSkew, 1.0f, 0.0001f) ||
				!fuzzyCompare(this->botSkew, 1.0f, 0.0001f))
			{
				
				float top_width = (tr_pos.x - tl_pos.x) * 0.5f;
				float bot_width = (br_pos.x - bl_pos.x) * 0.5f;

				float top_center = tl_pos.x + top_width;
				float bot_center = bl_pos.x + bot_width;

				tl_pos.x = top_center - (top_width * this->topSkew);
				tr_pos.x = top_center + (top_width * this->topSkew);

				bl_pos.x = bot_center - (bot_width * this->botSkew);
				br_pos.x = bot_center + (bot_width * this->botSkew);

			}

			memcpy(pos_ptr, (void*)&bl_pos, sizeof(vec2));
			pos_ptr = PTR_ADD(pos_ptr, stride);

			memcpy(pos_ptr, (void*)&tl_pos, sizeof(vec2));
			pos_ptr = PTR_ADD(pos_ptr, stride);

			memcpy(pos_ptr, (void*)&tr_pos, sizeof(vec2));
			pos_ptr = PTR_ADD(pos_ptr, stride);

			memcpy(pos_ptr, (void*)&br_pos, sizeof(vec2));
		}

		{
			vec2 bl_uv = toVec2(this->postParams.fillRect.getBL());
			vec2 tl_uv = toVec2(this->postParams.fillRect.getTL());
			vec2 tr_uv = toVec2(this->postParams.fillRect.getTR());
			vec2 br_uv = toVec2(this->postParams.fillRect.getBR());

			void* uv_ptr =
				this->resolveGeom->getGeometryData()->decl.getAttributePointerAtIndex<char>(data, AttributeType::AttribTexCoord0, 0);

			memcpy(uv_ptr, (void*)&bl_uv, sizeof(vec2));
			uv_ptr = PTR_ADD(uv_ptr, stride);

			memcpy(uv_ptr, (void*)&tl_uv, sizeof(vec2));
			uv_ptr = PTR_ADD(uv_ptr, stride);

			memcpy(uv_ptr, (void*)&tr_uv, sizeof(vec2));
			uv_ptr = PTR_ADD(uv_ptr, stride);

			memcpy(uv_ptr, (void*)&br_uv, sizeof(vec2));
		}

		return this->getVertexBufferSize();
	}

	size_t PostProcess::updateIndices(uchar* data, size_t bufferSize)
	{
		memcpy(data, kStaticQuadIndices, 6 * sizeof(uint16));
		return this->getIndexBufferSize();
	}

	void PostProcess::setDrawParams(int32 index, std::vector<DrawCallPtr>& dcs)
	{
		dcs.push_back(this->resolveDraw);
	}

	size_t PostProcess::getVertexBufferSize()
	{
		assert(this->resolveGeom.get());
		size_t stride = this->resolveGeom->getGeometryData()->decl.getStride();
		return 4 * stride;
	}

	size_t PostProcess::getIndexBufferSize()
	{
		return 6 * sizeof(uint16);
	}

	void PostProcess::initGeometry()
	{

		GeometryDataPtr data = this->initGeometryData();

		if (this->postParams.isDynamic)
		{
			this->initDynamicGeometry(data);
		}
		else
		{
			this->initStaticGeometry(data);
		}
	}

	void PostProcess::initDynamicGeometry(GeometryDataPtr& data)
	{
		DynamicGeometryPtr resolveDynamicGeom = CREATE_CLASS(DynamicGeometry, data);

		this->resolveGeom = resolveDynamicGeom;

		DynamicGeometry::VertexUpdateFunc vfunc;
		vfunc = std::bind(&PostProcess::updateVertices,
			this,
			std::placeholders::_1,
			std::placeholders::_2,
			std::placeholders::_3);
		resolveDynamicGeom->setVertexUpdateFunc(vfunc);

		DynamicGeometry::IndexUpdateFunc ifunc;
		ifunc = std::bind(&PostProcess::updateIndices,
			this,
			std::placeholders::_1,
			std::placeholders::_2);
		resolveDynamicGeom->setIndexUpdateFunc(ifunc);

		DynamicGeometry::AdjustDrawCallFunc dcfunc;
		dcfunc = std::bind(&PostProcess::setDrawParams,
			this,
			std::placeholders::_1,
			std::placeholders::_2);
		resolveDynamicGeom->setDrawCallAdjustFunc(dcfunc);

		DynamicGeometry::GetVertexSizeFunc numVFunc = std::bind(&PostProcess::getVertexBufferSize, this);
		resolveDynamicGeom->setVertexBufferSizeFunc(numVFunc);

		DynamicGeometry::GetIndexSizeFunc numIFunc = std::bind(&PostProcess::getIndexBufferSize, this);
		resolveDynamicGeom->setIndexBufferSizeFunc(numIFunc);

		resolveDynamicGeom->update();
	}

	GeometryDataPtr PostProcess::initGeometryData()
	{
		GeometryDataPtr data = CREATE_CLASS(GeometryData);
		const float kDefaultGeomDimm = 0.5;

		std::vector<vec2> pos;
		pos.push_back(toVec2(this->postParams.viewport.getBL()));
		pos.push_back(toVec2(this->postParams.viewport.getTL()));
		pos.push_back(toVec2(this->postParams.viewport.getTR()));
		pos.push_back(toVec2(this->postParams.viewport.getBR()));

		assert(!this->postParams.fillRect.isZero());
		std::vector<vec2> uvs =
		{
			toVec2(this->postParams.fillRect.getBL()),
			toVec2(this->postParams.fillRect.getTL()),
			toVec2(this->postParams.fillRect.getTR()),
			toVec2(this->postParams.fillRect.getBR())
		};

		data->vertexSize = 4;
		for (int i = 0; i < 4; i++)
		{
			// pos + texture dada
			data->pushVertexData(pos[i]);
			data->pushVertexData(uvs[i]);
		}

		data->indexSize = 6;
		data->indexData.push_back(kStaticQuadIndices[0]);
		data->indexData.push_back(kStaticQuadIndices[1]);
		data->indexData.push_back(kStaticQuadIndices[2]);
		data->indexData.push_back(kStaticQuadIndices[3]);
		data->indexData.push_back(kStaticQuadIndices[4]);
		data->indexData.push_back(kStaticQuadIndices[5]);

		data->decl.addAttrib(AttribPosition, { AttribPosition, TypeFloat, 2, 0 });
		data->decl.addAttrib(AttribTexCoord0, { AttribTexCoord0, TypeFloat, 2, sizeof(vec2) });

		this->resolveDraw = CREATE_CLASS(DrawCall);
		this->resolveDraw->type = DrawTriangles;
		this->resolveDraw->offset = 0;
		this->resolveDraw->count = 6;
		this->resolveDraw->textures = this->textureInputs;
		this->resolveDraw->shaderHandle = this->postParams.shader;
		this->resolveDraw->indexType = TypeUnsignedShort;
		this->resolveDraw->indices = data->indexData;
		this->resolveDraw->color = this->tint;
		this->resolveDraw->depthTest = false;
        this->resolveDraw->cullFace = CullNone;
        
		data->drawCalls.push_back(this->resolveDraw);

		return data;
	}

	void PostProcess::initStaticGeometry(GeometryDataPtr& data)
	{
		this->resolveGeom = CREATE_CLASS(cs::Geometry, data);
	}

	void PostProcess::setTexture(TextureHandlePtr& handle, TextureStage stage)
	{
		if (!this->resolveGeom)
		{
			this->initGeometry();
		}
		this->resolveDraw->textures[stage] = handle;
	}

	void PostProcess::setOutput(RenderTexturePtr& out)
	{
		textureOutput = out;
	}

	void PostProcess::revertTexture(TextureStage stage)
	{
		if (!this->resolveDraw)
			return;

		TextureStages::iterator it = this->textureInputs.find(stage);
		if (it == this->textureInputs.end())
			return;

		this->resolveDraw->textures[stage] = it->second;
	}


}
