#include "PCH.h"

#include "gfx/TextureHandle.h"
#include "gfx/RenderTarget.h"

#include "global/ResourceFactory.h"


namespace cs
{
	DEFINE_META_VECTOR(TextureAnimationSheet::AnimationSheetFrames, RectF, AnimationSheetFrames);

	BEGIN_META_CLASS(TextureAnimation)
		ADD_MEMBER(flipU);
	SET_MEMBER_DEFAULT(false);
	SET_MEMBER_CALLBACK_POST(&TextureAnimation::update);
	ADD_MEMBER(flipV);
	SET_MEMBER_DEFAULT(false);
	SET_MEMBER_CALLBACK_POST(&TextureAnimation::update);
	END_META();

	BEGIN_META_CLASS(TextureAnimationSheet)

		ADD_MEMBER(numRows);
	SET_MEMBER_NO_SLIDER();
	SET_MEMBER_CALLBACK_POST(&TextureAnimationSheet::updateImpl);

	ADD_MEMBER(numColumns);
	SET_MEMBER_NO_SLIDER();
	SET_MEMBER_CALLBACK_POST(&TextureAnimationSheet::updateImpl);
	ADD_MEMBER(speed);
	SET_MEMBER_MIN(0.1f);
	SET_MEMBER_MAX(30.0f);
	ADD_MEMBER(frames);
	SET_MEMBER_IGNORE_GUI();
	ADD_MEMBER(numFrames);
	ADD_MEMBER(rowMajor);
	SET_MEMBER_DEFAULT(false);
	SET_MEMBER_CALLBACK_POST(&TextureAnimationSheet::onSheetDimensionsChanged);
	END_META();

	BEGIN_META_CLASS(TextureHandle)

		ADD_MEMBER(uvRect);
	SET_MEMBER_CALLBACK_POST(&TextureHandle::onTextureChanged);
	ADD_MEMBER_RESOURCE(texture);
	SET_MEMBER_CALLBACK_POST(&TextureHandle::onTextureChanged);

	ADD_MEMBER(wrapU);
	SET_MEMBER_DEFAULT(false);

	ADD_MEMBER(wrapV);
	SET_MEMBER_DEFAULT(false);

	ADD_MEMBER(flipU);
	SET_MEMBER_DEFAULT(false);
	SET_MEMBER_CALLBACK_POST(&TextureHandle::onTextureChanged);

	ADD_MEMBER(flipV);
	SET_MEMBER_DEFAULT(false);
	SET_MEMBER_CALLBACK_POST(&TextureHandle::onTextureChanged);

	ADD_MEMBER_PTR(animation);
	ADD_COMBO_META_LABEL(TextureAnimation, "None");
	ADD_COMBO_META_LABEL(TextureAnimationSheet, "Animation Sheet");
	SET_MEMBER_COLLAPSEABLE();
	SET_MEMBER_START_COLLAPSED();

	ADD_MEMBER(targetType);
	SET_MEMBER_DEFAULT(-1);
	SET_MEMBER_IGNORE_GUI();

	ADD_MEMBER(atlased);
	SET_MEMBER_DEFAULT(false);
	SET_MEMBER_CALLBACK_POST(&TextureHandle::onTextureChanged);

	ADD_META_FUNCTION("Fit UV", &TextureHandle::fitUV);

	END_META()

		BEGIN_META_CLASS(TextureHandleConst)

		ADD_MEMBER_RESOURCE(texture);
	SET_MEMBER_CONST();

	END_META()

		const RectF& TextureAnimation::getRectForIndex(size_t index) const
	{
		return TextureHandle::kDefaultTextureSampleRect;
	}

	const RectF& TextureAnimation::getRectForPercent(AnimationInstanceData& data, float32 dt) const
	{
		return TextureHandle::kDefaultTextureSampleRect;
	}

	const RectF& TextureAnimation::getRectForDelta(AnimationInstanceData& data, float32 dt)
	{
		return TextureHandle::kDefaultTextureSampleRect;
	}

	void TextureAnimationSheet::onNew()
	{
		this->onSheetDimensionsChanged();
		this->numFrames = this->numRows * this->numColumns;
	}

	void TextureAnimationSheet::updateImpl()
	{
		this->onSheetDimensionsChanged();
		this->numFrames = this->numRows * this->numColumns;
	}

	void TextureAnimationSheet::onSheetDimensionsChanged()
	{
		this->numRows = std::max<int32>(this->numRows, 1);
		this->numColumns = std::max<int32>(this->numColumns, 1);

		this->frames.clear();
		float32 yInc = 1.0f / float32(this->numRows);
		float32 xInc = 1.0f / float32(this->numColumns);

		if (this->rowMajor)
		{
			for (int32 c = this->numColumns - 1; c >= 0; c--)
			{
				for (int32 r = 0; r < this->numRows; r++)
				{
					float32 x = c * xInc;
					float32 y = (this->numRows - r - 1) * yInc;

					float32 useX = (this->flipU) ? x + xInc : x;
					float32 useY = (this->flipV) ? y + yInc : y;
					float32 useW = (this->flipU) ? -xInc : xInc;
					float32 useH = (this->flipV) ? -yInc : yInc;
					this->frames.push_back(RectF(useX, useY, useW, useH));
				}
			}
		}
		else
		{
			for (int32 r = this->numRows - 1; r >= 0; r--)
			{
				for (int32 c = 0; c < this->numColumns; c++)
				{
					float32 x = c * xInc;
					float32 y = (this->numRows - r - 1) * yInc;

					float32 useX = (this->flipU) ? x + xInc : x;
					float32 useY = (this->flipV) ? y + yInc : y;
					float32 useW = (this->flipU) ? -xInc : xInc;
					float32 useH = (this->flipV) ? -yInc : yInc;
					this->frames.push_back(RectF(useX, useY, useW, useH));
				}
			}
		}

		if (this->numFrames == -1)
		{
			this->numFrames = this->numRows * this->numColumns;
		}
	}

	void TextureAnimationSheet::onPostLoad(const LoadFlagMask& flags)
	{
		this->onSheetDimensionsChanged();
	}

	const RectF& TextureAnimationSheet::getRectForDelta(AnimationInstanceData& data, float32 dt)
	{
		data.curDelta += this->speed * dt;

		size_t curIndex = size_t(data.curDelta) % this->numFrames;
		return getRectForIndex(std::min<size_t>(curIndex, this->numFrames - 1));
	}

	const RectF& TextureAnimationSheet::getRectForIndex(size_t index) const
	{
		assert(index < this->frames.size());
		assert(index < size_t(this->numFrames));

		return this->frames[index];
	}

	const RectF& TextureAnimationSheet::getRectForPercent(AnimationInstanceData& data, float32 pct) const
	{
		pct = clamp<float32>(0.0f, 1.0f, pct);
		size_t index = std::min<size_t>(size_t(this->numFrames * pct), numFrames - 1);
		return this->getRectForIndex(index);
	}

	const RectF TextureHandle::kDefaultTextureSampleRect(0.0f, 1.0f, 1.0f, -1.0f);
	const RectF TextureHandle::kDefaultTextureSampleRectSafe(0.01f, 0.99f, 0.99f, -0.99f);
	const RectF TextureHandle::kDefaultRenderTextureSampleRect(0.0f, 0.0f, 1.0f, 1.0f);

	TextureHandle::TextureHandle()
		: uvRect(kDefaultTextureSampleRect)
		, texture(nullptr)
		, wrapU(false)
		, wrapV(false)
		, flipU(false)
		, flipV(false)
		, targetType(RenderTargetTypeNone)
		, targetUseDepth(false)
		, atlased(false)
	{
	}

	TextureHandle::TextureHandle(const TextureHandlePtr& rhs)
		: uvRect(rhs->uvRect)
		, texture(rhs->texture)
		, wrapU(rhs->wrapU)
		, wrapV(rhs->wrapV)
		, flipU(rhs->flipU)
		, flipV(rhs->flipV)
		, targetType(RenderTargetTypeNone)
		, targetUseDepth(false)
		, atlased(false)
	{ }

	TextureHandle::TextureHandle(const TexturePtr& ptr)
		: uvRect(kDefaultTextureSampleRect)
		, texture(ptr)
		, wrapU(false)
		, wrapV(false)
		, flipU(false)
		, flipV(false)
		, targetType(RenderTargetTypeNone)
		, targetUseDepth(false)
		, atlased(false)
	{
		if (this->texture.get())
		{
			this->texture->adjustRect(this->uvRect);
		}
	}

	TextureHandle::TextureHandle(const TexturePtr& ptr, RectF& rect)
		: uvRect(rect)
		, texture(ptr)
		, wrapU(false)
		, wrapV(false)
		, flipU(false)
		, flipV(false)
		, targetType(RenderTargetTypeNone)
		, targetUseDepth(false)
		, atlased(false)
	{

	}

	TextureHandle::TextureHandle(const std::string& fileName)
		: uvRect(kDefaultTextureSampleRect)
		, texture(nullptr)
		, wrapU(false)
		, wrapV(false)
		, flipU(false)
		, flipV(false)
		, targetType(RenderTargetTypeNone)
		, targetUseDepth(false)
		, atlased(false)
	{
		this->init(fileName);
	}

	TextureHandle::TextureHandle(RenderTargetType targetType, bool useDepth)
		: uvRect(kDefaultRenderTextureSampleRect)
		, texture((useDepth) ? RenderTargetManager::getInstance()->getTarget(targetType)->getDepthTexture() : RenderTargetManager::getInstance()->getTarget(targetType)->getTexture())
		, wrapU(false)
		, wrapV(false)
		, flipU(false)
		, flipV(false)
		, targetType(targetType)
		, targetUseDepth(useDepth)
		, atlased(false)
	{
		if (this->texture.get())
		{
			this->texture->adjustRect(this->uvRect);
		}
	}

	TextureHandle::~TextureHandle()
	{

	}

	void TextureHandle::onPostLoad(const LoadFlagMask& flags)
	{
		if (this->animation.get())
		{
			this->animation->onPostLoad(flags);
		}

		if (targetType != RenderTargetTypeNone)
		{
			this->texture = (this->targetUseDepth) ?
				RenderTargetManager::getInstance()->getTarget((RenderTargetType) this->targetType)->getDepthTexture() :
				RenderTargetManager::getInstance()->getTarget((RenderTargetType) this->targetType)->getTexture();
			assert(this->texture.get());
		}
	}

	void TextureHandle::init(const std::string& fileName)
	{
		this->texture = std::static_pointer_cast<Texture>(
			ResourceFactory::getInstance()->loadResource<Texture>(fileName));

		this->fitUV();
	}

	void TextureHandle::bind(uint32 stage)
	{
		if (this->texture)
			this->texture->bind(stage, this->wrapU, this->wrapV);
	}


	void TextureHandle::fitUV()
	{
		if (this->texture.get())
		{
			this->texture->adjustRect(this->uvRect);
			this->onChanged.invoke();
		}
	}

	uint32 TextureHandle::getWidth() const
	{
		return (this->texture.get()) ? this->texture->getWidth() : 0;
	}

	uint32 TextureHandle::getHeight() const
	{
		return (this->texture.get()) ? this->texture->getHeight() : 0;
	}


	float32 TextureHandle::getAspectRatio() const
	{
		if (this->getHeight() == 0)
			return 0.0f;

		return this->getWidth() / float32(this->getHeight());
	}

	bool TextureHandle::equals(const TextureHandlePtr& rhs)
	{
		if (!this->texture)
			return false;

		return this->texture->equals(rhs->getTexture());
	}

	const std::string TextureHandle::getTextureName() const
	{
		if (!this->texture)
			return "empty_texture";

		return this->texture->getName();
	}

	void TextureHandle::onTextureChanged()
	{
		this->onChanged.invoke();
	}

	RectF TextureHandle::getUVRect(float32 delta)
	{
		if (this->hasAnimation())
			return this->animation->getRectForDelta(this->animationInstance, delta);

		RectF retRect = this->uvRect;
		if (this->flipU)
		{
			retRect.flipHorizontal();
		}
		if (this->flipV)
		{
			retRect.flipVertical();
		}
		return retRect;
	}

	vec2 TextureHandle::getUVByIndex(uint32 idx, float32 dt)
	{
		return toVec2(this->uvRect.getByCorner((RectCorner)idx));
	}

	void TextureHandle::getCorners(vec2* corners, float32 dt)
	{
		if (this->animation.get() && this->animation->hasAnimation())
		{
			const RectF& animRect = this->animation->getRectForPercent(this->animationInstance, dt);
			for (size_t i = 0; i < CornersMAX; i++)
			{
				corners[i] = toVec2(animRect.getByCorner(RectCorner(i)));
			}
			return;
		}

		for (size_t i = 0; i < CornersMAX; i++)
		{
			corners[i] = toVec2(this->uvRect.getByCorner(RectCorner(i)));
		}
	}

	void TextureHandle::getCorners(vec2* corners, int32 frameIndex)
	{
		if (this->animation.get() && this->animation->hasAnimation())
		{
			const RectF& animRect = this->animation->getRectForIndex(frameIndex);
			for (size_t i = 0; i < CornersMAX; i++)
			{
				corners[i] = toVec2(animRect.getByCorner(RectCorner(i)));
			}
			return;
		}

		for (size_t i = 0; i < CornersMAX; i++)
		{
			corners[i] = toVec2(this->uvRect.getByCorner(RectCorner(i)));
		}
	}

	void TextureHandle::setUVRect(const RectF& rect)
	{
		this->uvRect = rect;
		this->onChanged.invoke();
	}


	void TextureHandle::squishWidth(float32 pct)
	{
		float adjustWidth = this->uvRect.size.w * (1.0 - pct) * 0.5f;
		this->uvRect.pos.x += adjustWidth;
		this->uvRect.size.w -= adjustWidth * 2.0f;
	}

	void TextureHandle::squishHeight(float32 pct)
	{
		float adjustHeight = this->uvRect.size.h * (1.0 - pct) * 0.5f;
		this->uvRect.pos.y += adjustHeight;
		this->uvRect.size.h -= adjustHeight * 2.0f;
	}

	void TextureHandle::shiftHorizontal(float32 pct)
	{
		float adjustWidth = this->uvRect.size.w * pct;
		this->uvRect.pos.x += adjustWidth;
	}

	void TextureHandle::shiftVertical(float32 pct)
	{
		float adjustHeight = this->uvRect.size.h * pct;
		this->uvRect.pos.y += adjustHeight;
	}
    
    void TextureHandle::setUVRectSafe()
    {
        this->uvRect = kDefaultTextureSampleRectSafe;
        this->onChanged.invoke();
    }

	void TextureHandle::setFlipHorizontal(bool flip)
	{
		if (this->flipU != flip)
		{
			this->flipU = flip;
			this->onChanged.invoke();
		}
	}

	void TextureHandle::setFlipVertical(bool flip)
	{
		if (this->flipV != flip)
		{
			this->flipV = flip;
			this->onChanged.invoke();
		}
	}
}
