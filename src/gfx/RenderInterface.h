#pragma once

#include "gfx/TextureResource.h"
#include "gfx/Shader.h"
#include "gfx/ShaderResource.h"
#include "gfx/ShaderProgram.h"
#include "gfx/BufferObject.h"
#include "gfx/FrameBuffer.h"
#include "gfx/DepthBuffer.h"
#include "gfx/PixelBuffer.h"
#include "gfx/Geometry.h"
#include "gfx/Color.h"
#include "gfx/VertexArrayObject.h"
#include "fx/ParticleEffect.h"

#include "math/Rect.h"

#define USE_DEBUG_SCOPE 1

#if defined(USE_DEBUG_SCOPE)
	#define PushDebugScope(tag)		RenderInterface::getInstance()->pushDebugScope(tag)
	#define PopDebugScope()			RenderInterface::getInstance()->popDebugScope()
#else
	#define PushDebugScope(tag)		void(0)
	#define PopDebugScope()			void(0)
#endif

namespace cs
{
	class Mesh;
	class ShaderHandle;
	class TextureHandle;
	class ParticleEffectHandle;

	class RenderInterface
	{
	public:

		RenderInterface()
			: clearColor(ColorF(0.0f, 0.0f, 0.0f, 0.0f))
			, frameBuffer(nullptr)
			, currentViewport(RectI(0, 0, 1, 1))
			, srcBlend(BlendNone)
			, dstBlend(BlendNone)
			, depthFunc(DepthLess)
			, depthWrite(true)
			, lineWidth(0.0f)
			, cullFace(CullBack)
			, frontFace(FrontFaceCCW)
			, contentScale(1.0f)
            , allowDepthBuffering(true)
            , zNear(-1.0f)
            , zFar(1.0f)
		{
			for (uint32 i = 0; i < BufferTypeMAX; i++)
				currentBuffers[i] = nullptr;
		}

		void free();

		const static uint32 kMaxTextureStages = 4;
		const static uint32 kMaxLightIndex = 3;
		const static DepthComponent kDefaultDepthComponent;

		static RenderInterface* getInstance();

		virtual TextureResourcePtr loadTexture(const std::string& fileName) = 0;
		virtual TextureResourcePtr loadTexture(const Dimensions& dimm, TextureChannels channels = TextureNone, uchar* bytes = nullptr, TextureUsage usage = TextureUsageShaderRead) = 0;
		
		virtual ShaderPtr loadShader(ShaderType type, const std::string& source = "", bool printSource = false) = 0;
		virtual ShaderProgramPtr createShaderProgram() = 0;
		virtual BufferObjectPtr createBufferObject(BufferType type) = 0;
		virtual FrameBufferPtr createFrameBuffer() = 0;
		virtual DepthBufferPtr createDepthBuffer(const Dimensions& dimm, DepthComponent comp, bool isRenderTexture = true) = 0;
		virtual PixelBufferPtr createPixelBuffer(const Dimensions& dimm, TextureChannels c) = 0;
		virtual VertexArrayObjectPtr createVertexArrayObject() = 0;

		const RectI& getViewport() const { return this->currentViewport; }

		virtual void clear(const std::vector<ClearMode>& clearParams) = 0;
		
        void init(int32 window_width, int32 window_height);
		virtual void initPlatform() = 0;
		virtual void draw(Geometry* geom, DrawCallPtr& dc, DrawCallOverrides* overrides = nullptr) = 0;

		void setClearColor(const ColorF& clearColor);
        virtual void setScreenClearColor(const ColorF& clearColor) { }
        
		void begin();
		void update(float32 dt);
        
        virtual void beginFrame() { }
        virtual void endFrame() { }
        virtual void testFrame() { }
        virtual void getScreenResolution(uint32& width, uint32& height);

        virtual void clearTextureStage(uint32 stage) = 0;
        
		bool setCurrentShader(ShaderResourcePtr& s)
		{
			if (this->shader.get() != s.get())
			{
				this->shader = s;
				return true;
			}
			return false;
		}
        
        virtual TextureChannels getCurrentRenderChannels() const { return TextureNone; }
        virtual TextureChannels getCurrentDepthChannels() const { return TextureNone; }
        
		const ShaderResourcePtr& getCurrentShader() const { return this->shader; }

		void setViewport(const RectI& viewport, bool force = false);
        void setZ(float32 z_near, float32 z_far);
		void setBuffer(BufferObjectPtr& ptr);
		void clearBuffer(BufferType type);
		void setBlendFunc(BlendType src, BlendType dst);
		void setDepthTestFunc(DepthType depthFunc);
		void setLineWidth(float32 lw);
		void setScissorRect(const RectI& rect);
		void setCullFace(CullFace cf);
		void setFrontFace(FrontFace ff);
		void setDepthWrite(bool dw);
        
        void captureDefaultFrameBuffer();
        void setDefaultFrameBuffer();
        virtual void setFrameBuffer(const std::string& tag, FrameBufferPtr& frameBuffer, bool isMainScreen = false, bool forceFrameBuffer = false, const std::vector<ClearMode>* clearParams = nullptr);
        void clearFrameBuffer();
        
        virtual void startRenderPass() { }
        
        BufferObjectPtr getCurrentBufferBinding(BufferType type);

		void set(StateType type, bool enabled, bool force = false);
		bool get(StateType type) const { return this->renderState[type]; }
        
        int32 getWindowWidth() const { return this->windowWidth; }
        int32 getWindowHeight() const { return this->windowHeight; }

		virtual void pushDebugScope(const std::string& tag) { }
		virtual void popDebugScope() { }

		float32 getContentScale() const { return this->contentScale; }
		void setContentScale(float32 scale);
        
        static void setBlurPercent(float32 blur);
        static void setChomaticAbberationStrength(float32 ca);
        
		static std::shared_ptr<TextureHandle> kDefaultTexture;
		static std::shared_ptr<TextureHandle> kWhiteTexture;
		static std::shared_ptr<TextureHandle> kErrorTexture;
		static std::shared_ptr<TextureHandle> kEmptyTexture;

        static std::shared_ptr<ShaderHandle> kDefaultParticleShader;
		static std::shared_ptr<ShaderHandle> kDefaultTextureShader;
		static std::shared_ptr<ShaderHandle> kDefaultTextureShader2;
		static std::shared_ptr<ShaderHandle> kDefaultColorShader;
		static std::shared_ptr<ShaderHandle> kFontAtkasShader;
		static std::shared_ptr<ShaderHandle> kDefaultDebugShader;
		static std::shared_ptr<ShaderHandle> kDefaultFontShader;
		static std::shared_ptr<ShaderHandle> kDefaultTextureColorShader;
		static std::shared_ptr<ShaderHandle> kDefaultTextureColorShader2;
		static std::shared_ptr<ShaderHandle> kTextureAlphaShader;
		static std::shared_ptr<ShaderHandle> kTextureSingleChannelShader;
		static std::shared_ptr<ShaderHandle> kTextureAlphaBWShader;
		static std::shared_ptr<ShaderHandle> kDefaultFontOutlineShader;
		static std::shared_ptr<ShaderHandle> kNormalColorLit;
		static std::shared_ptr<ShaderHandle> kVertexPhongLit;
		static std::shared_ptr<ShaderHandle> kVertexPhongLitTexture;
        static std::shared_ptr<ShaderHandle> kMetalTest;
		static std::shared_ptr<ShaderHandle> kColorOutline;
        static std::shared_ptr<ShaderHandle> kSolidColorTest;

		static std::shared_ptr<ParticleEffectHandle> kDefaultParticleEffect;

		static std::shared_ptr<Mesh> kCubeMesh;

		static std::shared_ptr<TextureHandle> getDefaultTextureForStage(TextureStage stage);

		enum RenderStat
		{
			RenderStatNone = -1,
			RenderStatDrawCall,
			RenderStatTextureSwap,
			RenderStatPrimitives,
			//...
			RenderStatMAX
		};

		static uint32 renderStatCounts[RenderStatMAX];
		static const char* kRenderStatStr[];

		static void resetRenderStats();
		static void incrementRenderStat(RenderStat stat, uint32 inc);
		static uint32 getRenderStat(RenderStat stat);
        
	protected:

		virtual void setClearColorImpl() = 0;
		virtual void setViewportImpl() = 0;
		virtual void clearBufferTypeImpl(BufferType type) = 0;
		virtual void setBlendFuncImpl() = 0;
		virtual void setDepthTestFuncImpl() = 0;
		virtual void setLineWidthImpl() = 0;
		virtual void setScissorRectImpl() = 0;
		virtual void setEnabledImpl(StateType type) = 0;
		virtual void setDisabledImpl(StateType type) = 0;
		virtual void setCullFaceImpl() = 0;
		virtual void setFrontFaceImpl() = 0;
		virtual void setDepthWriteImpl() = 0;
        virtual void captureDefaultFrameBufferImpl() = 0;
        virtual void setDefaultFrameBufferImpl() = 0;
        virtual void setZImpl() = 0;

		ColorF clearColor;
		uint32 clearOptions;
		FrameBufferPtr frameBuffer;
		RectI currentViewport;
		BlendType srcBlend;
		BlendType dstBlend;
		DepthType depthFunc;
		bool depthWrite;
		float32 lineWidth;
		RectI scissorRect;
		CullFace cullFace;
		FrontFace frontFace;
		ShaderResourcePtr shader;
        bool allowDepthBuffering;
        int32 windowWidth;
        int32 windowHeight;
        float32 zNear;
        float32 zFar;
        static float32 blurPercent;
        static float32 caStrength;

		float32 contentScale;

        FrameBufferPtr currentFrameBuffer;
		BufferObjectPtr currentBuffers[BufferTypeMAX];	

		bool renderState[StateTypeMAX];

		static RenderInterface* instance;

	};

}
