#pragma once

#include "scripting/LuaMacro.h"

#include "scripting/LuaCallback.h"
#include "global/Event.h"
#include "os/Input.h"
#include "global/PropertySet.h"
#include "scripting/ScriptNotification.h"
#include "os/InputManager.h"

namespace cs
{
	PROTO_LUA_CLASS(ClickInput);
	PROTO_LUA_CLASS(TouchState);

	PROTO_LUA_CLASS(Event);
	PROTO_LUA_CLASS(LuaCallback);
    PROTO_LUA_CLASS(ScriptNotification);
    
	PROTO_LUA_CLASS(LoadFlags);
	PROTO_LUA_CLASS(LoadFlagMask);

	PROTO_LUA_CLASS(PropertySetHandle);
	PROTO_LUA_CLASS(PropertySetResource);

	PROTO_LUA_CLASS(InputManager);
	
}
