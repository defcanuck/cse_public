#pragma once

#include "gfx/RenderInterface.h"

namespace cs
{
	class RenderInterface_Metal : public RenderInterface
	{

		typedef RenderInterface BASECLASS;

	public:

		RenderInterface_Metal();
		virtual ~RenderInterface_Metal() { }
	
		virtual TextureResourcePtr loadTexture(const std::string& filePath);
		virtual TextureResourcePtr loadTexture(const Dimensions& dimm, TextureChannels channels = TextureNone, uchar* bytes = nullptr, TextureUsage usage = TextureUsageShaderRead);
		
		virtual ShaderPtr loadShader(ShaderType type, const std::string& source = "", bool printSource = false);

		virtual ShaderProgramPtr createShaderProgram();
		virtual BufferObjectPtr createBufferObject(BufferType type);
		virtual FrameBufferPtr createFrameBuffer();
		virtual DepthBufferPtr createDepthBuffer(const Dimensions& dimm, DepthComponent comp, bool isRenderTexture = true);
		virtual PixelBufferPtr createPixelBuffer(const Dimensions& dimm, TextureChannels c);
		virtual VertexArrayObjectPtr createVertexArrayObject();

		virtual void initPlatform();
		virtual void clear(const std::vector<ClearMode>& clearParams);
		virtual void draw(Geometry* geom, DrawCallPtr& dc, DrawCallOverrides* overrides = nullptr);
        virtual void setScreenClearColor(const ColorF& clearColor) { this->screenClearColor = clearColor; }
        
        virtual void clearTextureStage(uint32 stage);
        
		virtual void pushDebugScope(const std::string& tag);
		virtual void popDebugScope();
        
        virtual void beginFrame();
        virtual void endFrame();
        virtual void testFrame();
        
        virtual void startRenderPass();
        
        virtual void setFrameBuffer(const std::string& tag, FrameBufferPtr& frameBuffer, bool isMainScreen = false, bool forceFrameBuffer = false, const std::vector<ClearMode>* clearParams = nullptr);
        virtual TextureChannels getCurrentRenderChannels() const { return this->currentChannels; }
        virtual TextureChannels getCurrentDepthChannels() const { return this->currentDepth; }
        
        enum Extension
        {
            ExNone = -1,
            ExDebugLabel,
            ExDebugMarker,
            ExMAX
        };
        
        bool extensions[ExMAX];
        
	protected:

		virtual void setClearColorImpl();
		virtual void setViewportImpl();
		virtual void clearBufferTypeImpl(BufferType type);
		virtual void setBlendFuncImpl();
		virtual void setDepthTestFuncImpl();
		virtual void setLineWidthImpl();
		virtual void setScissorRectImpl();
		virtual void setEnabledImpl(StateType type);
		virtual void setDisabledImpl(StateType type);
		virtual void setCullFaceImpl();
		virtual void setFrontFaceImpl();
		virtual void setDepthWriteImpl();
        virtual void captureDefaultFrameBufferImpl();
        virtual void setDefaultFrameBufferImpl();
        virtual void setZImpl();

	private:
        
        int defaultFrameBuffer;
		void checkExtensions();
        
        void* currentEncoder;
        TextureChannels currentChannels;
        TextureChannels currentDepth;
        TextureResourcePtr currentRenderTexture;
        
        int debugScope;
        ColorF screenClearColor;
	};

}

