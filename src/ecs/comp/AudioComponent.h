#pragma once

#include "ecs/comp/Component.h"
#include "audio/SoundManager.h"
#include "scene/Camera.h"

namespace cs
{

	struct AudioComponentUpdateParams
	{
		float32 dt;
		bool active;
		CameraPtr camera;
	};

	CLASS_DEFINITION_REFLECT(AudioVolumeAdjuster)
	public:

		virtual float32 getVolume(SceneNode* node, const AudioComponentUpdateParams& params) { return 1.0f; }

	};

	CLASS_DEFINITION_DERIVED_REFLECT(AudioVolumeDistanceAdjustor, AudioVolumeAdjuster)
	public:

		AudioVolumeDistanceAdjustor();
		virtual float32 getVolume(SceneNode* node, const AudioComponentUpdateParams& params);

	private:

		float32 minDistance;
		float32 maxDistance;
		bool flipY;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(AudioComponent, Component)
	public:

		AudioComponent();
		virtual ~AudioComponent();

		bool isPlaying() const { return this->instance.channel >= 0 && this->instance.effect.get(); }
		virtual void reset(bool active = false);
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);

		void process(const AudioComponentUpdateParams& params);
		void forceStop();

	protected:

		void playInternal();
		void stopInternal();
		void resetSound();
		void onVolumeChanged();
		void updateVolume();

		AudioVolumeAdjusterPtr adjustor;
		SoundEffectHandlePtr effect;
		SoundEffectInstance instance;
		bool loop;
		float32 currentVolume;
		float32 maxVolume;
	};
}