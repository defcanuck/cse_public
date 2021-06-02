#include "scripting/LuaMacro.h"

#include "scene/Scene.h"
#include "scene/Actor.h"
#include "scene/Camera.h"
#include "scene/Light.h"
#include "scene/SceneCreator.h"
#include "scene/SceneReference.h"

#include "scene/behaviors/Behavior.h"
#include "scene/behaviors/CameraMoveBehavior.h"
#include "scene/behaviors/SelectionBehavior.h"
#include "scene/behaviors/ScriptInputBehavior.h"
#include "scene/behaviors/GameInputBehavior.h"

namespace cs
{
    PROTO_LUA_CLASS(HighPrecisionTimer);
	PROTO_LUA_CLASS(Behavior);
	PROTO_LUA_CLASS(CameraMoveBehavior);
	PROTO_LUA_CLASS(ScriptInputBehavior);
	PROTO_LUA_CLASS(GameInputBehavior);
	
	PROTO_LUA_CLASS(SceneTimer);
	PROTO_LUA_CLASS(Scene);
	PROTO_LUA_CLASS(SceneData);
	PROTO_LUA_CLASS(SceneCreator);
	PROTO_LUA_CLASS(SceneCreateComponent);
	PROTO_LUA_CLASS(SceneCreateComponentMask);
	PROTO_LUA_CLASS(SceneRender);
	PROTO_LUA_CLASS(SceneRenderMask);
	PROTO_LUA_CLASS(SceneParams);
	PROTO_LUA_CLASS(SceneReferenceCache);

	PROTO_LUA_CLASS(Actor);
	PROTO_LUA_CLASS(Camera);
	PROTO_LUA_CLASS(ReferenceNode);
	PROTO_LUA_CLASS(Light);
}
