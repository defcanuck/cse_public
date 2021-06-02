#include "PCH.h"

#include "scripting/LuaBindUI.h"
#include "scripting/LuaState.h"
#include <string>

#include <luabind/adopt_policy.hpp>

namespace cs
{
	struct LuaUIDocument : UIDocument, luabind::wrap_base
	{
		LuaUIDocument(const std::string& name) : UIDocument(name) { }
		
		virtual void luaPopulate()
		{
			CALL_LUA_FUNCTION_SINGLE("populate");
		}

		static void default_luaPopulate(UIDocument* ptr)
		{
			return ptr->UIDocument::luaPopulate();
		}

		virtual void luaProcess(float32 dt)
		{
			CALL_LUA_FUNCTION("process", dt);
		}

		static void default_luaProcess(UIDocument* ptr, float32 dt)
		{
			return ptr->UIDocument::luaProcess(dt);
		}

		virtual void luaOnCursor(ClickInput input, TouchState state, vec2 position)
		{
			// CALL_LUA_FUNCTION("onCursor", input, state, position);
		}

		static void default_luaOnCursor(UIDocument* ptr, ClickInput input, TouchState state, vec2 position)
		{
			return ptr->UIDocument::luaOnCursor(input, state, position);
		}

		virtual void luaOnCursorMove(vec2 pos)
		{
			// CALL_LUA_FUNCTION("onCursorMove", pos);
		}

		static void default_luaOnCursorMove(UIDocument* ptr, vec2 position)
		{
			return ptr->UIDocument::luaOnCursorMove(position);
		}
	};

	using namespace luabind;

	BEGIN_DEFINE_LUA_ENUM(UIBatchPass)
		.enum_("constants")
		[
			value("Main", UIBatchPassMain),
			value("Stencil", UIBatchPassStencil)
		]
	END_DEFINE_LUA_ENUM()

	BEGIN_DEFINE_LUA_ENUM(UIElementFlag)
		.enum_("constants")
		[
			value("Animating", UIElementFlagAnimating)
		]
	END_DEFINE_LUA_ENUM()

	BEGIN_DEFINE_LUA_ENUM(SpanMeasure)
		.enum_("constants")
		[
			value("Pixels", SpanPixels),
			value("Percent", SpanPercent),
			value("Points", SpanPoints)
		]
	END_DEFINE_LUA_ENUM()

	BEGIN_DEFINE_LUA_ENUM(HAlign)
		.enum_("constants")
		[
			value("Left", HAlignLeft),
			value("Right", HAlignRight),
			value("Center", VAlignCenter)
		]
	END_DEFINE_LUA_ENUM()

	BEGIN_DEFINE_LUA_ENUM(VAlign)
		.enum_("constants")
		[
			value("Bottom", VAlignBottom),
			value("Top", VAlignTop),
			value("Center", VAlignCenter)
		]
	END_DEFINE_LUA_ENUM()

	BEGIN_DEFINE_LUA_CLASS_SHARED(UIClickBehavior)
		.def(constructor<>())
		.def("setClickSound", &UIClickBehavior::setClickSound)
		.def_readwrite("onClick", &UIClickBehavior::onClick)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(UIClickAnimationBehavior, UIClickBehavior)
		.def(constructor<>())
		.def_readwrite("onBeginAnimation", &UIClickAnimationBehavior::onBeginAnimation)
		.def_readwrite("onEndAnimation", &UIClickAnimationBehavior::onEndAnimation)
		.def("addAnimation", &UIClickAnimationBehavior::addAnimation<float32>)
		.def("addAnimation", &UIClickAnimationBehavior::addAnimation<vec2>)
		.def("addAnimation", &UIClickAnimationBehavior::addAnimation<vec3>)
		.def("addAnimation", &UIClickAnimationBehavior::addAnimation<ColorB>)
		.def("addAnimation", &UIClickAnimationBehavior::addAnimation<ColorF>)
		.def("addUniformAnimation", &UIClickAnimationBehavior::addUniformAnimation<float32>)
		.def("addUniformAnimation", &UIClickAnimationBehavior::addUniformAnimation<vec2>)
		.def("addUniformAnimation", &UIClickAnimationBehavior::addUniformAnimation<vec3>)
		.def("addUniformAnimation", &UIClickAnimationBehavior::addUniformAnimation<ColorB>)
		.def("addUniformAnimation", &UIClickAnimationBehavior::addUniformAnimation<ColorF>)
		.def("setClickSound", &UIClickAnimationBehavior::setClickSound)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(UIHoverBehavior)
		.def(constructor<>())
		.def("setHoverColor", &UIHoverBehavior::setHoverColor)
		.def("setOperation", &UIHoverBehavior::setOperation)
	END_DEFINE_LUA_CLASS()

	DEFINE_DERIVED_VALUES_2(UIElement, UITextElement, UIFlipView)
	BEGIN_DEFINE_LUA_CLASS_SHARED(UIElement)
		SET_DERIVED_VALUES_2(UIElement, UITextElement, UIFlipView)
		.def(constructor<const std::string&>())
		.def("getName", &UIElement::getName)
		.def("setTexture", (void(UIElement::*)(const std::string&)) &UIElement::setTexture)
		.def("setTexture", (void(UIElement::*)(TextureHandlePtr&)) &UIElement::setTexture)
		.def("setTextureStage", &UIElement::setTextureStage)
		.def("getTexture", &UIElement::getTexture)
		.def("clearTexture", &UIElement::clearTexture)
		.def("fitTexture", &UIElement::fitTexture)
		.def("setAndFitTexture", &UIElement::setAndFitTexture)
		.def("setAndFitTextureScaled", &UIElement::setAndFitTextureScaled)
		.def("setWidth", (void(UIElement::*)(float32, SpanMeasure)) &UIElement::setWidth<float32>)
		.def("setHeight", (void(UIElement::*)(float32, SpanMeasure)) &UIElement::setHeight<float32>)
		.def("aspectMatchWidth", &UIElement::aspectMatchWidth)
		.def("aspectMatchHeight", &UIElement::aspectMatchHeight)
		.def("setShader", (void(UIElement::*)(const std::string&)) &UIElement::setShader)
		.def("setShader", (void(UIElement::*)(const ShaderHandlePtr&)) &UIElement::setShader)
		.def("setHintWidth", &UIElement::setHintWidth)
		.def("setHintHeight", &UIElement::setHintHeight)
		.def("setFlagCallback", &UIElement::setFlagCallback)
		.def("addChild", (void(UIElement::*)(std::shared_ptr<UIElement>&)) &UIElement::addChild<UIElement>)
		.def("addChild", (void(UIElement::*)(std::shared_ptr<UITextElement>&)) &UIElement::addChild<UITextElement>)
		.def("addChild", (void(UIElement::*)(std::shared_ptr<UISlider>&)) &UIElement::addChild<UISlider>)
		.def("addChild", (void(UIElement::*)(std::shared_ptr<UIFlipView>&)) &UIElement::addChild<UIFlipView>)

		.def("removeChild", (void(UIElement::*)(std::shared_ptr<UIElement>&)) &UIElement::removeChild<UIElement>)
		.def("removeChild", (void(UIElement::*)(std::shared_ptr<UITextElement>&)) &UIElement::removeChild<UITextElement>)
		.def("removeChild", (void(UIElement::*)(std::shared_ptr<UISlider>&)) &UIElement::removeChild<UISlider>)
		.def("removeChild", (void(UIElement::*)(std::shared_ptr<UIFlipView>&)) &UIElement::removeChild<UIFlipView>)
		.def("childExists", &UIElement::childExists)
		.def("removeChildByName", &UIElement::removeChildByName)

        .def("getChildByIndex", &UIElement::getChildByIndex)
		.def("getChildByName", &UIElement::getChildByName)

		.def("removeAllChildren", &UIElement::removeAllChildren)
		.def("setEnabled", &UIElement::setEnabled)
		.def("setEnabledRecursive", &UIElement::setEnabledRecursive)
		.def("setVisible", &UIElement::setVisible)
		.def("setVisibleRecursive", &UIElement::setVisibleRecursive)

		.def("setTextureWrapHorizontal", &UIElement::setTextureWrapHorizontal)
		.def("setTextureWrapVertical", &UIElement::setTextureWrapVertical)

		.def("setTextureFlipHorizontal", &UIElement::setTextureFlipHorizontal)
		.def("setTextureFlipVertical", &UIElement::setTextureFlipVertical)
		
		.def("setPosition", (void(UIElement::*)(const vec2&, HAlign, VAlign, SpanMeasure, SpanMeasure)) &UIElement::setPosition)
		.def("setPosition", (void(UIElement::*)(const PointF&, HAlign, VAlign, SpanMeasure, SpanMeasure)) &UIElement::setPosition)
		.def("setPosition", (void(UIElement::*)(float32, float32, HAlign, VAlign, SpanMeasure, SpanMeasure)) &UIElement::setPosition)
		.def("setXPosition", (void(UIElement::*)(float32, HAlign, SpanMeasure)) &UIElement::setXPosition)
		.def("setYPosition", (void(UIElement::*)(float32, VAlign, SpanMeasure)) &UIElement::setYPosition)
		.def("setDepthBias", &UIElement::setDepthBias)
		.def("setRawWidth", &UIElement::setRawWidth)
		.def("setRawHeight", &UIElement::setRawHeight)
		.def("setRawXPosition", &UIElement::setRawXPosition)
		.def("setRawYPosition", &UIElement::setRawYPosition)

		.def("getRawWidth", &UIElement::getRawWidth)
		.def("getRawHeight", &UIElement::getRawHeight)
		.def("getRawXPosition", &UIElement::getRawXPosition)
		.def("getRawYPosition", &UIElement::getRawYPosition)
		.def("setVertexColorAtIndex", &UIElement::setVertexColorAtIndex)
		.def("setVertexColor", &UIElement::setVertexColor)
		.def("getVertexColor", &UIElement::getVertexColor)
		.def("setElementColor", &UIElement::setElementColor)
		.def("setVertexAlpha", &UIElement::setVertexAlpha)
		.def("setElementAlpha", &UIElement::setElementAlpha)
		.def("addBehavior", &UIElement::addBehavior<std::shared_ptr<UIClickBehavior>>)
		.def("addBehavior", &UIElement::addBehavior<std::shared_ptr<UIHoverBehavior>>)
		.def("clearBehaviors", &UIElement::clearBehaviors)
		.def("setDecoratorsVisible", &UIElement::setDecoratorsVisible)
		.def("addBehavior", &UIElement::addBehavior<std::shared_ptr<UIClickAnimationBehavior>>)
		.def("addDecorator", &UIElement::addDecorator<std::shared_ptr<UIOutlineDecorator>>)
		.def("addDecorator", &UIElement::addDecorator<std::shared_ptr<UIComponentDecorator>>)
		.def("clearDecorators", &UIElement::clearDecorators)
		.def("getShader", &UIElement::getShader)
		.def("getLastDepth", &UIElement::getLastDepth)
		.def("flipTextureHorizontal", &UIElement::flipTextureHorizontal)
		.def("flipTextureVertical", &UIElement::flipTextureVertical)
		.def("printChildren", &UIElement::printChildren)
		.def("setAngleDegrees", &UIElement::setAngleDegrees)
		.def("setAngleRadians", &UIElement::setAngleRadians)
		.def("flushRemoved", &UIElement::flushRemoved)
		.def("setRenderOnPass", &UIElement::setRenderOnPass)
		.def("isPassSet", &UIElement::isPassSet)
		.def("getAnimationPlayer", &UIElement::getAnimationPlayer)
		.scope
		[
			def("getDepthIncrement", &UIElement::getDepthIncrement)
		]
	END_DEFINE_LUA_CLASS()
				
	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(UISlider, UIElement)
		.def(constructor<const std::string&>())

		.def_readwrite("onChanged", &UISlider::onChanged)
		.def("getTickElement", &UISlider::getTickElement)
		.def("setTickWidth", &UISlider::setTickWidth)
		.def("setTickDrag", &UISlider::setTickDrag)
		.def("setTickPercent", &UISlider::setTickPercent)
		.def("getTickPercent", &UISlider::getTickPercent)
		.def("getRawPercent", &UISlider::getRawPercent)
		.def("setFullscreenTouch", &UISlider::setFullscreenTouch)
	END_DEFINE_LUA_CLASS()

	DEFINE_INHERITED_VALUES(UIFlipView, UIElement)
	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(UIFlipView, UIElement)
		SET_INHERITED_VALUES(UIFlipView, UIElement)
		.def(constructor<const std::string&>())
		.def("populate", &UIFlipView::populate)
		.def("setPopulateCallback", &UIFlipView::setPopulateCallback)
		.def("setAnimateCallback", &UIFlipView::setAnimateCallback)
        .def("setUpdateCallback", &UIFlipView::setUpdateCallback)
		.def("setAnimationTime", &UIFlipView::setAnimationTime)
		.def("getIndex", &UIFlipView::getIndex)
		.def("getSwipeLeft", &UIFlipView::getSwipeLeft)
		.def("getSwipeRight", &UIFlipView::getSwipeRight)
		.def("addSwipeElement", &UIFlipView::addSwipeElement)
		.def("getSwipeElementAtIndex", &UIFlipView::getSwipeElementAtIndex)
		.def("getCurrentSwipeElement", &UIFlipView::getCurrentSwipeElement)
		.def("setSwipeEnabled", &UIFlipView::setSwipeEnabled)
		.def("getSwipeEnabled", &UIFlipView::getSwipeEnabled)
		.def("setSwipeSound", &UIFlipView::setSwipeSound)
        .def("getNumSwipeElements", &UIFlipView::getNumSwipeElements)
		.def("doSwipeLeft", &UIFlipView::doSwipeLeft)
		.def("doSwipeRight", &UIFlipView::doSwipeRight)
	END_DEFINE_LUA_CLASS()

	DEFINE_INHERITED_VALUES(UITextElement, UIElement)
	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(UITextElement, UIElement)
		SET_INHERITED_VALUES(UITextElement, UIElement)
		.def(constructor<const std::string&>())
		.def(constructor<const std::string&, const std::string&>())
		.def("setText", &UITextElement::setText)
		.def("setTextScale", &UITextElement::setTextScale)
		.def("setTextAngle", &UITextElement::setTextAngle)
		.def("setTextHAlign", &UITextElement::setTextHAlign)
		.def("setTextVAlign", &UITextElement::setTextVAlign)
		.def("setTextHPadding", &UITextElement::setTextHPadding)
		.def("setTextVPadding", &UITextElement::setTextVPadding)
		.def("setTextWrap", &UITextElement::setTextWrap)
		.def("setFontColor", &UITextElement::setFontColor)
		.def("setFontAlpha", &UITextElement::setFontAlpha)
		.def("setFontSize", &UITextElement::setFontSize)
		.def("setFont", (void(UITextElement::*)(const std::string&)) &UITextElement::setFont)
		.def("setFont", (void(UITextElement::*)(const FontPtr&)) &UITextElement::setFont)
		.def("setFontShader", &UITextElement::setFontShader)
		.def("getFontShader", &UITextElement::getFontShader)
		.def("addTextShadow", &UITextElement::addTextShadow)
		.def("clearTextShadows", &UITextElement::clearTextShadows)
		.def("setFontOutline", &UITextElement::setFontOutline)
		.def("setFontOutlineColor", &UITextElement::setFontOutlineColor)
		.def("setShadowAnimation", &UITextElement::setShadowAnimation)
		.def("setForceTextDirty", &UITextElement::setForceTextDirty)
        .def("setFontShadowShader", &UITextElement::setFontShadowShader)
		.def("setTextRenderOnPass", &UITextElement::setTextRenderOnPass)
		.def("isTextPassSet", &UITextElement::isTextPassSet)
	END_DEFINE_LUA_CLASS()

				
	BEGIN_DEFINE_LUA_CLASS_DERIVED_LUA(UIDocument, LuaUIDocument)
		.def(constructor<const std::string&>(), adopt(result))
		.def("getRoot", &UIDocument::getRoot)
		.def("getName", &UIDocument::getName)
		.def("populate", &UIDocument::luaPopulate, &LuaUIDocument::default_luaPopulate)
		.def("onCursor", &UIDocument::luaOnCursor, &LuaUIDocument::default_luaOnCursor)
		.def("onCursorMove", &UIDocument::luaOnCursorMove, &LuaUIDocument::default_luaOnCursorMove)
		.def("process", &UIDocument::luaProcess, &LuaUIDocument::default_luaProcess)
		.def("setDocumentSize", &UIDocument::setDocumentSize)
		.def("setDocumentColor", &UIDocument::setDocumentColor)
		.def("addEmitter", (void(UIDocument::*)(UIElementPtr&, const std::string&, const vec3&, bool)) &UIDocument::addEmitter)
		.def("addEmitter", (void(UIDocument::*)(UIElementPtr&, const std::string&, float32, bool)) &UIDocument::addEmitter)
		.def("removeEmitter", &UIDocument::removeEmitter)
		.def("removeAllEmitters", &UIDocument::removeAllEmitters)
		.def("setLocked", &UIDocument::setLocked)
		.def("getLocked", &UIDocument::getLocked)
		.def("setSortOrder", &UIDocument::setSortOrder)
		.def("setAnimationMode", &UIDocument::setAnimationMode)
		.def("setConsumeClicks", &UIDocument::setConsumeClicks)
		.def_readwrite("animations", &UIDocument::animations)
		.def("preload", &UIDocument::preload)
		.def_readwrite("fx", &UIDocument::fx)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(UIStack)
		.def(constructor<>())
		.def("pushDocument", (void(UIStack::*)(UIDocumentPtr&)) &UIStack::pushDocument)
		.def("pushDocument", (void(UIStack::*)(UIDocumentPtr&, bool)) &UIStack::pushDocument)
		.def("popDocument", &UIStack::popDocument)
		.def("popAllDocuments", &UIStack::popAllDocuments)
		.def("popUnlockedDocuments", &UIStack::popUnlockedDocuments)
		.def("removeDocument", (void(UIStack::*)(const std::string&)) &UIStack::removeDocument)
		.def("removeDocument", (void(UIStack::*)(UIDocumentPtr&)) &UIStack::removeDocument)
		.def("clearActionElement", &UIStack::clearActionElement)
		.def("getTop", &UIStack::getTop)
		.def("getDocumentByName", &UIStack::getDocumentByName)
		.def("updateStack", &UIStack::updateStack)
		.def("setTextRenderOnPass", &UITextElement::setTextRenderOnPass)
		.def("isTextPassSet", &UITextElement::isTextPassSet)
        .def("toggleOverlay", &UIStack::toggleOverlay)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(UIDecorator)
		.def("setDecoratorColor", &UIDecorator::setDecoratorColor)
		.def("setInheritColor", &UIDecorator::setInheritColor)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(UIOutlineDecorator, UIDecorator)
		.def(constructor<const ColorB&, float32>())
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(UIComponentDecorator, UIDecorator)
		.def(constructor<>())
		.def("setTexture", &UIComponentDecorator::setTexture)
		.def("setWidth", &UIComponentDecorator::setWidth)
		.def("setShader", &UIComponentDecorator::setShader)
		.def("getShader", &UIComponentDecorator::getShader)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_ENUM(UIComponentDecoratorType)
	.enum_("constants")
	[
		value("Three", UIComponentDecoratorType3Quadrant),
		value("Eight", UIComponentDecoratorType3Quadrant)
	]
	END_DEFINE_LUA_ENUM()

	BEGIN_DEFINE_LUA_ENUM(UIComponentDecoratorQuadrant)
		.enum_("constants")
		[
			value("Corner", UIComponentDecoratorQuadrantCorner),
			value("CornerTopLeft", UIComponentDecoratorQuadrantCornerTopLeft),
			value("CornerTopRight", UIComponentDecoratorQuadrantCornerTopRight),
			value("CornerBottomLeft", UIComponentDecoratorQuadrantCornerBotLeft),
			value("CornerBottomRight", UIComponentDecoratorQuadrantCornerBotRight),

			value("Side", UIComponentDecoratorQuadrantSide),
			value("SideLeft", UIComponentDecoratorQuadrantSideLeft),
			value("SideRight", UIComponentDecoratorQuadrantSideRight),
			value("SideTop", UIComponentDecoratorQuadrantSideTop),
			value("SideBottom", UIComponentDecoratorQuadrantSideBottom)
		]
	END_DEFINE_LUA_ENUM()

	BEGIN_DEFINE_LUA_ENUM(UIAnimationType)
		.enum_("constants")
		[
			value("Width", UIAnimationTypeWidth),
			value("Height", UIAnimationTypeHeight),
			value("X", UIAnimationTypeX),
			value("Y", UIAnimationTypeY),
			value("Position", UIAnimationTypePosition),
			value("Size", UIAnimationTypeSize),
			value("VertexColor", UIAnimationTypeVertexColor),
			value("VertexAlpha", UIAnimationTypeVertexAlpha),
			value("ElementColor", UIAnimationTypeElementColor),
			value("ElementAlpha", UIAnimationTypeElementAlpha)
		]
	END_DEFINE_LUA_ENUM()

	BEGIN_DEFINE_LUA_ENUM(UITextAnimationType)
		.enum_("constants")
		[
			value("FontColor", UITextElementTypeFontColor),
			value("FontAlpha", UITextElementTypeFontAlpha),
			value("Shadow", UITextElementTypeFontShadow),
			value("TextScale", UITextElementTypeTextScale)
		]
	END_DEFINE_LUA_ENUM()

	BEGIN_DEFINE_LUA_CLASS(UIAnimationCreator)
	.scope
	[
		def("createAnimation", &UIAnimationCreator::createAnimation<float32>),
		def("createAnimation", &UIAnimationCreator::createAnimation<vec2>),
		def("createAnimation", &UIAnimationCreator::createAnimation<vec3>),
		def("createAnimation", &UIAnimationCreator::createAnimation<ColorB>),
		def("createAnimation", &UIAnimationCreator::createAnimation<ColorF>),
		
		def("createTextAnimation", &UIAnimationCreator::createTextAnimation<float32>),
		def("createTextAnimation", &UIAnimationCreator::createTextAnimation<vec2>),
		def("createTextAnimation", &UIAnimationCreator::createTextAnimation<vec3>),
		def("createTextAnimation", &UIAnimationCreator::createTextAnimation<ColorB>),
		def("createTextAnimation", &UIAnimationCreator::createTextAnimation<ColorF>),

		def("createUniformAnimation", &UIAnimationCreator::createUniformAnimation<float32>)
	]
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(UIAnimationPlayer)
		.def(constructor<>())
		.def("addAnimation", &UIAnimationPlayer::addAnimation<float32>)
		.def("addAnimation", &UIAnimationPlayer::addAnimation<vec2>)
		.def("addAnimation", &UIAnimationPlayer::addAnimation<vec3>)
		.def("addAnimation", &UIAnimationPlayer::addAnimation<ColorB>)
		.def("addAnimation", &UIAnimationPlayer::addAnimation<ColorF>)
		.def("addTextAnimation", &UIAnimationPlayer::addTextAnimation<float32>)
		.def("addTextAnimation", &UIAnimationPlayer::addTextAnimation<vec2>)
		.def("addTextAnimation", &UIAnimationPlayer::addTextAnimation<vec3>)
		.def("addTextAnimation", &UIAnimationPlayer::addTextAnimation<ColorB>)
		.def("addTextAnimation", &UIAnimationPlayer::addTextAnimation<ColorF>)
		.def("addUniformAnimation", &UIAnimationPlayer::addUniformAnimation<float32>)
		.def("addAndStartAnimation", &UIAnimationPlayer::addAndStartAnimation<float32>)
		.def("addAndStartAnimation", &UIAnimationPlayer::addAndStartAnimation<vec2>)
		.def("addAndStartAnimation", &UIAnimationPlayer::addAndStartAnimation<vec3>)
		.def("addAndStartAnimation", &UIAnimationPlayer::addAndStartAnimation<ColorB>)
		.def("addAndStartAnimation", &UIAnimationPlayer::addAndStartAnimation<ColorF>)
		.def("addAndStartTextAnimation", &UIAnimationPlayer::addAndStartTextAnimation<float32>)
		.def("addAndStartTextAnimation", &UIAnimationPlayer::addAndStartTextAnimation<vec2>)
		.def("addAndStartTextAnimation", &UIAnimationPlayer::addAndStartTextAnimation<vec3>)
		.def("addAndStartTextAnimation", &UIAnimationPlayer::addAndStartTextAnimation<ColorB>)
		.def("addAndStartTextAnimation", &UIAnimationPlayer::addAndStartTextAnimation<ColorF>)
		.def("start", &UIAnimationPlayer::start)
		.def("startAll", &UIAnimationPlayer::startAll)
		.def("addOnEndCallback", &UIAnimationPlayer::addOnEndCallback)
		.def("clear", &UIAnimationPlayer::clear)
	END_DEFINE_LUA_CLASS()
	
}
