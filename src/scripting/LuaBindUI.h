#pragma once

#include "scripting/LuaMacro.h"

#include "ui/UIAnimationPlayer.h"
#include "ui/UIAnimation.h"
#include "ui/UIElement.h"
#include "ui/UITextElement.h"
#include "ui/UIDocument.h"
#include "ui/UIStack.h"
#include "ui/UIBehavior.h"
#include "ui/UISlider.h"
#include "ui/UIFlipView.h"


#include "ui/decorator/UIOutlineDecorator.h"
#include "ui/decorator/UIComponentDecorator.h"
#include "ui/behavior/UIClickBehavior.h"
#include "ui/behavior/UIHoverBehavior.h"
#include "ui/behavior/UIClickAnimationBehavior.h"

namespace cs
{
	PROTO_LUA_CLASS(SpanMeasure);
	PROTO_LUA_CLASS(HAlign);
	PROTO_LUA_CLASS(VAlign);
	PROTO_LUA_CLASS(UIClickBehavior);
	PROTO_LUA_CLASS(UIHoverBehavior);
	PROTO_LUA_CLASS(UIElement);
	PROTO_LUA_CLASS(UITextElement);
	PROTO_LUA_CLASS(UIDocument);
	PROTO_LUA_CLASS(UIStack);
	PROTO_LUA_CLASS(UISlider);
	PROTO_LUA_CLASS(UIDecorator);
	PROTO_LUA_CLASS(UIOutlineDecorator);
	PROTO_LUA_CLASS(UIComponentDecorator);
	PROTO_LUA_CLASS(UIComponentDecoratorType);
	PROTO_LUA_CLASS(UIComponentDecoratorQuadrant);
	PROTO_LUA_CLASS(UIClickAnimationBehavior);
	PROTO_LUA_CLASS(UIAnimationType);
	PROTO_LUA_CLASS(UITextAnimationType);
	PROTO_LUA_CLASS(UIAnimationCreator);
	PROTO_LUA_CLASS(UIAnimationPlayer);
	PROTO_LUA_CLASS(UIFlipView);
	PROTO_LUA_CLASS(UIElementFlag);
	PROTO_LUA_CLASS(UIBatchPass);
}