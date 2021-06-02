#include "PCH.h"

#include "gfx/RenderInterface.h"
#include "gfx/Mesh.h"
#include "gfx/TextureHandle.h"
#include "gfx/Mesh.h"

#include "global/ResourceFactory.h"

#define RENDER_INIT_PLATFORM 1

#define RENDER_INIT_DEFAULT_PARTICLE_EFFECT 1
#define RENDER_INIT_DEFAULT_MESH 1

namespace cs
{
	RenderInterface* RenderInterface::instance = nullptr;
    float RenderInterface::blurPercent = 0.0f;
    float RenderInterface::caStrength = 0.0f;
	const DepthComponent RenderInterface::kDefaultDepthComponent = DepthComponent32;

	TextureHandlePtr RenderInterface::kDefaultTexture = nullptr;
	TextureHandlePtr RenderInterface::kWhiteTexture = nullptr;
	TextureHandlePtr RenderInterface::kErrorTexture = nullptr;
	TextureHandlePtr RenderInterface::kEmptyTexture = nullptr;

    ShaderHandlePtr RenderInterface::kDefaultParticleShader = nullptr;
	ShaderHandlePtr RenderInterface::kDefaultTextureShader = nullptr;
	ShaderHandlePtr RenderInterface::kDefaultTextureShader2 = nullptr;
	ShaderHandlePtr RenderInterface::kDefaultTextureColorShader = nullptr;
	ShaderHandlePtr RenderInterface::kDefaultTextureColorShader2 = nullptr;
	ShaderHandlePtr RenderInterface::kDefaultColorShader = nullptr;
	ShaderHandlePtr RenderInterface::kDefaultDebugShader = nullptr;
	ShaderHandlePtr RenderInterface::kFontAtkasShader = nullptr;
	ShaderHandlePtr RenderInterface::kTextureAlphaShader = nullptr;
	ShaderHandlePtr RenderInterface::kTextureAlphaBWShader = nullptr;
	ShaderHandlePtr RenderInterface::kVertexPhongLit = nullptr;
	ShaderHandlePtr RenderInterface::kVertexPhongLitTexture = nullptr;
	ShaderHandlePtr RenderInterface::kDefaultFontShader = nullptr;
	ShaderHandlePtr RenderInterface::kDefaultFontOutlineShader = nullptr;
    ShaderHandlePtr RenderInterface::kMetalTest = nullptr;
	ShaderHandlePtr RenderInterface::kColorOutline = nullptr;
    ShaderHandlePtr RenderInterface::kSolidColorTest = nullptr;
	ShaderHandlePtr RenderInterface::kTextureSingleChannelShader = nullptr;

	ShaderHandlePtr RenderInterface::kNormalColorLit = nullptr;

	MeshPtr RenderInterface::kCubeMesh = nullptr;

	ParticleEffectHandlePtr RenderInterface::kDefaultParticleEffect = nullptr;

	uint32 RenderInterface::renderStatCounts[RenderStatMAX];
	const char* RenderInterface::kRenderStatStr[] =
	{
		"Draw Calls",
		"Texture Swaps",
		"Primitives"
	};

	void RenderInterface::resetRenderStats()
	{
		memset(RenderInterface::renderStatCounts, 0, RenderStatMAX * sizeof(uint32));
	}

	void RenderInterface::incrementRenderStat(RenderStat stat, uint32 inc)
	{
		RenderInterface::renderStatCounts[stat] += inc;
	}

	uint32 RenderInterface::getRenderStat(RenderStat stat)
	{
		return RenderInterface::renderStatCounts[stat];
	}

	void RenderInterface::setContentScale(float32 scale) 
	{ 
		this->contentScale = scale; 
	}

    void RenderInterface::setBlurPercent(float32 blur)
    {
        if (RenderInterface::blurPercent != blur)
        {
            RenderInterface::blurPercent = blur;
            UniformPtr blur_pct = SharedUniform::getInstance().getUniform("blur_percent");
            assert(blur_pct);
            blur_pct->setValue(RenderInterface::blurPercent);
        }
    }

    void RenderInterface::setChomaticAbberationStrength(float32 ca)
    {
        if (RenderInterface::caStrength != ca)
        {
            RenderInterface::caStrength = ca;
            UniformPtr ca_strength = SharedUniform::getInstance().getUniform("chromatic_abberation_strength");
            assert(ca_strength);
            ca_strength->setValue(RenderInterface::caStrength);
        }
    }
    
    void RenderInterface::getScreenResolution(uint32& width, uint32& height)
    {
        width = this->windowWidth;
        height = this->windowHeight;
    }

    
	void RenderInterface::init(int32 window_width, int32 window_height)
	{
        this->windowWidth = window_width;
        this->windowHeight = window_height;
        
        
		RenderInterface::kDefaultTexture = CREATE_CLASS(TextureHandle, "test_texture.png");
        RenderInterface::kWhiteTexture = CREATE_CLASS(TextureHandle, "white.png");
		RenderInterface::kErrorTexture = CREATE_CLASS(TextureHandle, "missing_texture.png");
		RenderInterface::kEmptyTexture = CREATE_CLASS(TextureHandle, "empty.png");

		SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, "mvp", UniformMat4, 1, ShaderVertex));
		SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, "object", UniformMat4, 1, ShaderVertex));
		SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, "color", UniformVec4, 1, ShaderFragment));
        
        SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, "padding4", UniformVec4, 1, ShaderFragment));
        SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, "padding3", UniformVec3, 1, ShaderFragment));
        SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, "padding2", UniformVec2, 1, ShaderFragment));
        SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, "padding1", UniformFloat, 1, ShaderFragment));
        
		for (uint32 i = 0; i < kMaxLightIndex; ++i)
		{
			std::stringstream lp;
			lp << "light_position" << i;
			SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, lp.str(), UniformVec3, 1, ShaderVertex));
			std::stringstream ld;
			ld << "light_direction" << i;
			SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, ld.str(), UniformVec3, 1, ShaderVertex));
			std::stringstream di;
			di << "diffuse_intensity" << i;
			SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, di.str(), UniformFloat, 1, ShaderFragment));
			std::stringstream ai;
			ai << "ambient_intensity" << i;
			SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, ai.str(), UniformFloat, 1, ShaderFragment));
		}
		
		SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, "camera_position", UniformVec3, 1, ShaderVertex));
		SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, "camera_direction", UniformVec3, 1, ShaderVertex));

		SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, "viewport", UniformVec2, 1, ShaderVertex));
		SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, "viewport_inv", UniformVec2, 1, ShaderVertex));

		SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, "animation_time", UniformFloat, 1, ShaderFragment));
		SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, "animation_time_vtx", UniformFloat, 1, ShaderVertex));
		SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, "animation_pct", UniformFloat, 1, ShaderFragment));
        SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, "chromatic_abberation_strength", UniformFloat, 1, ShaderFragment));
        
        SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, "blur_percent", UniformFloat, 1, ShaderFragment));

		SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, "copy_percent", UniformVec2, 1, ShaderFragment));
		SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, "copy_half_percent", UniformVec2, 1, ShaderFragment));
		SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, "copy_quarter_percent", UniformVec2, 1, ShaderFragment));

		SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, "accelerometer_x", UniformFloat, 1, ShaderFragment));
		SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, "accelerometer_y", UniformFloat, 1, ShaderFragment));
		SharedUniform::getInstance().addUniform(CREATE_CLASS(Uniform, "accelerometer_z", UniformFloat, 1, ShaderFragment));
        
        UniformPtr blur_pct = SharedUniform::getInstance().getUniform("blur_percent");
        assert(blur_pct);
        blur_pct->setValue(0.0f);
        
		MeshMaterialInstance::initUniforms();
		DisplayList::initUniforms();

#if defined(RENDER_INIT_PLATFORM)
		this->initPlatform();
#endif
        
#if defined(RENDER_INIT_DEFAULT_PARTICLE_EFFECT)
		ParticleEffectDataPtr particleData = CREATE_CLASS(ParticleEffectData);
		particleData->lifeTime = CREATE_CLASS(ParticleFloatValueConstant, 1.0f);
		particleData->emissionHandler = CREATE_CLASS(EmissionHandlerInfinite, 10);
		particleData->spawn = CREATE_CLASS(ParticleVec3ValueRandom, vec3(-100.0f, -100.0f, 0.0f), vec3(100.0f, 100.0f, 0.0f));
		particleData->modules.push_back(CREATE_CLASS(ParticleModuleSize, vec2(10.0f, 10.0f)));
		particleData->updateMaxParticles();
		ParticleEffectPtr effect = CREATE_CLASS(ParticleEffect, "default_effect", particleData);
		effect->onPostLoad();
		kDefaultParticleEffect = CREATE_CLASS(ParticleEffectHandle, effect);
		ResourceFactory::getInstance()->addResource<ParticleEffect>(effect);
#endif
        
#if defined(RENDER_INIT_DEFAULT_MESH)
		RenderInterface::kCubeMesh = std::static_pointer_cast<Mesh>(ResourceFactory::getInstance()->loadResource<Mesh>("cube.obj"));
#endif
    }

	void RenderInterface::begin()
	{
        this->frameBuffer = nullptr;
		this->setClearColorImpl();
		this->setBlendFuncImpl();
		
		this->clearBuffer(BufferTypeVertex);
		this->clearBuffer(BufferTypeIndex);

		this->set(StateCullFace, false);
		this->setCullFaceImpl();
		this->setFrontFaceImpl();

		memset(this->renderState, 0, StateTypeMAX * sizeof(bool));
		for (size_t i = 0; i < StateTypeMAX; ++i)
		{
			StateType type = (StateType)i;
			this->set(type, false, true);
		}

		RenderInterface::resetRenderStats();
	}

	void RenderInterface::free()
	{
		kDefaultTextureShader.reset();
		kDefaultColorShader.reset();
		kDefaultDebugShader.reset();
		kFontAtkasShader.reset();
		kDefaultFontShader.reset();
		kDefaultTextureColorShader.reset();

		kDefaultTexture.reset();
		kWhiteTexture.reset();
	
		kCubeMesh.reset();
	}

	void RenderInterface::setClearColor(const ColorF& cc)
	{
		if (cc != this->clearColor) 
		{
			this->clearColor = cc;
			this->setClearColorImpl();
		}
	}

	void RenderInterface::setViewport(const RectI& viewport, bool force)
	{
		if (viewport != this->currentViewport || force) 
		{
			this->currentViewport = viewport;
			this->setViewportImpl();
		}
	}
    
    void RenderInterface::setZ(float32 z_near, float32 z_far)
    {
        if (this->zNear != z_near || this->zFar != z_far)
        {
            this->zNear = z_near;
            this->zFar = z_far;
            this->setZImpl();
        }
    }
    
    BufferObjectPtr RenderInterface::getCurrentBufferBinding(BufferType type)
    {
        return  this->currentBuffers[type];
    }
    
	void RenderInterface::setBuffer(BufferObjectPtr& ptr)
	{
		if (!ptr)
			return;

		BufferType bufferType = ptr->getType();
		if (ptr.get() == this->currentBuffers[bufferType].get() &&
			BufferObject::areBuffersEqual(ptr, this->currentBuffers[bufferType]))
		{
            return;
        }

        this->currentBuffers[bufferType] = ptr;
        if (this->currentBuffers[bufferType])
        {
            this->currentBuffers[bufferType]->bindImpl();
        }
	}

	void RenderInterface::clearBuffer(BufferType type)
	{
		this->clearBufferTypeImpl(type);
		this->currentBuffers[type] = nullptr;
	}

	void RenderInterface::setBlendFunc(BlendType src, BlendType dst)
	{
		if (this->srcBlend != src || this->dstBlend != dst)
		{
			this->srcBlend = src;
			this->dstBlend = dst;
			this->setBlendFuncImpl();
		}
	}

	void RenderInterface::setDepthTestFunc(DepthType df)
	{
        if (!this->allowDepthBuffering)
            return;
        
		if (this->depthFunc != df)
		{
			this->depthFunc = df;
			this->setDepthTestFuncImpl();
		}
	}

	void RenderInterface::setDepthWrite(bool dw)
	{
        if (!this->allowDepthBuffering)
            return;
        
		if (this->depthWrite != dw)
		{
			this->depthWrite = dw;
			this->setDepthWriteImpl();
		}
	}

	void RenderInterface::setLineWidth(float32 lw)
	{
		if (this->lineWidth != lw)
		{
			this->lineWidth = lw;
			this->setLineWidthImpl();
		}
	}

	void RenderInterface::setScissorRect(const RectI& rect)
	{
		if (this->scissorRect != rect)
		{
			this->scissorRect = rect;
			this->setScissorRectImpl();
		}
	}

	void RenderInterface::setCullFace(CullFace cf)
	{
		if (this->cullFace != cf)
		{
			this->cullFace = cf;
			this->setCullFaceImpl();
		}
	}

	void RenderInterface::setFrontFace(FrontFace ff)
	{
		if (this->frontFace != ff)
		{
			this->frontFace = ff;
			this->setFrontFaceImpl();
		}
	}

	void RenderInterface::set(StateType type, bool enabled, bool force)
	{
        if (type == StateDepthTest && !this->allowDepthBuffering)
        {
            return;
        }
        
		if (enabled != this->renderState[type] || force)
		{
			this->renderState[type] = enabled;
			if (enabled)
				this->setEnabledImpl(type);
			else
				this->setDisabledImpl(type);
		}
	}


	TextureHandlePtr RenderInterface::getDefaultTextureForStage(TextureStage stage)
	{
		switch (stage)
		{
			case TextureStageDiffuse:
			case TextureStageSpecular:
			case TextureStageNormal:
			case TextureStageSpecularHighlight:
			case TextureStageAmbient:
				return kDefaultTexture;
		default:

			return kWhiteTexture;
		}

		return kWhiteTexture;
	}

	void RenderInterface::update(float32 dt)
	{
		struct local
		{
			static void updateShaderResources(std::shared_ptr<ShaderResource>& resource, uintptr_t* userData)
			{
				const Uniform::UpdateParams* updateParams = reinterpret_cast<Uniform::UpdateParams*>(userData);
				resource->update(*updateParams);
			}
		};

		Uniform::UpdateParams params;
		params.dt = dt;
		ResourceFactory::getInstance()->performOperation<ShaderResource>(&local::updateShaderResources, (uintptr_t*) &params);
	}
    
    void RenderInterface::captureDefaultFrameBuffer()
    {
        this->captureDefaultFrameBufferImpl();
    }
    
    void RenderInterface::setDefaultFrameBuffer()
    {
        this->setDefaultFrameBufferImpl();
        this->allowDepthBuffering = true;
    }
    
    void RenderInterface::setFrameBuffer(const std::string& tag, FrameBufferPtr& frameBuffer, bool isMainScreen, bool forceFrameBuffer, const std::vector<ClearMode>* clearParams)
    {
        if (!frameBuffer.get())
        {
            this->clearFrameBuffer();
            return;
        }
        
        if (this->currentFrameBuffer.get() != frameBuffer.get() || forceFrameBuffer)
        {
            this->currentFrameBuffer = frameBuffer;
            this->currentFrameBuffer->bind();
            bool shouldDepthTest = this->currentFrameBuffer->getDepthBuffer().get() != nullptr;
            if (!shouldDepthTest)
            {
                this->set(StateDepthTest, false);
                this->setDepthWrite(false);
                this->setDepthTestFunc(DepthNever);
                this->allowDepthBuffering = false;
            }
        }
    }
    
    void RenderInterface::clearFrameBuffer()
    {
        if (this->currentFrameBuffer)
        {
            this->currentFrameBuffer->unbind();
        }
        this->currentFrameBuffer = 0;
    }
}
