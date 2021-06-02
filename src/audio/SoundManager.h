#pragma once

#include <SDL_mixer.h>

#include "ClassDef.h"
#include "global/Resource.h"
#include "global/Singleton.h"
#include "global/Event.h"
#include "global/SerializableHandle.h"

namespace cs
{

	CLASS_DEFINITION_DERIVED_REFLECT(SoundEffect, Resource)
	public:
		SoundEffect() 
			: Resource("Error")
			, chunk(nullptr)
			, volume(1.0f)
		{ }
		~SoundEffect();
		SoundEffect(const std::string& fileName, const std::string& path);

		int32 play(int32 loops = 0);
		
		void setVolume(float32 vol);

	private:

		void load(const std::string& name, const std::string& path);

		float32 volume;
		Mix_Chunk* chunk;
	};

	struct SoundEffectInstance
	{
		SoundEffectInstance()
			: channel(-1)
		{ }
		SoundEffectInstance(int32 c)
			: channel(c)
		{ }

		void stop();
		void setVolume(float32 vol);

		SoundEffectPtr effect;
		int32 channel;
	};

	CLASS_DEFINITION_REFLECT(SoundEffectHandle)
	public:
		SoundEffectHandle()
			: effect(nullptr) { }
		SoundEffectHandle(SoundEffectPtr& ptr)
			: effect(ptr) { }
		SoundEffectHandle(const std::string& fileName);

		Event onChanged;
		void onEffectChanged();

		SoundEffectPtr& getEffect() { return this->effect; }

		void clear()
		{
			this->onChanged.invoke();
		}

	private:

		SoundEffectPtr effect;

	};

	CLASS_DEFINITION_DERIVED_REFLECT(Music, Resource)
	public:
		Music()
			: Resource("Error")
			, music(nullptr)
		{ }
		~Music();
		Music(const std::string& fileName, const std::string& path);

		void play(int32 loops = -1);
		void stop();

	private:

		void load(const std::string& name, const std::string& path);


		Mix_Music* music;
	};

	class SoundManager : public Singleton<SoundManager>
	{
	public:

		SoundManager()
			: soundVolume(1.0)
			, musicVolume(0.5f)
		{ }

		void init();

		void playSound(const std::string& name);
		void playMusic(const std::string& name);
		void stopMusic();
		void fadeOutMusic(int seconds);

		SoundEffectPtr getSound(const std::string& name);
		
		void setMusicVolume(float pct);
		void setSoundVolume(float pct);
		
		float getMusicVolume() const { return this->musicVolume; }
		float getSoundVolume() const { return this->soundVolume; }

	private:

		typedef std::map<std::string, SoundEffectPtr> SoundEffectMap;

		SoundEffectMap effects;
		MusicPtr music;

		float soundVolume;
		float musicVolume;
	};

	template <>
    std::string SerializableHandle<SoundEffect>::getExtension();

	template <>
    std::string SerializableHandle<SoundEffect>::getDescription();

	template <>
    std::string SerializableHandle<Music>::getExtension();

	template <>
    std::string SerializableHandle<Music>::getDescription();
}
