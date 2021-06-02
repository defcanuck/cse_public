#include "PCH.h"

#include "audio/SoundManager.h"
#include "os/LogManager.h"
#include "global/ResourceFactory.h"

#include <SDL.h>

namespace cs
{
	BEGIN_META_RESOURCE(SoundEffect)

	END_META()

	BEGIN_META_CLASS(SoundEffectHandle)
		ADD_MEMBER_RESOURCE(effect);
			SET_MEMBER_CALLBACK_POST(&SoundEffectHandle::onEffectChanged);
	END_META()

	BEGIN_META_RESOURCE(Music)

	END_META()

	
	SoundEffect::SoundEffect(const std::string& fileName, const std::string& path)
		: Resource(fileName)
		, chunk(nullptr)
	{
		this->load(fileName, path);
	}

	void SoundEffect::load(const std::string& name, const std::string& path)
	{
		this->chunk = Mix_LoadWAV(path.c_str());
        if (!this->chunk)
		{
            log::error("Failed to load ", name, " with error: ", SDL_GetError());
		}
	}

	SoundEffectHandle::SoundEffectHandle(const std::string& fileName)
	{
		this->effect = ResourceFactory::getInstance()->loadResourceTyped<SoundEffect>(fileName);
	}

	void SoundEffectHandle::onEffectChanged()
	{

	}

	SoundEffect::~SoundEffect()
	{
		if (this->chunk)
		{
			Mix_FreeChunk(this->chunk);
			this->chunk = nullptr;
		}
	}

	void SoundEffect::setVolume(float32 vol)
	{
		if (!this->chunk)
			return;

		int32 int_vol = lerp<int32>(0, MIX_MAX_VOLUME, vol);
		Mix_VolumeChunk(this->chunk, int_vol);
	}

	int32 SoundEffect::play(int32 loop)
	{
		int32 channel = -1;
		if (this->chunk)
		{
			if ((channel = Mix_PlayChannel(-1, this->chunk, loop)) < 0)
			{
				log::error("Failed to play audio: ", SDL_GetError());
			}
			else
			{
				// in case this channel was previously halted, resume it and reset the volume
				int32 int_vol = lerp<int32>(0, MIX_MAX_VOLUME, SoundManager::getInstance()->getSoundVolume());
				Mix_Volume(channel, int_vol);
			}
		}
		return channel;
	}

	void SoundEffectInstance::setVolume(float32 vol)
	{
		if (this->channel < 0)
			return;

		int32 int_vol = lerp<int32>(0, MIX_MAX_VOLUME, vol);
		Mix_Volume(this->channel, int_vol);
		
	}

	void SoundEffectInstance::stop()
	{
		if (this->channel >= 0)
		{
			Mix_HaltChannel(this->channel);
			this->channel = -1;
		}
	}

	Music::Music(const std::string& fileName, const std::string& path)
		: Resource(fileName)
		, music(nullptr)
	{
		this->load(fileName, path);
	}

	void Music::load(const std::string& name, const std::string& path)
	{
		this->music = Mix_LoadMUS(path.c_str());
		if (!this->music)
		{
			log::error("Failed to load ", name, " with error: ", SDL_GetError());
		}
	}

	Music::~Music()
	{
		if (this->music)
		{
			Mix_FreeMusic(this->music);
			this->music = nullptr;
		}
	}

	void Music::play(int loops)
	{
		if (this->music)
		{
			if (Mix_PlayMusic(this->music, loops) < 0)
			{
				log::error("Failed to play music: ", SDL_GetError());
			}
		}
	}

	void Music::stop()
	{
		if (this->music)
		{
			Mix_HaltMusic();
		}
	}

	void SoundManager::init()
	{
		//SDL_AudioInit("waveout");
		//Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_FLAC | MIX_INIT_MOD );

		//Initialize SDL_mixer
		SDL_SetMainReady();
		if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
		{
			log::error("Failed to initialize audio device with error ", SDL_GetError());
			return;
		}
		else
		{
			log::info("Audio Device Initialized Successfully!");
		}
	}

	void SoundManager::playSound(const std::string& name)
	{
		SoundEffectPtr effect = this->getSound(name);
		if (effect.get())
		{
			effect->play();
		}
	}

	void SoundManager::stopMusic()
	{
		if (this->music.get())
		{
			this->music->stop();
			this->music = nullptr;
		}
	}

	void SoundManager::fadeOutMusic(int seconds)
	{
		if (!this->music.get())
			return;

		int ms = seconds * 1000;
		Mix_FadeOutMusic(ms);
	}

	void SoundManager::setMusicVolume(float pct)
	{
		if (this->musicVolume != pct)
		{
			this->musicVolume = pct;
			int amt = static_cast<int>(pct * MIX_MAX_VOLUME);
			Mix_VolumeMusic(amt);  
		}
	}

	void SoundManager::setSoundVolume(float pct)
	{
		if (this->soundVolume != pct)
		{
			this->soundVolume = pct;
			int amt = static_cast<int>(pct * MIX_MAX_VOLUME);
			Mix_Volume(-1, amt);
		}
	}

	void SoundManager::playMusic(const std::string& name)
	{
		if (this->music)
		{
			if (this->music->getName() == name)
				return;
			this->music->stop();
		}

		MusicPtr music_ptr = ResourceFactory::getInstance()->loadResourceTyped<Music>(name);
		if (music_ptr.get())
		{
			this->music = music_ptr;
			this->music->play();
		}
	}


	SoundEffectPtr SoundManager::getSound(const std::string& name)
	{
		SoundEffectMap::iterator it = this->effects.find(name);
		if (it != this->effects.end())
		{
			return it->second;
		}

		SoundEffectPtr effect = ResourceFactory::getInstance()->loadResourceTyped<SoundEffect>(name);
		if (effect.get())
		{
			this->effects[name] = effect;
		}
		return effect;
	}
    
    template <>
    std::string SerializableHandle<SoundEffect>::getExtension()
    {
        return "wav";
    }
    
    template <>
    std::string SerializableHandle<SoundEffect>::getDescription()
    {
        return "CSE SoundEffect";
    }
    
    template <>
    std::string SerializableHandle<Music>::getExtension()
    {
        return "ogg";
    }
    
    template <>
    std::string SerializableHandle<Music>::getDescription()
    {
        return "CSE Music";
    }
}
