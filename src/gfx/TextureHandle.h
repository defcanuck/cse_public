#pragma once

#include "ClassDef.h"
#include "gfx/Texture.h"

#include "global/Event.h"

#include <string>

namespace cs
{
	struct AnimationInstanceData
	{
		AnimationInstanceData()
			: curDelta(0.0f)
		{ }

		float32 curDelta;
	};

	CLASS_DEFINITION_REFLECT(TextureAnimation)
	public:
		TextureAnimation() 
			: flipU(false)
			, flipV(false)
		{ }
		virtual ~TextureAnimation() { }

		virtual bool hasAnimation() const { return false; }
		virtual const RectF& getRectForIndex(size_t index) const;
		virtual const RectF& getRectForPercent(AnimationInstanceData& data, float32 dt) const;
		virtual const RectF& getRectForDelta(AnimationInstanceData& data, float32 dt);

		virtual int32 getNumFrames() const { return 0; }
		virtual bool doesAnimate() const { return false; }

		void update() { this->updateImpl(); }
		virtual void updateImpl() { }

		bool flipU;
		bool flipV;

	};

	CLASS_DEFINITION_DERIVED_REFLECT(TextureAnimationSheet, TextureAnimation)
	public:

		typedef std::vector<RectF> AnimationSheetFrames;

		TextureAnimationSheet() 
			: TextureAnimation()
			, numRows(1)
			, numColumns(1)
			, numFrames(-1)
			, speed(1.0f)
			, rowMajor(false)
		{ }

		virtual ~TextureAnimationSheet() { }

		virtual void onNew();
		virtual void onPostLoad(const LoadFlagMask& flags);
		virtual bool hasAnimation() const { return true; }
		virtual int32 getNumFrames() const { return this->numFrames; }

		virtual const RectF& getRectForIndex(size_t index) const;
		virtual const RectF& getRectForPercent(AnimationInstanceData& data, float32 pct) const;
		virtual const RectF& getRectForDelta(AnimationInstanceData& data, float32 dt);

		virtual bool doesAnimate() const { return true; }
		virtual void updateImpl();

	private:

		void onSheetDimensionsChanged();

		int32 numRows;
		int32 numColumns;
		int32 numFrames;
		float32 speed;
		bool rowMajor;

		AnimationSheetFrames frames;
	};

	struct TextureAtlasData;
	class TextureAtlas;
	
	CLASS_DEFINITION_REFLECT(TextureHandle)
		
	public:

		const static RectF kDefaultTextureSampleRect;
		const static RectF kDefaultRenderTextureSampleRect;
        const static RectF kDefaultTextureSampleRectSafe;

		TextureHandle();
		TextureHandle(const TextureHandlePtr& rhs);
		TextureHandle(const TexturePtr& ptr);
		TextureHandle(const TexturePtr& ptr, RectF& rect);

		TextureHandle(const std::string& fileName);
		TextureHandle(RenderTargetType targetType, bool useDepth = false);

		virtual void onPostLoad(const LoadFlagMask& flags);

		virtual ~TextureHandle();

		const std::string getTextureName() const;
		const TexturePtr& getTexture() { return this->texture; }
		bool hasTexture() { return this->texture != nullptr; }

		RectF getUVRect(float32 dt = 0.0f);
        void setUVRectSafe();
		void setUVRect(const RectF& rect);

		void squishWidth(float32 pct);
		void squishHeight(float32 pct);

		void shiftHorizontal(float32 pct);
		void shiftVertical(float32 pct);

		vec2 getUVByIndex(uint32 idx, float32 pct = 0.0f);
		
		void getCorners(vec2* corners, float32 pct = 0.0f);
		void getCorners(vec2* corners, int32 frameIndex = 0);

		void setFlipHorizontal(bool flip);
		void setFlipVertical(bool flip);

		bool getFlipHorizontal() const { return this->flipU; }
		bool getFlipVertical() const { return this->flipV; }

		void setWrapHorizontal(bool wrap) { this->wrapU = wrap; }
		void setWrapVertical(bool wrap) { this->wrapV = wrap; }

        TextureSample getTextureSampleU() const { return (this->wrapU) ? TextureSampleRepeat :TextureSampleClamp; }
        TextureSample getTextureSampleV() const { return (this->wrapV) ? TextureSampleRepeat :TextureSampleClamp; }

		void bind(uint32 stage);

		void fitUV();
		bool equals(const TextureHandlePtr& rhs);
		void onTextureChanged();

		bool getIsAtlased() const { return this->atlased; }

		bool hasTargetType() const { return this->targetType != -1; }
		RenderTargetType getTargetType() const { return (RenderTargetType) this->targetType; }

		uint32 getWidth() const;
		uint32 getHeight() const;
		float32 getAspectRatio() const;

		bool hasAnimation() const 
		{ 
			return 
				this->animation.get() != nullptr &&
				this->animation->doesAnimate();
		}

		TextureAnimationPtr& getAnimation()
		{
			return this->animation;
		}

		Event onChanged;

	protected:

		void init(const std::string& filePath);

		RectF uvRect;
		TexturePtr texture;
		bool wrapU;
		bool wrapV;
		bool flipU;
		bool flipV;

		int32 targetType;
		bool targetUseDepth;

		TextureAnimationPtr animation;
		AnimationInstanceData animationInstance;

		bool atlased;


	};

	CLASS_DEFINITION_REFLECT(TextureHandleConst)
	public:
		TextureHandleConst()
			: texture(nullptr)
		{ }

		TextureHandleConst(const TextureHandleConstPtr& rhs)
			: texture(rhs->texture)
		{ }

		TextureHandleConst(const TextureHandlePtr& rhs)
			: texture(rhs->getTexture())
		{ }

		TextureHandleConst(const TexturePtr& ptr)
			: texture(ptr)
		{ }

	protected:

		TexturePtr texture;

	};
}
