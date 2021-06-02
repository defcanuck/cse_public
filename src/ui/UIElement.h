#pragma once

#include "ClassDef.h"

#include "math/Rect.h"
#include "math/GLM.h"

#include "ui/UIGlobal.h"
#include "ui/UISpan.h"
#include "ui/UIBehavior.h"
#include "ui/UIDecorator.h"

#include "os/Input.h"

#include <map>

namespace cs
{
	class UIElement;

	enum UIElementFlag
	{
		UIElementFlagAnimating,
		//...
		UIElementFlagMAX
	};

	enum UIElementQuadType
	{
		UIElementQuadSolid,
		UIElementQuadComponent,
		UIElementQuadTypeMAX
	};

	enum UIElementQuadTextureSlot
	{
		UIElementQuadTextureSlotCenter,
		UIElementQuadTextureSlotCornerTopLeft,
		UIElementQuadTextureSlotCornderTopRight,
		UIElementQuadTextureSlotCornerBotLeft,
		UIElementQuadTextureSlotCornerBotRight,
		UIElementQuadTextureSlotLeft,
		UIElementQuadTextureSlotRight,
		UIElementQuadTextureSlotTop,
		UIElementQuadTextureSlotBottom,
		UIElementQuadTextureSlotMAX
	};

	typedef std::unordered_map<UIElementQuadTextureSlot, TextureHandlePtr> UIElementQuadTextureSlotMap;

	typedef BitMask<UIElementFlag, UIElementFlagMAX> UIElementFlags;

	struct UIBatchProcessInfo
	{
		UIBatchProcessInfo()
			: maxDepth(0.0f)
			, numElements(0)
		{ }

		inline void updateAndEvaluate(float32 depth)
		{
			this->maxDepth = (depth > this->maxDepth) ? depth : this->maxDepth;
			this->numElements++;
		}

		inline void incrementElementCount(size_t count)
		{
			this->numElements += count;
		}

		float32 getMaxDepth() const { return this->maxDepth; }
		size_t getNumElements() const { return this->numElements; }

	private:

		float32 maxDepth;
		size_t numElements;
	};

	enum UIBatchPass
	{
		UIBatchPassMain,
		UIBatchPassStencil,
		UIBatchPassMAX
	};

	typedef BitMask<UIBatchPass, UIBatchPassMAX> UIBatchPassMask;

	struct UIBatchProcessData
	{
		UIBatchProcessData()
			: depth(0.0f)
			, delta(0.0f)
			, tint(ColorB::White)
			, bounds(RectF()) 
			, animating(false) 
			, screen()
			, pass(UIBatchPassMain)
		{ }

		UIBatchProcessData(const UIBatchProcessData& rhs)
			: depth(rhs.depth)
			, delta(rhs.delta)
			, tint(rhs.tint)
			, bounds(rhs.bounds)
			, animating(rhs.animating)
			, screen(rhs.screen)
			, pass(rhs.pass)
		{ }

		bool animating;
		float32 depth;
		float32 delta;
		ColorB tint;
		RectF bounds;
		RectF screen;
		UIBatchPass pass;
	};

	class UIAnimationPlayer;

	CLASS_DEFINITION(UIElement)

	public:

		typedef void(*traverseFunc)(UIElement*);

		UIElement(const std::string& n);
		virtual ~UIElement();

		virtual bool shouldDraw() const;
		virtual void batch(
			BatchDrawList& display_list,
			uint32& numVertices,
			uint16& numIndices,
			const UIBatchProcessData& data, 
			UIBatchProcessInfo& info);

		virtual void setTexture(const std::string& fileName);
		virtual void setTextureStage(const std::string& fileName, int32 stage);
		virtual void setAndFitTexture(const std::string& fileName);
		virtual void setAndFitTextureScaled(const std::string& fileName, float32 scale = 1.0f);
		virtual void setTexture(TextureHandlePtr& tex);
		virtual TextureHandlePtr& getTexture() { return this->texture[BATCH_DEFAULT_STAGE]; }
		virtual void clearTexture();
		virtual void fitTexture(float32 scale = 1.0f);

		void setHintWidth(bool value) { this->hintWidth = value; }
		void setHintHeight(bool value) { this->hintHeight = value; }

		virtual void setShader(const ShaderHandlePtr& sh) { this->shader = sh; }
		virtual void setShader(const std::string& str);
		virtual ShaderHandlePtr& getShader();

		const std::string& getName() const { return this->name; }
		const UIElementFlags& getFlags() { return this->flags; }
		void setFlag(UIElementFlag flag);
		void unsetFlag(UIElementFlag flag);
		void setFlagCallback(UIElementFlag flag, LuaCallbackPtr& callback);

		void prepareToUnload();

		void traverse(traverseFunc func);

		void onFirstFrame();

		virtual void setEnabled(bool val);
		virtual void setEnabledRecursive(bool val);
		virtual void setVisible(bool val);
		virtual void setVisibleRecursive(bool val);
		virtual bool getEnabled() const { return this->enabled; }
		virtual bool getVisible() const { return this->visible; }

		void setTextureWrapHorizontal(bool wrap);
		void setTextureWrapVertical(bool wrap);

		void setTextureFlipHorizontal(bool flip);
		void setTextureFlipVertical(bool flip);

		void aspectMatchWidth(RectF& parentDimm);
		void aspectMatchHeight(RectF& parentDimm);

		float32 getLastDepth() { return this->lastDepth; }
		static float32 getDepthIncrement() { return UIElement::kDepthIncrement; }

		template <typename T>
		void setWidth(T width, SpanMeasure measure = SpanPixels)
		{
			this->span.setWidth((float32) width, measure);
		}

		template <typename T>
		void setHeight(T height, SpanMeasure measure = SpanPixels)
		{
			this->span.setHeight((float32) height, measure);
		}

	
		template <class T>
		void addChild(std::shared_ptr<T>& elem)
		{
			UIElementPtr elemPtr = std::static_pointer_cast<UIElement>(elem);
			assert(this != elem.get());
			this->addChildImpl(elemPtr);
		}

		template <class T>
		void removeChild(std::shared_ptr<T>& elem)
		{
            UIElementPtr baseElem = std::static_pointer_cast<UIElement>(elem);
			this->removeChildImpl(baseElem);
		}

		void removeAllChildren()
		{
			for (auto& it : this->children)
			{
				it->onDelete();
			}
			this->children.clear();
		}
    
        UIElementPtr getChildByIndex(int32 idx)
        {
            if (int(this->children.size()) <= idx)
            {
                log::error("No child at index ", idx);
                return UIElementPtr();
            }
            return this->children[idx];
        }

		UIElementPtr getChildByName(const std::string& n)
		{
			for (size_t i = 0; i < this->children.size(); ++i)
			{
				if (this->children[i]->getName() == n)
					return this->children[i];
			}
			return UIElementPtr();
		}

		bool removeChildByName(const std::string& n)
		{
			for (size_t i = 0; i < this->children.size(); ++i)
			{
				if (this->children[i]->getName() == n)
				{
					this->removeChildImpl(this->children[i]);
					return true;
				}
			}
			return false;
		}

		bool childExists(const std::string& n)
		{
			for (size_t i = 0; i < this->children.size(); ++i)
			{
				if (this->children[i]->getName() == n)
				{
					return true;
				}
			}
			return false;
		}

		virtual void preload();
		void printChildren();

		void setRawWidth(float32 value);
		void setRawHeight(float32 value);
		void setRawXPosition(float32 value);
		void setRawYPosition(float32 value);

		float32 getRawWidth() const;
		float32 getRawHeight() const;
		float32 getRawXPosition() const;
		float32 getRawYPosition() const;

		void setPosition(const vec2& pos, HAlign halign = HAlignNone, VAlign valign = VAlignNone, SpanMeasure measureX = SpanNone, SpanMeasure measureY = SpanNone);
		void setPosition(const PointF& pos, HAlign halign = HAlignNone, VAlign valign = VAlignNone, SpanMeasure measureX = SpanNone, SpanMeasure measureY = SpanNone);
		void setPosition(float32 x, float32 y, HAlign halign = HAlignNone, VAlign valign = VAlignNone, SpanMeasure measureX = SpanNone, SpanMeasure measureY = SpanNone);

		void setXPosition(float32 x, HAlign align = HAlignNone, SpanMeasure measure = SpanNone);
		void setYPosition(float32 y, VAlign align = VAlignNone, SpanMeasure measure = SpanNone);

		void setDepthBias(float32 bias) { this->depthBias = bias; }

		vec2 getAbsolutePosition() const;

		RectF getScreenRect();
		RectF getScreenRect(const RectF& parent_rect);

		RectF getMaxRect();
		RectF getMaxRect(const RectF& parent_rect);

		RectF getMaxRelativeRect();
		RectF getMaxRelativeRect(const RectF& parent_rect);

		PointF getMaxDimensions();
		PointF getMaxDimensions(const PointF& parent_dimm);

		void flipTextureHorizontal();
		void flipTextureVertical();

		void startAnimation(std::shared_ptr<UIAnimationPlayer>& uiAnimation);

		void setVertexColor(ColorB vc);
		void setVertexColorAtIndex(ColorB vc, int32 idx);

		const ColorB getVertexColor() const { return this->vertexColor[0]; }

		void setVertexColors(const ColorBList& vc);
		const ColorBList getVertexColors() const;

		void setElementColor(ColorB ec) { this->elementColor = ec; }
		const ColorB& getElementColor() const { return this->elementColor; }

		void setVertexAlpha(float32 alpha);

		float32 getVertexAlpha() const
		{
			return this->vertexColor[0].a / 255.0f;
		}

		virtual void setElementAlpha(float32 alpha)
		{
			this->elementColor.a = (uchar)(255.0f * alpha);
			this->refreshColors();
		}

		float32 getElementAlpha() const
		{
			return this->vertexColor[0].a / 255.0f;
		}

		virtual void onSetAction() { }
		virtual void onDesetAction() { }

		template <class T>
		void addBehavior(T& ptr)
		{
			this->behaviors.push_back(std::static_pointer_cast<UIBehavior>(ptr));
		}

		void setDecoratorsVisible(bool b) { this->decoratorsVisible = b; }

		template <class T>
		void addDecorator(T & ptr)
		{
			this->decorators.push_back(std::static_pointer_cast<UIDecorator>(ptr));
		}

		void clearBehaviors()
		{
			this->behaviors.clear();
		}

		void clearDecorators()
		{
			this->decorators.clear();
		}

		virtual bool onCursor(ClickInput input, ClickParams& params, UIClickResults& results, uint32 depth = 0);

		virtual bool onEnter(UIElementClick& click);
		virtual bool onUpdate(vec2& screen_pos);
		virtual	void onExit(ClickInput input);

		virtual void trigger()
		{
			this->setEnabled(true);
			this->setVisible(true);
		}

		virtual void triggerRecursive()
		{
			this->setEnabled(true);
			this->setVisible(true);
			for (auto& it : this->children)
				it->triggerRecursive();
		}

		virtual void toggle()
		{
			this->setEnabled(!this->enabled);
			this->setVisible(!this->visible);
		}

		virtual void toggleRecursive()
		{
			this->toggle();
			for (auto& it : this->children)
			{
				it->setEnabled(this->enabled);
				it->setVisible(this->visible);
			}
		}

		virtual void close()
		{
			this->setEnabled(false);
			this->setVisible(false);
		}

		virtual void closeRecursive()
		{
			this->setEnabled(false);
			this->setVisible(false);
			for (auto& it : this->children)
				it->closeRecursive();
		}

		virtual void show(bool val)
		{
			this->setEnabled(val);
		}

		virtual void showRecursive(bool val)
		{
			this->show(val);
			for (auto& it : this->children)
			{
				it->showRecursive(val);
			}
		}

		void doClose() { this->doCloseImpl(); }
		virtual bool readyToClose() const { return this->shouldClose; }
		virtual void setReadyToClose(bool sc) { this->shouldClose = sc; }

		void updateLayout(void* parent_ptr = nullptr);

		virtual bool prepareGUI(bool inWindow = false, size_t depth = 0);

		void setPreCallback(DrawCall::PreDraw callback) { this->preCallback = callback; }
		void setPostCallback(DrawCall::PostDraw callback) { this->postCallback = callback; }

		template <class T>
		void setUniformValue(std::string name, T value)
		{
			if (this->shader)
			{
				this->shader->setUniformValue<T>(name, value);
			}
		}

		void setAngleRadians(float32 a) { this->angle = a; this->drawDirty = true; }
		void setAngleDegrees(float32 deg) { this->angle = degreesToRadians(deg);  this->drawDirty = true; }

		void flushRemoved();

		void setRenderOnPass(UIBatchPass pass) { this->passMask.set(pass); }
		bool isPassSet(UIBatchPass pass) { return this->passMask.test(pass); }

		std::shared_ptr<UIAnimationPlayer> getAnimationPlayer();

	protected:

		virtual void setDirty();
		void onDelete();

		virtual void addChildImpl(UIElementPtr& ptr);
		virtual void removeChildImpl(UIElementPtr& ptr);

		virtual void bubbleLayoutUp();
		virtual void bubbleLayoutDown(void* parent_ptr);

		virtual void updateLayoutImpl() { }

		const static float32 kDepthIncrement;

		virtual void combineChildrenRect(RectF& combineRect, const RectF& parent_rect);
		virtual void combineChildrenDimm(PointF& combineDimm, const PointF& parent_dimm);

		virtual ColorB getColorByIndex(int32 idx);

		virtual void doCloseImpl() { }

		virtual void build() { }

		void buildGeometry();
		void refreshPositions();
		void refreshUV();
		void refreshColors();

		virtual void batchQuad(
			UIBatchProcessInfo& info,
			BatchDrawList& display_list,
			uint32& numVertices,
			uint16& numIndices,
			float32 depth = 0.0f,
			const ColorB& tint = ColorB::White
			);

		virtual void batchChildren(
			BatchDrawList& display_list,
			uint32& numVertices,
			uint16& numIndices,
			const ColorB& tint,
			const UIBatchProcessData& data,
			UIBatchProcessInfo& info);

		UIElement* parent;
		std::string name;
		bool visible;
		bool enabled;
		bool shouldClose;
		UIElementFlags flags;
		LuaCallbackPtr flagCallback[UIElementFlagMAX];
		float32 depthBias;
		float32 lastDepth;

		bool hintWidth;
		bool hintHeight;

		TextureHandlePtr texture[BATCH_TEXTURE_STAGES];
		ShaderHandlePtr shader;
		UISpan span;
		RectF screen_rect;
		UIElementQuadType quadType;

		const static int32 kNumVertexColors = 4;

		ColorB vertexColor[kNumVertexColors];
		ColorB elementColor;

		typedef std::vector<UIElementPtr> UIElementVector;
		UIElementVector children;
		UIElementVector childrenToRemove;

		typedef std::vector<UIBehaviorPtr> UIBehaviorVector;
		UIBehaviorVector behaviors;

		typedef std::vector<UIDecoratorPtr> UIDecoratorVector;
		UIDecoratorVector decorators;
		bool decoratorsVisible;

		uint32 sortValue;
		
		BatchDrawDataPtr drawData;
		DrawCall::PreDraw preCallback;
		DrawCall::PostDraw postCallback;

		bool drawDirty;

		float angle;

		UIBatchPassMask passMask;
		std::shared_ptr<UIAnimationPlayer> animations;
	};
}
