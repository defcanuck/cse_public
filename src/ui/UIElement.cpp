#include "PCH.h"

#include "ui/UIElement.h"
#include "ui/UIAnimationPlayer.h"
#include "geom/Shape.h"
#include "global/ResourceFactory.h"
#include "global/Utils.h"
#include "global/Stats.h"

namespace cs
{
	const float32 UIElement::kDepthIncrement = 0.001f;

	UIElement::UIElement(const std::string& n)
		: parent(nullptr)
		, name(n)
		, visible(true)
		, enabled(true)
		, shouldClose(false)
		, depthBias(0.0f)
		, lastDepth(0.0f)
		, shader(CREATE_CLASS(ShaderHandle, defaultShaderResource))
		, span(UISpan())
		, screen_rect(RectF())
		, quadType(UIElementQuadSolid)
		, elementColor(255, 255, 255, 255)
		, sortValue(0)
		, drawData(CREATE_CLASS(BatchDrawData))
		, drawDirty(true)
		, decoratorsVisible(true)
		, hintWidth(false)
		, hintHeight(false)
		, angle(0.0f)
		, animations(nullptr)
	{
		for (int32 i = 0; i < kNumVertexColors; ++i)
		{
			this->vertexColor[i] = ColorB::White;
		}
		for (int32 i = 0; i < BATCH_TEXTURE_STAGES; ++i)
		{
			this->texture[i] == nullptr;
		}

		this->passMask.set(UIBatchPassMain);
		EngineStats::incrementStat(StatTypeUIElement);
	}

	UIElement::~UIElement()
	{
		EngineStats::decrementStat(StatTypeUIElement);
	}

	void UIElement::setEnabled(bool val)
	{
		this->enabled = val;
		// log::info(this->name, " set enabled ", (this->enabled) ? "true" : "false");
	}

	void UIElement::setEnabledRecursive(bool val)
	{
		this->setEnabled(val); for (auto& it : this->children) it->setEnabledRecursive(val);
	}

	void UIElement::setVisible(bool val)
	{
		this->visible = val;
		// log::info(this->name, " set visible ", (this->visible) ? "true" : "false");
	}

	void UIElement::setVisibleRecursive(bool val)
	{
		this->setVisible(val); for (auto& it : this->children) it->setVisibleRecursive(val);
	}

	ShaderHandlePtr& UIElement::getShader() 
	{ 
		return this->shader; 
	}

	RectF UIElement::getScreenRect()
	{
		if (this->parent)
		{
			RectF combinedRect = this->getScreenRect(parent->getScreenRect());
			return combinedRect;
		}
		return this->span.getRawRect();
	}

	RectF UIElement::getScreenRect(const RectF& parent_rect)
	{
		float32 textureAspectRatio = 0.0f;
		if (this->texture[BATCH_DEFAULT_STAGE].get())
		{
			textureAspectRatio = this->texture[BATCH_DEFAULT_STAGE]->getAspectRatio();
		}
		return this->span.evaluateToParent(parent_rect, this->hintWidth, this->hintHeight, textureAspectRatio);
	}

	PointF UIElement::getMaxDimensions()
	{
		RectF thisRect = this->span.getRawRect();
		PointF combineDimm = thisRect.size;
		this->combineChildrenDimm(combineDimm, thisRect.size);
		return combineDimm;
	}

	PointF UIElement::getMaxDimensions(const PointF& parent_dimm)
	{
		PointF thisDimm = this->span.evaluateDimensions(parent_dimm);
		PointF combineDimm = thisDimm;
		this->combineChildrenDimm(combineDimm, thisDimm);
		return combineDimm;
	}

	RectF UIElement::getMaxRelativeRect()
	{
		RectF thisRect = this->span.getRawRect();
		RectF combineRect = thisRect;

		this->combineChildrenRect(combineRect, thisRect);
		return combineRect;
	}

	RectF UIElement::getMaxRelativeRect(const RectF& parent_rect)
	{
		RectF raw = this->span.getRawRect();
		if (raw.size.w <= 0 || raw.size.h <= 0)
		{
			RectF combineRect = parent_rect;
			combineRect.pos = this->span.getRawRect().pos;
			this->combineChildrenRect(combineRect, parent_rect);
			return combineRect;
		}

		RectF thisRect = this->span.evaluateToParent(parent_rect);
		RectF combineRect = thisRect;

		this->combineChildrenRect(combineRect, thisRect);
		return combineRect;
	}

	RectF UIElement::getMaxRect()
	{
		if (this->parent)
		{
			return this->getMaxRect(this->parent->getScreenRect());
		}

		RectF thisRect = this->span.getRawRect();
		RectF combineRect;

		this->combineChildrenRect(combineRect, thisRect);
		combineRect = RectF(0.0f, 0.0f, combineRect.size.w, combineRect.size.h);
		return combineRect;
	}

	RectF UIElement::getMaxRect(const RectF& parent_rect)
	{
		RectF newRect = parent_rect;
		RectF thisRect = this->span.evaluateToParent(parent_rect);
		newRect.combine(thisRect);

		this->combineChildrenRect(newRect, thisRect);

		newRect = RectF(0.0f, 0.0f, newRect.size.w, newRect.size.h);
		return newRect;
	}

	void UIElement::combineChildrenDimm(PointF& combineDimm, const PointF& parent_dimm)
	{
		for (auto it : this->children)
		{
			combineDimm.combine(it->getMaxDimensions(parent_dimm));
		}
	}

	void UIElement::combineChildrenRect(RectF& combineRect, const RectF& parent_rect)
	{
		for (auto it : this->children)
		{
			combineRect.combine(it->getMaxRect(parent_rect));
		}
	}

	void UIElement::setShader(const std::string& str)
	{
		ShaderResourcePtr shaderResource = std::static_pointer_cast<ShaderResource>(
			ResourceFactory::getInstance()->loadResource<ShaderResource>(str));
		if (shaderResource.get())
		{
			ShaderHandlePtr shaderHandle = CREATE_CLASS(ShaderHandle, shaderResource);
			this->setShader(shaderHandle);
		}
		else
		{
			log::error("Cannot find shader with name ", str);
		}
	}

	bool UIElement::shouldDraw() const
	{
		return this->visible || (this->screen_rect.size.w <= 0 || this->screen_rect.size.h <= 0);
	}

	void UIElement::flushRemoved()
	{
		if (this->childrenToRemove.size() == 0)
			return;

		for (UIElementVector::iterator itrem = this->childrenToRemove.begin(); itrem != this->childrenToRemove.end(); ++itrem)
		{
			for (UIElementVector::iterator it = this->children.begin(); it != this->children.end(); it++)
			{
				if ((void*)(*it).get() == (void*)(*itrem).get())
				{
					(*it)->onDelete();
					this->children.erase(it);
					break;
				}
			}
		}
		this->childrenToRemove.clear();
	}


	void UIElement::traverse(traverseFunc func)
	{
		func(this);
		for (auto& it : this->children)
		{
			it->traverse(func);
		}
	}

	void UIElement::onFirstFrame()
	{
		for (auto& it : this->behaviors)
		{
			it->onFirstFrame();
		}
	}

	void UIElement::flipTextureHorizontal()
	{
		if (this->texture)
		{
			this->texture[BATCH_DEFAULT_STAGE]->setFlipHorizontal(true);
		}
	}

	void UIElement::flipTextureVertical()
	{
		if (this->texture)
		{
			this->texture[BATCH_DEFAULT_STAGE]->setFlipVertical(true);
		}
	}

	void UIElement::batch(
		BatchDrawList& display_list,
		uint32& numVertices,
		uint16& numIndices,
		const UIBatchProcessData& data,
		UIBatchProcessInfo& info)
	{

		if (this->animations.get())
		{
			this->animations->process(data.delta);
		}

		this->flushRemoved();

		this->screen_rect = this->getScreenRect(data.bounds);

		if (!this->shouldDraw() || RectF::excludes(data.screen, this->screen_rect))
		{
			return;
		}

		ColorB current_tint = data.tint * this->elementColor;

		if (data.pass == UIBatchPassMain)
		{
			UIBehaviorVector processBehaviors = this->behaviors;
			for (auto& it : processBehaviors)
			{
				(*it).process(this, data.delta);
			}
		}

		// update the rect if the underlying stuff changed
		if (this->drawDirty)
		{
			this->screen_rect = this->getScreenRect(data.bounds);
		}

		if (this->passMask.test(data.pass))
		{
			this->batchQuad(info, display_list, numVertices, numIndices, data.depth, current_tint);
		}

		float32 depthInc = data.depth + this->depthBias + kDepthIncrement;
		info.updateAndEvaluate(depthInc);

		if (this->decoratorsVisible && this->passMask.test(data.pass))
		{
			for (auto& it : this->decorators)
			{
				(*it).batch((uintptr_t) this, display_list, numVertices, numIndices, depthInc, this->screen_rect, current_tint, this->vertexColor[0], this->shader);
			}
		}

		this->batchChildren(display_list, numVertices, numIndices, current_tint, data, info);
	}

	void UIElement::batchChildren(
		BatchDrawList& display_list,
		uint32& numVertices,
		uint16& numIndices,
		const ColorB& tint,
		const UIBatchProcessData& data,
		UIBatchProcessInfo& info)
	{

		float32 depthInc = data.depth + kDepthIncrement;

		UIBatchProcessData child_data(data);
		child_data.depth = depthInc + this->depthBias;
		child_data.tint = tint;
		child_data.bounds = this->screen_rect;

		UIElementVector copyChildren = this->children;
		for (auto& it : copyChildren)
		{
			if (it->getVisible())
			{
				(*it).batch(display_list, numVertices, numIndices, child_data, info);
			}
		}
	}

	bool UIElement::prepareGUI(bool inWindow, size_t depth)
	{
		if (!this->getEnabled())
			return false;

		UIElementVector frameChildren = this->children;
		UIElementVector closedChildren;
		bool ret = false;
		for (auto it : frameChildren)
		{
			ret |= it->prepareGUI(inWindow, depth);
			if (it->readyToClose())
			{
				it->doClose();
				closedChildren.push_back(it);
			}
		}

		for (auto it : closedChildren)
		{
			this->removeChild(it);
		}

		return ret;
	}

	void UIElement::setVertexAlpha(float32 alpha)
	{
		this->vertexColor[0].a = (uchar)(255.0f * alpha);
		this->vertexColor[1].a = (uchar)(255.0f * alpha);
		this->vertexColor[2].a = (uchar)(255.0f * alpha);
		this->vertexColor[3].a = (uchar)(255.0f * alpha);
		this->refreshColors();
	}

	void UIElement::setVertexColor(ColorB vc)
	{
		if (this->vertexColor[0] != vc)
		{
			this->vertexColor[0] = vc;
			this->vertexColor[1] = vc;
			this->vertexColor[2] = vc;
			this->vertexColor[3] = vc;
			this->setDirty();
		}
	}

	void UIElement::setVertexColorAtIndex(ColorB vc, int32 idx)
	{
		assert(idx < kNumVertexColors);
		if (this->vertexColor[idx] != vc)
		{
			this->vertexColor[idx] = vc;
			this->setDirty();
		}
	}

	void UIElement::setVertexColors(const ColorBList& vc)
	{
		int32 max_col = std::min(4, static_cast<int32>(vc.size()));
		for (int32 i = 0; i < max_col; ++i)
		{
			this->vertexColor[i] = vc[i];
		}
		this->setDirty();
	}

	const ColorBList UIElement::getVertexColors() const
	{
		ColorBList vcols;
		vcols.push_back(this->vertexColor[0]);
		vcols.push_back(this->vertexColor[1]);
		vcols.push_back(this->vertexColor[2]);
		vcols.push_back(this->vertexColor[3]);
		return vcols;
	}

	void UIElement::buildGeometry()
	{
		this->drawData->indices.clear();
		QuadShape::generateIndicesImpl(this->drawData->indices);

		this->refreshPositions();
		this->refreshUV();
		this->refreshColors();
	}

	void UIElement::refreshPositions()
	{
		this->drawData->positions.clear();
		QuadShape::generatePositionsImpl(this->screen_rect, this->drawData->positions, 0.0f, this->angle);
	}

	void UIElement::refreshUV()
	{
		std::vector<vec2>* uvs[] =
		{
			&this->drawData->uvs0,
			&this->drawData->uvs1
		};

		for (size_t i = 0; i < BATCH_TEXTURE_STAGES; ++i)
		{
			(*uvs[i]).clear();
			if (this->texture[i])
			{
				const RectF& uvRect = texture[i]->getUVRect();
				QuadShape::generateUVImpl(uvRect, (*uvs[i]));
			}
		}
	}

	void UIElement::refreshColors()
	{
		this->drawData->vcolors.clear();
		this->drawData->vcolors.reserve(4);
		for (size_t i = 0; i < 4; i++)
		{
			this->drawData->vcolors.push_back(this->getColorByIndex(i));
		}
	}

	void UIElement::batchQuad(
		UIBatchProcessInfo& info,
		BatchDrawList& display_list,
		uint32& numVertices,
		uint16& numIndices,
		float32 depth,
		const ColorB& tint)
	{

		this->build();

		switch (this->quadType)
		{
			case UIElementQuadSolid:
			{
				if (this->drawData)
				{
					this->buildGeometry();
					this->drawDirty = false;
				}

				bool alphaMatch = fuzzyCompare(this->vertexColor[0].a, (uchar)0, (uchar)5);

				if ((this->vertexColor[0] != ColorB::Clear && !alphaMatch) || this->preCallback || this->postCallback)
				{

					float32 adj_depth = depth + this->depthBias;
					this->lastDepth = adj_depth;

					display_list.push_back(BatchDrawParams(this->drawData, numVertices));
					BatchDrawParams& params = display_list.back();
					params.tag = this->getName();
					params.bounds = this->screen_rect;
					params.transform = glm::translate(vec3(0.0f, 0.0f, adj_depth));
					params.tint = tint;
					params.postDrawCallback = this->postCallback;
					params.preDrawCallback = this->preCallback;
					params.depth = true;
					params.depthType = DepthLess;

					info.updateAndEvaluate(adj_depth);

					if (this->texture[BATCH_DEFAULT_STAGE].get())
					{
						for (int i = 0; i < BATCH_TEXTURE_STAGES; ++i)
							this->drawData->texture[i] = this->texture[i];
					}
					
					if (this->shader.get())
						this->drawData->shader = this->shader;

					numVertices += static_cast<uint32>(this->drawData->positions.size());
					numIndices += static_cast<uint16>(this->drawData->indices.size());

				}
				else
				{
					// log::info("Skipping draw");
				}
			}
		}

	}

	ColorB UIElement::getColorByIndex(int32 i)
	{
		int32 idx = std::min(i, kNumVertexColors - 1);
		return this->vertexColor[idx];
	}

	void UIElement::setTextureWrapHorizontal(bool wrap)
	{
		if (!this->texture[BATCH_DEFAULT_STAGE].get())
		{
			log::warning("No texture bound - wrap request ignored");
			return;
		}
		this->texture[BATCH_DEFAULT_STAGE]->setWrapHorizontal(wrap);
	}

	void UIElement::setTextureWrapVertical(bool wrap)
	{
		if (!this->texture[BATCH_DEFAULT_STAGE].get())
		{
			log::warning("No texture bound - wrap request ignored");
			return;
		}
		this->texture[BATCH_DEFAULT_STAGE]->setWrapVertical(wrap);
	}


	void UIElement::setTextureFlipHorizontal(bool flip)
	{
		if (!this->texture[BATCH_DEFAULT_STAGE].get())
		{
			log::warning("No texture bound - flip request ignored");
			return;
		}
		this->texture[BATCH_DEFAULT_STAGE]->setFlipHorizontal(flip);
	}

	void UIElement::setTextureFlipVertical(bool flip)
	{
		if (!this->texture[BATCH_DEFAULT_STAGE].get())
		{
			log::warning("No texture bound - flip request ignored");
			return;
		}
		this->texture[BATCH_DEFAULT_STAGE]->setFlipVertical(flip);
	}

	void UIElement::aspectMatchWidth(RectF& parentDimm)
	{
		if (!this->texture[BATCH_DEFAULT_STAGE].get())
			return;

		this->screen_rect = this->getScreenRect(parentDimm);
		float32 aspect = this->texture[BATCH_DEFAULT_STAGE]->getAspectRatio();
		this->setHeight(this->screen_rect.size.w * 1.0f / aspect, SpanPixels);
	}

	void UIElement::aspectMatchHeight(RectF& parentDimm)
	{
		if (!this->texture[BATCH_DEFAULT_STAGE].get())
			return;

		this->screen_rect = this->getScreenRect(parentDimm);
		float32 aspect = this->texture[BATCH_DEFAULT_STAGE]->getAspectRatio();
		this->setWidth(this->screen_rect.size.h * aspect, SpanPixels);
	}

	void UIElement::setDirty()
	{
		this->drawDirty = true;
		for (auto& it : this->children)
		{
			it->setDirty();
		}
	}

	void UIElement::preload()
	{
		for (auto& it : this->children)
		{
			it->preload();
		}
	}

	void UIElement::printChildren()
	{
		log::info("Children of Element ", this->getName());
		for (auto& it : this->children)
		{
			log::info(it->getName());
		}
	}

	void UIElement::setTexture(const std::string& filePath)
	{

		TexturePtr uiTexRes = std::static_pointer_cast<Texture>(
			ResourceFactory::getInstance()->loadResource<Texture>(filePath));

		this->texture[BATCH_DEFAULT_STAGE] = CREATE_CLASS(TextureHandle, uiTexRes);
		this->setDirty();
	}

	void UIElement::setTextureStage(const std::string& filePath, int32 stage)
	{

		TexturePtr uiTexRes = std::static_pointer_cast<Texture>(
			ResourceFactory::getInstance()->loadResource<Texture>(filePath));

		this->texture[stage] = CREATE_CLASS(TextureHandle, uiTexRes);
		this->setDirty();
	}

	void UIElement::setAndFitTexture(const std::string& fileName)
	{
		this->setTexture(fileName);
		this->fitTexture();
	}

	void UIElement::setAndFitTextureScaled(const std::string& fileName, float32 scale)
	{
		this->setTexture(fileName);
		this->fitTexture(scale);
	}

	void UIElement::clearTexture()
	{
		for (int i = 0; i < BATCH_TEXTURE_STAGES; ++i)
			this->texture[i] = nullptr;
		this->setDirty();
	}

	void UIElement::fitTexture(float32 scale)
	{
		if (this->texture[BATCH_DEFAULT_STAGE].get())
		{
			float32 w = static_cast<float32>(this->texture[BATCH_DEFAULT_STAGE]->getWidth());
			float32 h = static_cast<float32>(this->texture[BATCH_DEFAULT_STAGE]->getHeight());
			if (w > 0.0f && h > 0.0f)
			{
				this->setWidth(w * scale, SpanPoints);
				this->setHeight(h * scale, SpanPoints);
			}
			else
			{
				log::error("Bad dimensions for fit texture!");
			}
		}
		else
		{
			log::error("No texture to fit!");
		}
	}

	void UIElement::setTexture(TextureHandlePtr& tex)
	{
		this->texture[BATCH_DEFAULT_STAGE] = CREATE_CLASS(TextureHandle, tex);
		this->setDirty();
	}

	void UIElement::setRawWidth(float32 value)
	{
		this->span.setWidth(value);
		this->setDirty();
	}

	void UIElement::setRawHeight(float32 value)
	{
		this->span.setHeight(value);
		this->setDirty();
	}

	void UIElement::setRawXPosition(float32 value)
	{
		this->span.setX(value, SpanNone);
		this->setDirty();
	}

	void UIElement::setRawYPosition(float32 value)
	{
		this->span.setY(value, SpanNone);
		this->setDirty();
	}

	float32 UIElement::getRawWidth() const
	{
		return this->span.getRawWidth().value;
	}

	float32 UIElement::getRawHeight() const
	{
		return this->span.getRawHeight().value;
	}

	float32 UIElement::getRawXPosition() const
	{
		return this->span.getRawX().value.value;
	}

	float32 UIElement::getRawYPosition() const
	{
		return this->span.getRawY().value.value;
	}

	void UIElement::setPosition(const vec2& pos, HAlign halign, VAlign valign, SpanMeasure measureX, SpanMeasure measureY)
	{
		this->span.setPosition(pos, halign, valign, measureX, measureY);
		this->setDirty();

	}

	void UIElement::setPosition(const PointF& pos, HAlign halign, VAlign valign, SpanMeasure measureX, SpanMeasure measureY)
	{
		this->span.setPosition(vec2(pos.x, pos.y), halign, valign, measureX, measureY);
		this->setDirty();

	}

	void UIElement::setPosition(float32 x, float32 y, HAlign halign, VAlign valign, SpanMeasure measureX, SpanMeasure measureY)
	{
		this->span.setPosition(vec2(x, y), halign, valign, measureX, measureY);
		this->setDirty();

	}

	void UIElement::setXPosition(float32 x, HAlign align, SpanMeasure measure)
	{
		this->span.setX(x, align, measure);
		this->setDirty();

	}

	void UIElement::setYPosition(float32 y, VAlign align, SpanMeasure measure)
	{
		this->span.setY(y, align, measure);
		this->setDirty();

	}

	vec2 UIElement::getAbsolutePosition() const
	{
		return vec2(toVec2(this->screen_rect.pos));
	}

	bool UIElement::onCursor(ClickInput input, ClickParams& params, UIClickResults& results, uint32 depth)
	{

		if (!this->enabled)
		{
			if (input == ClickPrimary && rectContains<float32, vec2>(this->screen_rect, params.position))
			{
				// log::info(this->name, " element disabled - click ignored");
			}
			return false;
		}

		if (this->behaviors.size() > 0)
		{
			if (rectContains<float32, vec2>(this->screen_rect, params.position))
			{
				if (params.state == TouchDown)
				{
					log::info("Touched ", this->getName());
				}

				UIElementClick result;
				result.state = params.state;
				result.inputType = input;
				result.absolutePos = params.position;
				result.relativePos = params.position;
				result.relativePct = rectPercent<float32, vec2>(this->screen_rect, params.position);
				result.depth = depth;
				results[this] = result;

			}
			else {

				this->onExit(input);
				for (auto it : this->children)
					(*it).onExit(input);
			}
		}

		bool ret = false;
		for (auto& it : this->children)
			ret |= (*it).onCursor(input, params, results, depth + 1);

		return ret;
	}

	bool UIElement::onEnter(UIElementClick& click)
	{
		if (!this->enabled)
			return false;

		for (auto it : this->behaviors)
		{
			if (it->onEnter(this, click))
				return true;
		}
		return false;
	}

	bool UIElement::onUpdate(vec2& screen_pos)
	{
		for (auto it : this->behaviors)
		{
			if ((this->enabled || it->canProcessDisabled()) && it->onUpdate(this, screen_pos))
				return true;
		}

		return false;
	}

	void UIElement::onExit(ClickInput input)
	{
		if (!this->enabled)
			return;

		for (auto it : this->behaviors)
		{
			it->onExit(this, input);
		}
	}

	void UIElement::bubbleLayoutUp()
	{
		this->updateLayoutImpl();

		for (auto it : this->children)
			it->bubbleLayoutUp();
	}

	void UIElement::setFlag(UIElementFlag flag) 
	{ 
		if (!flags.test(flag) && this->flagCallback[flag].get())
		{
			(*this->flagCallback[flag])(true);
		}
		this->flags.set(flag); 
	}
	
	void UIElement::unsetFlag(UIElementFlag flag) 
	{ 
		if (flags.test(flag) && this->flagCallback[flag].get())
		{
			(*this->flagCallback[flag])(false);
		}
		this->flags.unset(flag);
	}

	void UIElement::setFlagCallback(UIElementFlag flag, LuaCallbackPtr& callback)
	{
		this->flagCallback[flag] = callback;
	}

	void UIElement::addChildImpl(UIElementPtr& ptr)
	{
		this->children.push_back(ptr);
		ptr->parent = this;
	}

	void UIElement::removeChildImpl(UIElementPtr& ptr)
	{
		for (UIElementVector::iterator it = this->children.begin(); it != this->children.end(); it++)
		{
			if ((void*)(*it).get() == (void*)ptr.get())
			{
				this->childrenToRemove.push_back(ptr);
				return;
			}
		}
	}
	
	void UIElement::bubbleLayoutDown(void* parent_ptr)
	{
		for (auto it : this->children)
			if (it.get() != parent_ptr)
				it->bubbleLayoutUp();

		this->updateLayoutImpl();

		if (this->parent)
			this->parent->bubbleLayoutDown(this);
		
	}

	void UIElement::updateLayout(void* parent_ptr)
	{
		this->bubbleLayoutUp();
		this->updateLayoutImpl();
		this->bubbleLayoutDown(this);
	}

	void UIElement::onDelete()
	{
		// kludge - animPlayer may have circular reference
		// prevent leak and unbind it
		this->parent = nullptr;
		this->removeAllChildren();
	}

	void UIElement::startAnimation(std::shared_ptr<UIAnimationPlayer>& uiAnimation)
	{
		
	}

	void UIElement::prepareToUnload()
	{
		if (this->animations.get())
		{
			this->animations->clear();
			this->animations = nullptr;
		}
		for (auto& it : this->children)
		{
			it->prepareToUnload();
		}
	}

	std::shared_ptr<UIAnimationPlayer> UIElement::getAnimationPlayer()
	{
		if (!this->animations)
		{
			this->animations = std::make_shared<UIAnimationPlayer>();
		}
		return this->animations;
	}
}
