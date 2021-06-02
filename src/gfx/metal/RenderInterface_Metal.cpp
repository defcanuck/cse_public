#include "PCH.h"

#include "gfx/RenderInterface.h"
#include "gfx/metal/RenderInterface_Metal.h"
#include "os/FileManager.h"
#include "global/ResourceFactory.h"
#include "global/Utils.h"
#include "gfx/Mesh.h"
#include "gfx/ShaderUtils.h"

#include "gfx/metal/Shader_Metal.h"
#include "gfx/metal/ShaderProgram_Metal.h"
#include "gfx/metal/BufferObject_Metal.h"
#include "gfx/metal/MetalTest.h"
#include "gfx/metal/TextureResource_Metal.h"
#include "gfx/metal/FrameBuffer_Metal.h"
#include "gfx/metal/DepthBuffer_Metal.h"

#include "MTLRenderInterface.h"

#include <memory>
#include <assert.h>

// #define LOG_METAL_BIND_ERRORS 1

#if defined(LOG_METAL_BIND_ERRORS)
    #define PrintMetalBindError(...) log::error(__VA_ARGS__)
#else
    #define PrintMetalBindError(...) void(0)
#endif

namespace cs
{

    namespace RenderInterface_MetalUtils
    {
        uint32 getClearMask(const std::vector<ClearMode>& clearParams)
        {
            uint32 currentClear = 0;
            for (auto& it : clearParams)
            {
                currentClear |= (0x1 << uint32(it));
            }
            return currentClear;
        }
    }

	RenderInterface_Metal::RenderInterface_Metal()
		: defaultFrameBuffer(0)
        , currentEncoder(nullptr)
        , currentChannels(TextureBGRA)
        , currentDepth(TextureDepth32)
        , debugScope(0)
        , screenClearColor(ColorF::Black)
	{

	}

#if defined(CS_METAL)
	RenderInterface* RenderInterface::getInstance()
	{
		if (!instance)
			instance = new RenderInterface_Metal();

		return instance;
	}
#endif
    
    void RenderInterface_Metal::beginFrame()
    {
        MTLBeginFrame();
        
        this->setClearColor(this->screenClearColor);
        this->currentChannels = TextureBGRA;
        this->currentDepth = TextureDepth32;
        
    }
    
    void RenderInterface_Metal::endFrame()
    {
        if (this->currentEncoder)
        {
            // push the last encoder
            MTLPushEncoder(this->currentEncoder);
            this->currentEncoder = nullptr;
        }
        MTLEndFrame();
    }
    
    void RenderInterface_Metal::testFrame()
    {
        MetalTest::render();
    }
    
	TextureResourcePtr RenderInterface_Metal::loadTexture(const std::string& filePath)
	{
		// return std::shared_ptr<TextureResource>(new TextureResource_OpenGL(filePath));
        return std::shared_ptr<TextureResource>(new TextureResource_Metal(filePath));
	}

	TextureResourcePtr RenderInterface_Metal::loadTexture(const Dimensions& dimm, TextureChannels channels, uchar* bytes, TextureUsage usage)
	{
		//return std::shared_ptr<TextureResource>(new TextureResource_OpenGL(dimm, channels, bytes));
        return std::shared_ptr<TextureResource>(new TextureResource_Metal(dimm, channels, bytes, usage));
	}

	ShaderPtr RenderInterface_Metal::loadShader(ShaderType type, const std::string& fileName, bool printSource)
	{
        ShaderPtr shader = std::static_pointer_cast<Shader>(CREATE_CLASS(Shader_Metal, type));
        if (fileName.length() > 0)
        {
            // todo
            shader->compile(fileName, "", printSource);
        }
        return shader;
	}

	ShaderProgramPtr RenderInterface_Metal::createShaderProgram()
	{
		return std::shared_ptr<ShaderProgram>(new ShaderProgram_Metal());
    }

	BufferObjectPtr RenderInterface_Metal::createBufferObject(BufferType type)
	{
        return std::shared_ptr<BufferObject>(new BufferObject_Metal(type));
	}

	VertexArrayObjectPtr RenderInterface_Metal::createVertexArrayObject()
	{
		// return std::shared_ptr<VertexArrayObject>(new VertexArrayObject_OpenGL());
        return std::shared_ptr<VertexArrayObject>();
	}

	void RenderInterface_Metal::initPlatform()
	{
        MTLSetScreenResolution(this->windowWidth, this->windowHeight);
        ShaderUtils::addDefaultShaders();
	}

	void RenderInterface_Metal::setClearColorImpl()
	{
		MTLSetClearColor(this->clearColor.r, this->clearColor.g, this->clearColor.b, this->clearColor.a);
	}

	void RenderInterface_Metal::clear(const std::vector<ClearMode>& clearParams)
	{
        uint32 currentClear = RenderInterface_MetalUtils::getClearMask(clearParams);
        MTLSetClearMode(currentClear);
	}

	FrameBufferPtr RenderInterface_Metal::createFrameBuffer()
	{
        return std::shared_ptr<FrameBuffer>(new FrameBuffer_Metal());
    }

	DepthBufferPtr RenderInterface_Metal::createDepthBuffer(const Dimensions& dimm, DepthComponent comp, bool isRenderTexture)
	{
		//return std::shared_ptr<DepthBuffer>(new DepthBuffer_OpenGL(dimm, comp, isRenderTexture));
        return std::shared_ptr<DepthBuffer>(new DepthBuffer_Metal(dimm, comp));
    }

	PixelBufferPtr RenderInterface_Metal::createPixelBuffer(const Dimensions& dimm, TextureChannels c)
	{
		//return std::shared_ptr<PixelBuffer>(new PixelBuffer_OpenGL(dimm, c));
        return std::shared_ptr<PixelBuffer>();
    }

	void RenderInterface_Metal::setViewportImpl()
	{
        MTLSetViewport(
            (void*) this->currentEncoder,
            (double) this->currentViewport.pos.x,
			(double) this->currentViewport.pos.y,
			(double) this->currentViewport.size.w,
			(double) this->currentViewport.size.h,
            (double) 0.0f,
            (double) 1.0f);
        
	}
    
    void RenderInterface_Metal::setZImpl()
    {
        // this->setViewportImpl();
    }
    
    void RenderInterface_Metal::clearTextureStage(uint32 stage)
    {
        // glBindTexture(GL_TEXTURE_2D + stage, 0);
    }
    
	void RenderInterface_Metal::draw(Geometry* geom, DrawCallPtr& dc, DrawCallOverrides* overrides)
	{
        if (!this->currentEncoder)
        {
            log::error("No encoder bound!");
            assert(false);
        }
        BufferObjectPtr& vertexBuffer = geom->getVertexBuffer();
        BufferObjectPtr& indexBuffer = geom->getIndexBuffer();
        
        if (!vertexBuffer.get() || !indexBuffer.get())
        {
            log::error("No vertex/index buffers present!");
            return;
        }
        
        MTLPushDebugScope(this->currentEncoder, dc->tag.c_str());
        
        void* vertexBufferData = nullptr;
        void* indexBufferData = nullptr;
        
        int indexType = -1;
        int indexCount = 0;
        int offset = 0;
        
        if (vertexBuffer.get())
        {
            BufferObject_MetalPtr vertexBufferMtl =
                std::static_pointer_cast<BufferObject_Metal>(vertexBuffer);
            if (vertexBufferMtl.get())
            {
                vertexBufferData = vertexBufferMtl->getBufferObject();
            }
        }
        if (indexBuffer.get())
        {
            BufferObject_MetalPtr indexBufferMtl =
                std::static_pointer_cast<BufferObject_Metal>(indexBuffer);
            if (indexBufferMtl.get())
            {
                indexBufferData = indexBufferMtl->getBufferObject();
                indexCount = dc->count;
                indexType = dc->indexType;
                offset = dc->offset * sizeof(uint16);
            }
        }
        
        void* pipelineDescriptor = nullptr;
        const std::map<uint32, uint32>* stageRemap = nullptr;
        
        ShaderHandlePtr* useHandle = nullptr;
        if (dc->shaderHandle.get())
        {
            if (overrides)
            {
                ShaderResourcePtr shaderResource = dc->shaderHandle->getShader();
                if (shaderResource.get())
                {
                    const std::string shaderName = shaderResource->getName();
                    std::map<std::string, ShaderHandlePtr>::iterator it = overrides->shaderOverrideHandles.find(shaderName);
                           
                    if (it != overrides->shaderOverrideHandles.end())
                    {
                        useHandle = &it->second;
                    }
                }
            }
            else
            {
                useHandle = &dc->shaderHandle;
            }
            
            assert(useHandle);
           
            ShaderBindParams bindParams;
            bindParams.geom = geom;
            bindParams.channels = this->currentChannels;
            bindParams.depth = this->currentDepth;
            bindParams.srcBlend = dc->srcBlend;
            bindParams.dstBlend = dc->dstBlend;
            
            pipelineDescriptor =
                (*useHandle)->getPipelineDescriptor(bindParams);
            
            ShaderResourcePtr shaderResource = (*useHandle)->getShader();
            if (shaderResource.get())
            {
                stageRemap = shaderResource->getTextureRemap();
            }
            
            for (int32 i = 0; i < ShaderMAX; ++i)
            {
                void* uniformBuffer = (*useHandle)->getUniformBuffer((ShaderType) i, dc->instanceIndex);
                if (uniformBuffer)
                {
                    MTLSetUniformBuffer(this->currentEncoder, uniformBuffer, 0, 0, i);
                }
            }
        
            TextureStages useTextures;
            // First populate any textures specified by the shader handle
            (*useHandle)->populateTextureStages(useTextures);
            
            // then overwrite them with the draw call
            for (auto& it : dc->textures)
            {
                if (it.second.get() != nullptr)
                {
                    useTextures[it.first] = it.second;
                }
            }
            if (overrides)
            {
                for (auto& it : overrides->textureOverrideHandles)
                {
                    if (it.second.get())
                    {
                        useTextures[it.first] = it.second;
                    }
                }
            }
            
            for (auto& it : useTextures)
            {
                uint16 stage = it.first;
                TextureHandlePtr& texture = it.second;
                
                // NO empty texture bindings in Metal!
                assert(texture.get() != nullptr);
                if (stageRemap)
                {
                    uint32 physical_stage = (uint32) getPhysicalStage((TextureStage) stage);
                    std::map<uint32, uint32>::const_iterator remap = (*stageRemap).find(physical_stage);
                    if (remap != (*stageRemap).end())
                    {
                        stage = remap->second;
                    }
                    else
                    {
                        PrintMetalBindError("No remap texture stage available!");
                    }
                }
                
                if (texture.get())
                {
                    const TexturePtr& tex = texture->getTexture();
                    if (tex.get())
                    {
                        const TextureResourcePtr& texResource = tex->getTextureResource();
                        if (texResource.get())
                        {
                            MTLSamplerParams texParams;
                            texParams.wrapS = texture->getTextureSampleU();
                            texParams.wrapT = texture->getTextureSampleU();
                            texParams.minFilter = TextureFilterNearest;
                            texParams.maxFilter =  TextureFilterLinear;
                            
                            TextureResource_MetalPtr texResourceMetal = std::static_pointer_cast<TextureResource_Metal>(texResource);
                            void* texData = texResourceMetal->getMetalTexture();
                            assert(texData);
                            MTLBindTexture(this->currentEncoder, texData, (unsigned) stage, &texParams);
                        }
                        else
                        {
                            PrintMetalBindError("null Texture Resource!");
                        }
                    }
                    else
                    {
                        PrintMetalBindError("Null Texture GPU asset!");
                    }
                }
                else
                {
                    PrintMetalBindError("Empty Texture Handle at stage ", stage);
                }
            }
            
            if (pipelineDescriptor)
            {
                MTLSetWindingOrder(this->currentEncoder, (int32) dc->frontFace, (int32) dc->cullFace);
                MTLSetDepth(this->currentEncoder, dc->depthTest, dc->depthWrite, dc->depthFunc);
                MTLDraw(
                    this->currentEncoder,
                    vertexBufferData,
                    indexBufferData,
                    indexCount,
                    offset,
                    indexType,
                    (int) dc->type,
                    pipelineDescriptor);
            }
            MTLPopDebugScope(this->currentEncoder);
        }
        else
        {
              PrintMetalBindError("Empty shader handle!!");
        }
    }
    
    void RenderInterface_Metal::setFrameBuffer(const std::string& tag, FrameBufferPtr& frameBuffer, bool isMainScreen, bool forceFrameBuffer, const std::vector<ClearMode>* clearParams)
    {
        assert(frameBuffer.get());
        
        void* texData = nullptr;
        void* depthData = nullptr;
        
        // note MTL defaults to BGRA for some asinine reason
        TextureChannels depthFormat = TextureDepth32;
        TextureChannels channels = TextureBGRA;
        
        const TextureResourcePtr& texResource = frameBuffer->getColorBuffer();
        if (texResource.get())
        {
            TextureResource_MetalPtr texResourceMetal = std::static_pointer_cast<TextureResource_Metal>(texResource);
            texData = texResourceMetal->getMetalTexture();
            channels = texResource->getChannels();
        }
        
        const DepthBufferPtr& depthResource = frameBuffer->getDepthBuffer();
        if (depthResource.get())
        {
            TextureResource_MetalPtr depthResourceMetal = std::static_pointer_cast<TextureResource_Metal>(depthResource->getDepthTexture());
            if (depthResourceMetal.get())
            {
                depthFormat = depthResourceMetal->getChannels();
                depthData = depthResourceMetal->getMetalTexture();
            }
            else if (!isMainScreen)
            {
                depthFormat = TextureNone;
            }
        }
        else if (!isMainScreen)
        {
            depthFormat = TextureNone;
        }
        
        // if we have a previous encoder ready to commit, push that before starting a new pass
        if (this->currentEncoder)
        {
            MTLPushEncoder(this->currentEncoder);
            this->currentEncoder = nullptr;
        }
        
        uint32 currentClear = 0;
        if (clearParams)
        {
            currentClear = RenderInterface_MetalUtils::getClearMask(*clearParams);
        }
        else
        {
            // default to clear color and depth if params unspecified
            currentClear |= (0x1 << uint32(ClearColor));
            currentClear |= (0x1 << uint32(ClearDepth));
        }
        
        MTLSetRenderTarget(tag.c_str(), texData, channels, depthData, depthFormat, currentClear, isMainScreen);
        
        this->currentChannels = channels;
        this->currentDepth = depthFormat;
        
        this->currentRenderTexture = texResource;
        
        this->currentEncoder = MTLCreateEncoder();
    }
    
    
    void RenderInterface_Metal::startRenderPass()
    {
         if (!this->currentEncoder)
         {
             this->currentEncoder = MTLCreateEncoder();
         }
    }
    
    void RenderInterface_Metal::setDefaultFrameBufferImpl()
    {
        if (this->currentEncoder)
        {
            MTLPushEncoder(this->currentEncoder);
            this->currentEncoder = nullptr;
        }
        
        MTLClearRenderTarget();
        
        std::vector<ClearMode> kClearAll = { ClearColor, ClearDepth };
        
        this->setClearColor(this->screenClearColor);
        this->clear(kClearAll);
        
        uint32 w, h;
        RenderInterface::getInstance()->getScreenResolution(w, h);
        RectI viewport(0, 0, (int32) w, (int32) h);
        this->setViewport(viewport);
        
        this->currentChannels = TextureBGRA;
        this->currentDepth = TextureDepth32;
        this->currentRenderTexture = nullptr;
        
        this->startRenderPass();
        
    }
    
	void RenderInterface_Metal::clearBufferTypeImpl(BufferType type)
	{
        /*
		GLuint kBufferTypeGL[] =
		{
			GL_ARRAY_BUFFER,
			GL_ELEMENT_ARRAY_BUFFER
		};

		GLenum bufferType = kBufferTypeGL[type];
		GL_CHECK(glBindBuffer(bufferType, 0));;
         */
    }

	void RenderInterface_Metal::setBlendFuncImpl()
	{
		// GL_CHECK(glBlendFunc(kBlendTypeConvert[this->srcBlend], kBlendTypeConvert[this->dstBlend]));
	}

	void RenderInterface_Metal::setDepthTestFuncImpl()
	{
		// GL_CHECK(glDepthFunc(kDepthTypeConvert[this->depthFunc]));
	}

	void RenderInterface_Metal::setDepthWriteImpl()
	{
		// GL_CHECK(glDepthMask(this->depthWrite));
	}

	void RenderInterface_Metal::setLineWidthImpl()
	{
		// GL_CHECK(glLineWidth(this->lineWidth));
	}

	void RenderInterface_Metal::setCullFaceImpl()
	{
		// GL_CHECK(glCullFace(kCullFaceConvert[this->cullFace]));
	}

	void RenderInterface_Metal::setFrontFaceImpl()
	{
		// GL_CHECK(glFrontFace(kFrontFaceConvert[this->frontFace]));
	}

	void RenderInterface_Metal::setScissorRectImpl()
	{
		//GL_CHECK(glScissor(
		//	this->scissorRect.pos.x,
		//	this->scissorRect.pos.y,
		//	this->scissorRect.size.w,
		//	this->scissorRect.size.h));
	}

	void RenderInterface_Metal::setEnabledImpl(StateType type)
	{
		// GL_CHECK(glEnable(kStateConvert[type]));
	}

	void RenderInterface_Metal::setDisabledImpl(StateType type)
	{
		// GL_CHECK(glDisable(kStateConvert[type]));
	}

	void RenderInterface_Metal::checkExtensions()
	{
        
	}

	void RenderInterface_Metal::pushDebugScope(const std::string& tag)
	{
         if (this->currentEncoder)
         {
             MTLPushDebugScope(currentEncoder, tag.c_str());
             this->debugScope++;
         }
    }

	void RenderInterface_Metal::popDebugScope()
	{
        if (this->currentEncoder)
        {
            MTLPopDebugScope(this->currentEncoder);
            this->debugScope--;
            if (this->debugScope < 0)
            {
                log::error("Debug scope mismatch!");
                assert(false);
            }
        }
    }
    
    void RenderInterface_Metal::captureDefaultFrameBufferImpl()
    {
        //glGetIntegerv(GL_FRAMEBUFFER_BINDING, &this->defaultFrameBuffer);
    }
    
   
}
