#pragma once

#include "ClassDef.h"
#include "global/Singleton.h"
#include "gfx/RenderInterface.h"
#include "gfx/RenderTexture.h"
#include "gfx/Types.h"

#include "gfx/TextureHandle.h"

namespace cs
{

	
	typedef std::unordered_map<RenderTargetType, RenderTexturePtr> RenderTextureMap;

	struct RenderTextureInputMapping
	{
		RenderTextureInputMapping()
			: targetBuffer(RenderTargetTypeNone)
			, targetStage(TextureStageNone)
			, depth(false)
		{

		}

		RenderTextureInputMapping(const RenderTextureInputMapping& rhs)
			: targetBuffer(rhs.targetBuffer)
			, targetStage(rhs.targetStage)
			, depth(rhs.depth)
		{

		}

		RenderTextureInputMapping(RenderTargetType bufType, TextureStage stage, bool useDepth = false)
			: targetBuffer(bufType)
			, targetStage(stage)
			, depth(useDepth)
		{

		}

		void operator=(const RenderTextureInputMapping& rhs)
		{
			this->targetBuffer = rhs.targetBuffer;
			this->targetStage = rhs.targetStage;
			this->depth = rhs.depth;
		}

		RenderTargetType targetBuffer;
		TextureStage targetStage;
		bool depth;

	};

	struct RenderTextureSharedData
	{
		RenderTextureSharedData()
			: shared(false)
		{ }

		RenderTexturePtr target;
		bool shared;
	};

	class RenderTargetManager : public Singleton<RenderTargetManager>
	{
	public:

		static const RenderTargetType kUseBackBuffer;

		struct SharedRenderTarget
		{
			SharedRenderTarget()
				: mapped(false)
			{ }

			bool mapped;
			Dimensions dimm;
			TextureChannels channels;
			DepthComponent depthComponent;
			RenderTexturePtr target;
		};

		RenderTexturePtr populateSharedTarget(const std::string& name, int32 w, int32 h, TextureChannels c, DepthComponent depth);
		RenderTexturePtr populateSharedTarget(const std::string& name, Dimensions d, TextureChannels c, DepthComponent depth);
		RenderTexturePtr mapSharedTarget(const std::string& name, Dimensions d, TextureChannels c, DepthComponent depth, RenderTextureSharedData& data);
		void unmapSharedTarget(RenderTextureSharedData& data);
		vec2 getTextureFit(RenderTargetType type);

		RenderTargetManager()
		{
			for (size_t i = 0; i < RenderTargetTypeMAX; ++i) this->renderTargets[static_cast<RenderTargetType>(i)] = nullptr;
		}

		void init(const Dimensions& dimm);

		RenderTexturePtr& getTarget(RenderTargetType type) 
		{
			return this->renderTargets[type];
		}

		RectI getViewport(RenderTargetType type);

		RenderTextureMap renderTargets;

        std::map<std::string, SharedRenderTarget> sharedTargets;

	};
}
