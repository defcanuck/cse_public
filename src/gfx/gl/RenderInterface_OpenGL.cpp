#include "PCH.h"

#include "gfx/RenderInterface.h"
#include "gfx/gl/RenderInterface_OpenGL.h"
#include "os/FileManager.h"
#include "global/ResourceFactory.h"
#include "global/Utils.h"
#include "gfx/Mesh.h"

#include "gfx/gl/OpenGL.h"
#include "gfx/gl/TextureResource_OpenGL.h"
#include "gfx/gl/Shader_OpenGL.h"
#include "gfx/gl/ShaderProgram_OpenGL.h"
#include "gfx/gl/BufferObject_OpenGL.h"
#include "gfx/gl/DepthBuffer_OpenGL.h"
#include "gfx/gl/FrameBuffer_OpenGL.h"
#include "gfx/gl/PixelBuffer_OpenGL.h"
#include "gfx/gl/VertexBuffer_OpenGL.h"
#include "gfx/gl/VertexArrayObject_OpenGL.h"
#include "gfx/ShaderUtils.h"

#include <memory>

namespace cs
{
	RenderInterface_OpenGL::RenderInterface_OpenGL()
		: defaultFrameBuffer(0)
	{

	}

#if !defined(CS_METAL)
	RenderInterface* RenderInterface::getInstance()
	{
		if (!instance)
			instance = new RenderInterface_OpenGL();

		return instance;
	}
#endif
    
	TextureResourcePtr RenderInterface_OpenGL::loadTexture(const std::string& filePath)
	{
		return std::shared_ptr<TextureResource>(new TextureResource_OpenGL(filePath));
	}

	TextureResourcePtr RenderInterface_OpenGL::loadTexture(const Dimensions& dimm, TextureChannels channels, uchar* bytes, TextureUsage usage)
	{
		return std::shared_ptr<TextureResource>(new TextureResource_OpenGL(dimm, channels, bytes, usage));
	}

	ShaderPtr RenderInterface_OpenGL::loadShader(ShaderType type, const std::string& fileName, bool printSource)
	{
		ShaderPtr shader = std::static_pointer_cast<Shader>(CREATE_CLASS(Shader_OpenGL, type));
		if (fileName.length() > 0)
		{
			// todo
			shader->compile(fileName, "", printSource);
		}
		return shader;
	}

	ShaderProgramPtr RenderInterface_OpenGL::createShaderProgram()
	{
		return std::shared_ptr<ShaderProgram>(new ShaderProgram_OpenGL());
	}

	BufferObjectPtr RenderInterface_OpenGL::createBufferObject(BufferType type)
	{
		if (type == BufferTypeVertex)
		{
			return std::shared_ptr<BufferObject>(new VertexBuffer_OpenGL());
		}

		return std::shared_ptr<BufferObject>(new BufferObject_OpenGL(type));
	}

	VertexArrayObjectPtr RenderInterface_OpenGL::createVertexArrayObject()
	{
		return std::shared_ptr<VertexArrayObject>(new VertexArrayObject_OpenGL());
	}

	void RenderInterface_OpenGL::initPlatform()
	{

		TextureResource_OpenGL::initSamplers();
		
		static VertexArrayObjectPtr kDefaultVAO = this->createVertexArrayObject();
        glBindVertexArray(0);

		for (uint32 i = 0; i < RenderInterface::kMaxTextureStages; i++)
		{
			std::stringstream str;
			str << "texture" << i;
			UniformPtr texture = CREATE_CLASS(Uniform, str.str(), UniformInt);
			SharedUniform::getInstance().addUniform(texture);
		}

        ShaderUtils::addDefaultShaders();

		this->checkExtensions();

	}

	void RenderInterface_OpenGL::setClearColorImpl()
	{
		GL_CHECK(glClearColor(this->clearColor.r, this->clearColor.g, this->clearColor.b, this->clearColor.a));
	}

	void RenderInterface_OpenGL::clear(const std::vector<ClearMode>& clearParams)
	{
		uint32 flags = 0;
		for (auto& it : clearParams)
			flags |= kClearConvert[it];

		GL_CHECK(glClear(flags));
	}

	FrameBufferPtr RenderInterface_OpenGL::createFrameBuffer()
	{
		return std::shared_ptr<FrameBuffer>(new FrameBuffer_OpenGL());
	}

	DepthBufferPtr RenderInterface_OpenGL::createDepthBuffer(const Dimensions& dimm, DepthComponent comp, bool isRenderTexture)
	{
		return std::shared_ptr<DepthBuffer>(new DepthBuffer_OpenGL(dimm, comp, isRenderTexture));
	}

	PixelBufferPtr RenderInterface_OpenGL::createPixelBuffer(const Dimensions& dimm, TextureChannels c)
	{
		return std::shared_ptr<PixelBuffer>(new PixelBuffer_OpenGL(dimm, c));
	}

	void RenderInterface_OpenGL::setViewportImpl()
	{
		glViewport(
			this->currentViewport.pos.x,
			this->currentViewport.pos.y,
			this->currentViewport.size.w,
			this->currentViewport.size.h);
	}

    
    void RenderInterface_OpenGL::clearTextureStage(uint32 stage)
    {
        glBindTexture(GL_TEXTURE_2D + stage, 0);
    }
    
	void RenderInterface_OpenGL::draw(Geometry* geom, DrawCallPtr& dc, DrawCallOverrides* overrides)
	{
		// if (dc->type == cs::DrawLines)
		//	return;

		//glPushGroupMarker(0, "Draw Spaceship");
		
		/*
		const static char* kTestString = "Test";
		size_t len = sizeof(kTestString);

		void* proc = GetAnyGLFuncAddress("glPushGroupMarkerEXT");
		if (proc)
		{
			PFNGLPUSHGROUPMARKEREXTPROC glPushGroupMarker = (PFNGLPUSHGROUPMARKEREXTPROC)proc;
			glPushGroupMarker(0, kTestString);

		}
		*/

		this->set(StateBlend, dc->blend);
		if (dc->blend)
			this->setBlendFunc(dc->srcBlend, dc->dstBlend);

		this->set(StateDepthTest, dc->depthTest);
		if (this->get(StateDepthTest))
		{
			this->setDepthTestFunc(dc->depthFunc);
			this->setDepthWrite(dc->depthWrite);
		}

		this->set(StateScissorTest, dc->scissor);
		if (dc->scissor)
			this->setScissorRect(dc->scissorRect);

		this->set(StateCullFace, dc->cullFace != CullNone);
		if (dc->cullFace != CullNone)
			this->setCullFace(dc->cullFace);

		
		this->setFrontFace(dc->frontFace);

		assert(dc->count > 0);

		if (this->currentBuffers[BufferTypeIndex])
		{
			uchar* offset = static_cast<uchar*>(0) + (dc->offset * kTypeSize[dc->indexType]);
			GL_CHECK(glDrawElements(kDrawConvert[dc->type], dc->count, kTypeConvert[dc->indexType], offset));
		}
		else 
		{
			if (dc->indices.size() > 0)
			{
				GL_CHECK(glDrawElements(kDrawConvert[dc->type], dc->count, kTypeConvert[dc->indexType], &dc->indices[0]));
			}
			else 
			{
				GL_CHECK(glDrawArrays(kDrawConvert[dc->type], dc->offset, dc->count));
			}
		}
		
		RenderInterface::incrementRenderStat(RenderStatDrawCall, 1);
		RenderInterface::incrementRenderStat(RenderStatPrimitives, dc->count);
	}

	void RenderInterface_OpenGL::clearBufferTypeImpl(BufferType type)
	{
		GLuint kBufferTypeGL[] =
		{
			GL_ARRAY_BUFFER,
			GL_ELEMENT_ARRAY_BUFFER
		};

		GLenum bufferType = kBufferTypeGL[type];
		GL_CHECK(glBindBuffer(bufferType, 0));;
	}

	void RenderInterface_OpenGL::setBlendFuncImpl()
	{
		GL_CHECK(glBlendFunc(kBlendTypeConvert[this->srcBlend], kBlendTypeConvert[this->dstBlend]));
	}

	void RenderInterface_OpenGL::setDepthTestFuncImpl()
	{
		GL_CHECK(glDepthFunc(kDepthTypeConvert[this->depthFunc]));
	}

	void RenderInterface_OpenGL::setDepthWriteImpl()
	{
		GL_CHECK(glDepthMask(this->depthWrite));
	}

	void RenderInterface_OpenGL::setLineWidthImpl()
	{
		GL_CHECK(glLineWidth(this->lineWidth)); 
	}

	void RenderInterface_OpenGL::setCullFaceImpl()
	{
        if (this->cullFace != CullNone)
        {
            GL_CHECK(glEnable(GL_CULL_FACE));
            GL_CHECK(glCullFace(kCullFaceConvert[this->cullFace]));
        }
        else
        {
            GL_CHECK(glDisable(GL_CULL_FACE));
        }
    }

	void RenderInterface_OpenGL::setFrontFaceImpl()
	{
		GL_CHECK(glFrontFace(kFrontFaceConvert[this->frontFace]));
	}

	void RenderInterface_OpenGL::setScissorRectImpl()
	{
		GL_CHECK(glScissor(
			this->scissorRect.pos.x,
			this->scissorRect.pos.y,
			this->scissorRect.size.w,
			this->scissorRect.size.h));
	}

	void RenderInterface_OpenGL::setEnabledImpl(StateType type)
	{
		GL_CHECK(glEnable(kStateConvert[type]));
	}

	void RenderInterface_OpenGL::setDisabledImpl(StateType type)
	{
		GL_CHECK(glDisable(kStateConvert[type]));
	}

	void RenderInterface_OpenGL::checkExtensions()
	{
        
        const GLubyte* str = glGetString(GL_EXTENSIONS);
        if (!str)
        {
            return;
        }
        
        std::string ext((const char*) str);
        StringList extensionList = explode(ext, ' ');
        
        const char* kCheckExt[] =
        {
            "GL_EXT_debug_label",    // ExDebugLabel
            "GL_EXT_debug_marker"    // ExDebugMarker
        };
        
        for (int i = 0; i < ExMAX; ++i)
        {
            extensions[i] = false;
            for (size_t j = 0; j < extensionList.size(); j++)
            {
                const std::string& supportedExt = extensionList[j];
                if (std::string(kCheckExt[i]) == supportedExt)
                {
                    extensions[i] = true;
                    break;
                }
            }
        }
        
	}

	void RenderInterface_OpenGL::pushDebugScope(const std::string& tag)
	{
        
        if (!extensions[ExDebugMarker])
            return;
        
#if defined(CS_IOS)
        glPushGroupMarkerEXT(tag.length() + 1, tag.c_str());
#endif
        
	}

	void RenderInterface_OpenGL::popDebugScope()
	{
        if (!extensions[ExDebugMarker])
            return;
        
#if defined(CS_IOS)
        glPopGroupMarkerEXT();
#endif
	}
    
    void RenderInterface_OpenGL::captureDefaultFrameBufferImpl()
    {
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &this->defaultFrameBuffer);
    }
    
    void RenderInterface_OpenGL::setDefaultFrameBufferImpl()
    {
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFrameBuffer));
    }
}
