#include "PCH.h"

#include "scripting/LuaBindFX.h"

namespace cs
{
	using namespace luabind;

	BEGIN_DEFINE_LUA_CLASS_SHARED(ParticleEmitter)
		.def("setEmitting", &ParticleEmitter::setEmitting)
		.def("getEmitting", &ParticleEmitter::getEmitting)
		.def("overrideVelocity", &ParticleEmitter::overrideVelocity)
		.scope
		[
			def("burst", &ParticleEmitter::burst),
			def("burstTint", &ParticleEmitter::burstTint),
			def("getStats", &ParticleEmitter::getStats)
		]
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(ParticleEmitterAnchor, SceneNode)
		.def(constructor<>())
		.def(constructor<const std::string&, RenderTraversal>())
		.def("process", &ParticleEmitterAnchor::process)
		.def("getEmitter", &ParticleEmitterAnchor::getEmitter)
	END_DEFINE_LUA_CLASS()
	
	BEGIN_DEFINE_LUA_CLASS(ParticleEmitterScope)
		.def("addEmitter", &ParticleEmitterScope::addEmitter)
	END_DEFINE_LUA_CLASS()
}