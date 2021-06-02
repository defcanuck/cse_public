#include "PCH.h"

#include "scripting/LuaBindAnimation.h"
#include "scripting/LuaMacro.h"
#include "scripting/LuaConverter.h"

namespace cs
{
	using namespace luabind;

	BEGIN_DEFINE_LUA_ENUM(AnimationType)
		.enum_("constants")
		[
			value("None", AnimationTypeNone),
			value("Loop", AnimationTypeLoop),
			value("Bounce", AnimationTypeBounce)
		]
	END_DEFINE_LUA_ENUM()
		
	BEGIN_DEFINE_LUA_CLASS(AnimationBase)
		.def("process", &AnimationBase::process)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_DERIVED(FloatAnimation, AnimationBase)
		.def("getValue", &FloatAnimation::getValue)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_DERIVED(Vec2Animation, AnimationBase)
		.def("getValue", &Vec2Animation::getValue)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_DERIVED(Vec3Animation, AnimationBase)
		.def("getValue", &Vec3Animation::getValue)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_DERIVED(ColorBAnimation, AnimationBase)
		.def("getValue", &ColorBAnimation::getValue)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_DERIVED(ColorFAnimation, AnimationBase)
		.def("getValue", &ColorFAnimation::getValue)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS(FloatLerpAnimator)
	.scope
	[
		def("createAnimation", &FloatLerpAnimator::createAnimation)
	]
	END_DEFINE_LUA_CLASS()
	
	BEGIN_DEFINE_LUA_CLASS(Vec2LerpAnimator)
	.scope
	[
		def("createAnimation", &Vec2LerpAnimator::createAnimation)
	]
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS(Vec3LerpAnimator)
	.scope
	[
		def("createAnimation", &Vec3LerpAnimator::createAnimation)
	]
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS(ColorBLerpAnimator)
	.scope
	[
		def("createAnimation", &ColorBLerpAnimator::createAnimation)
	]
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS(ColorFLerpAnimator)
	.scope
	[
		def("createAnimation", &ColorFLerpAnimator::createAnimation)
	]
	END_DEFINE_LUA_CLASS()


}