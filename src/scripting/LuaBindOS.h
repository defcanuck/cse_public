#pragma once

#include "scripting/LuaMacro.h"
#include "game/GameInterface.h"
#include "scripting/LuaBindings.h"

#include "os/FileManager.h"
#include "os/LogManager.h"
#include "os/Leaderboard.h"
#include "os/Analytics.h"

#include "font/FontManager.h"

#if defined(CS_WINDOWS)
    #include "os/windows/FileManager_Windows.h"
#endif

#include <luabind/luabind.hpp>

namespace cs
{
	PROTO_LUA_CLASS(FileSystemNode);
	PROTO_LUA_CLASS(FileSystemBase);
	PROTO_LUA_CLASS(FileSystemEntry);
	PROTO_LUA_CLASS(FileSystemDirectory);

	PROTO_LUA_CLASS(FileDataType);
	PROTO_LUA_CLASS(FileManager);
    PROTO_LUA_CLASS(Leaderboard);
    PROTO_LUA_CLASS(Analytics);

#if defined(CS_WINDOWS)
	PROTO_LUA_CLASS(FileManager_Windows);
#endif
    
	PROTO_LUA_CLASS(LogManager);
}

