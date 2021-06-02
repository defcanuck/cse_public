#pragma once

#include "game/GameInterface.h"
#include "game/Context.h"

#include "scripting/LuaMacro.h"

#include "fx/ParticleEmitter.h"

namespace cs
{
	PROTO_LUA_CLASS(ParticleEmitter);
	PROTO_LUA_CLASS(ParticleEmitterAnchor);
	PROTO_LUA_CLASS(ParticleEmitterScope);
}
