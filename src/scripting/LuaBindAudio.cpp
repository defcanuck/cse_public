#include "PCH.h"

#include "scripting/LuaBindAudio.h"

namespace cs
{
	using namespace luabind;

	BEGIN_DEFINE_LUA_CLASS_SHARED(SoundManager)
		.def("playSound", &SoundManager::playSound)
		.def("playMusic", &SoundManager::playMusic)
		.def("stopMusic", &SoundManager::stopMusic)
		.def("setMusicVolume", &SoundManager::setMusicVolume)
		.def("setSoundVolume", &SoundManager::setSoundVolume)
		.def("getMusicVolume", &SoundManager::getMusicVolume)
		.def("getSoundVolume", &SoundManager::getSoundVolume)
		.def("fadeOutMusic", &SoundManager::fadeOutMusic)
		.scope
		[
			def("getInstance", &SoundManager::getInstance)
		]
	END_DEFINE_LUA_CLASS()
}