#include "PCH.h"

#include "scripting/LuaBindOS.h"
#include "scripting/LuaMacro.h"
#include "scripting/LuaConverter.h"

#include "os/FileManager.h"

#if defined(CS_WINDOWS)
    #include "os/windows/FileManager_Windows.h"
#endif



#include <string>

#include <luabind/iterator_policy.hpp>

namespace cs
{
    
	using namespace luabind;
    
    BEGIN_DEFINE_LUA_CLASS(Leaderboard)
        .scope
        [
			def("getRequestId", &Leaderboard::getRequestId),
			def("requestScores", &Leaderboard::requestScores),
			def("addScore", (bool(*)(const std::string&, int)) &Leaderboard::addScore),
			def("getScores", &Leaderboard::getScores)
        ]
    END_DEFINE_LUA_CLASS()
    
    BEGIN_DEFINE_LUA_CLASS(Analytics)
        .scope
        [
            def("startLevel", &Analytics::startLevel),
            def("finishLevel", (bool(*)(const std::string&, const std::string&, int)) &Analytics::finishLevel),
            def("modifyCurrency", &Analytics::modifyCurrency)
        ]
    END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_ENUM(FileDataType)
		.enum_("constants")
		[
			value("File", FileDataType_File),
			value("Directory", FileDataType_Directory)
		]
	END_DEFINE_LUA_ENUM()

	BEGIN_DEFINE_LUA_CLASS_SHARED(FileSystemNode)
		.def("getData", &FileSystemNode::getData<FileSystemBase>)
		.def("getDataFile", &FileSystemNode::getData<FileSystemEntry>)
		.def("getDataDirectory", &FileSystemNode::getData<FileSystemDirectory>)
		.def("print", &FileSystemNode::debugPrint)
		.def_readwrite("type", &FileSystemNode::type)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(FileSystemBase)
		.def_readwrite("name", &FileSystemBase::name)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_DERIVED(FileSystemEntry, FileSystemBase)
		.def_readwrite("extension", &FileSystemEntry::ext)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_DERIVED(FileSystemDirectory, FileSystemBase)
		.def_readwrite("nodes", &FileSystemDirectory::nodes, return_stl_iterator)
		.def("num", &FileSystemDirectory::getNumNodes)
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(FileManager)
		.def("getFilesByExtension", &FileManager::getFilesByExtension)
		.def("getNodesByExtension", &FileManager::getNodesByExtension)
		.def("printFileSystem", &FileManager::printFileSystem)
		.def("setSavePath", &FileManager::setSavePath)
		.def("getSavePath", &FileManager::getSavePath)
		.scope
		[
			def("getInstance", &FileManager::getInstance)
		]
		
	END_DEFINE_LUA_CLASS()

	BEGIN_DEFINE_LUA_CLASS_SHARED(LogManager)
		.def("setLogToConsole", &LogManager::setLogToConsole)
		.scope
		[
			def("getInstance", &LogManager::getInstance)
		]

	END_DEFINE_LUA_CLASS()

#if defined(CS_WINDOWS)
	BEGIN_DEFINE_LUA_CLASS_DERIVED(FileManager_Windows, FileManager)
	END_DEFINE_LUA_CLASS()
#endif
    
}
