#pragma once

#include "ClassDef.h"

#include "gfx/RenderTarget.h"
#include "gfx/ShaderResource.h"
#include "gfx/DisplayList.h"
#include "gfx/DynamicGeometry.h"

#include <vector>

namespace cs
{
	typedef std::vector<RenderTextureInputMapping> PostProcessInputMap;
	typedef std::function<void(const RectI&, RenderTexturePtr&)> PostProcessCallback;

	struct PostProcessParams
	{
		PostProcessParams()
			: shader(RenderInterface::kDefaultTextureShader)
			, viewport(RectI())
			, fillRect(RectF())
			, traversal(RenderTraversalMain)
			, clearColor(ColorF::Clear)
			, clearModes({ ClearColor, ClearDepth })
			, flipHorizontal(false)
			, flipVertical(false)
			, removeAfterUse(false)
			, isDynamic(false)
		{ }

		PostProcessParams(const PostProcessParams& rhs)
			: shader(rhs.shader)
			, inputMap(rhs.inputMap)
			, outputMap(rhs.outputMap)
			, viewport(rhs.viewport)
			, fillRect(rhs.fillRect)
			, traversal(rhs.traversal)
			, clearColor(rhs.clearColor)
			, clearModes(rhs.clearModes)
			, flipHorizontal(rhs.flipHorizontal)
			, flipVertical(rhs.flipVertical)
			, callback(rhs.callback)
			, removeAfterUse(rhs.removeAfterUse)
			, isDynamic(rhs.isDynamic)
		{ }

		RectI viewport;
		RectF fillRect;
		ShaderHandlePtr shader;
		PostProcessInputMap inputMap;
		RenderTextureInputMapping outputMap;
		RenderTraversal traversal;
		bool flipHorizontal;
		bool flipVertical;

		ColorF clearColor;
		std::vector<ClearMode> clearModes;
		PostProcessCallback callback;

		bool removeAfterUse;
		bool isDynamic;
	};

	CLASS_DEFINITION(PostProcess)
	public:

		
		PostProcess(const std::string& name, const PostProcessParams& params);
		virtual ~PostProcess() { }

		void mapInputs(RenderTextureMap& inputs);
		void mapOutputs(RenderTextureMap& output);

		void resolve(DisplayList& display_list);
		void draw(const RectI& viewport);
		void update();
		
		void setTexture(TextureHandlePtr& handle, TextureStage stage = TextureStageDiffuse);
		void setOutput(RenderTexturePtr& out);

		void revertTexture(TextureStage stage);

		const PostProcessParams& getParams() const { return this->postParams; }

		void setOrthoOffset(const vec2& offset) { this->orthoOffset = offset; }
		void setOrthoScale(const vec2& scale) { this->orthoScale = scale; }

		size_t updateVertices(uchar* data, size_t bufferSize, VertexDeclaration& decl);
		size_t updateIndices(uchar* data, size_t bufferSize);
		void setDrawParams(int32 index, std::vector<DrawCallPtr>& dcs);
		void setShader(const std::string& shaderName);
		
		size_t getVertexBufferSize();
		size_t getIndexBufferSize();

		DrawCallPtr& getResolveDraw() { return this->resolveDraw; }

		void setSkew(float top, float bot) 
		{ 
			this->topSkew = top; 
			this->botSkew = bot;
		}

		void setTint(const ColorB& tint);

	protected:

		void initGeometry();

		void initStaticGeometry(GeometryDataPtr& data);
		void initDynamicGeometry(GeometryDataPtr& data);

		GeometryDataPtr initGeometryData();
		
		std::string postName;

		TextureStages textureInputs;
		RenderTexturePtr textureOutput;

		PostProcessParams postParams;
		vec2 orthoOffset;
		vec2 orthoScale;
		float topSkew;
		float botSkew;

		DrawCallPtr resolveDraw;
		GeometryPtr resolveGeom;

		ColorB tint;

	};

	typedef std::vector<PostProcessPtr> PostProcessList;
	typedef std::map<std::string, PostProcessParams> PostProcessParamsMap;
}
