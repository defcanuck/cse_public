#include "ecs/comp/AudioComponent.h"

namespace cs
{
	BEGIN_META_CLASS(AudioComponent)
		ADD_MEMBER_PTR(effect);
		ADD_MEMBER_PTR(adjustor);
			ADD_COMBO_META_LABEL(AudioVolumeAdjuster, "None");
			ADD_COMBO_META_LABEL(AudioVolumeDistanceAdjustor, "Distance");
		ADD_MEMBER(loop);
			SET_MEMBER_CALLBACK_POST(&AudioComponent::resetSound);
		ADD_MEMBER(maxVolume);
			SET_MEMBER_MIN(0.0f);
			SET_MEMBER_MAX(1.0f);
			SET_MEMBER_DEFAULT(1.0f);
			SET_MEMBER_CALLBACK_POST(&AudioComponent::onVolumeChanged);
	END_META()

	BEGIN_META_CLASS(AudioVolumeAdjuster)
	END_META()

	BEGIN_META_CLASS(AudioVolumeDistanceAdjustor)
		ADD_MEMBER(minDistance);
			SET_MEMBER_MIN(0.0f);
			SET_MEMBER_MAX(2000.0f);
		ADD_MEMBER(maxDistance);
			SET_MEMBER_MIN(0.0f);
			SET_MEMBER_MAX(2000.0f);
		ADD_MEMBER(flipY);
			SET_MEMBER_DEFAULT(true);
	END_META()

	AudioVolumeDistanceAdjustor::AudioVolumeDistanceAdjustor()
		: minDistance(10.0f)
		, maxDistance(100.0f)
		, flipY(true)
	{

	}

	float32 AudioVolumeDistanceAdjustor::getVolume(SceneNode* node, const AudioComponentUpdateParams& params)
	{
		assert(this->maxDistance > this->minDistance);
		
		vec3 cam_pos = params.camera->getCenter();
		if (this->flipY)
		{
			cam_pos.y *= -1.0f;
		}
		vec3 obj_pos = node->getWorldPosition();

		float32 dist = glm::distance(cam_pos, obj_pos);
		return 1.0f - std::min(1.0f, std::max(0.0f, dist - this->minDistance) / (this->maxDistance - this->minDistance));
	}

	AudioComponent::AudioComponent()
		: effect(CREATE_CLASS(SoundEffectHandle))
		, loop(false)
		, maxVolume(1.0f)
		, currentVolume(1.0f)
	{

	}

	AudioComponent::~AudioComponent()
	{
		this->stopInternal();
	}

	void AudioComponent::reset(bool active)
	{

	}

	void AudioComponent::onPostLoad(const LoadFlagMask& flags)
	{

	}

	void AudioComponent::playInternal()
	{
		if (this->isPlaying() || this->currentVolume <= 0.0f)
			return;

		assert(this->instance.channel == -1);
		SoundEffectPtr& e = this->effect->getEffect();

		if (e.get() && this->currentVolume > 0.0f)
		{
			this->instance.channel = e->play((this->loop) ? -1 : 0);
			this->instance.effect = e;
			this->updateVolume();
		}
	}

	void AudioComponent::stopInternal()
	{
		if (!this->isPlaying())
			return;

		this->instance.stop();
		this->instance.effect = nullptr;
	}

	void AudioComponent::resetSound()
	{
		this->stopInternal();
		this->playInternal();
	}

	void AudioComponent::onVolumeChanged()
	{
		if (!this->isPlaying())
			return;

		this->currentVolume = this->maxVolume;
		this->updateVolume();
	}

	void AudioComponent::updateVolume()
	{
        float32 curVolume = SoundManager::getInstance()->getSoundVolume();
		this->instance.setVolume(this->currentVolume * curVolume);
	}

	void AudioComponent::forceStop()
	{
		this->stopInternal();
	}

	void AudioComponent::process(const AudioComponentUpdateParams& params)
	{
		if (params.active)
		{
			if (this->adjustor)
			{
				SceneNode* node = reinterpret_cast<SceneNode*>(this->getParent());
				this->currentVolume = this->maxVolume * this->adjustor->getVolume(node, params);
				this->updateVolume();
			}

			if (!this->isPlaying())
			{
				this->playInternal();
			}
		}
		else
		{
			if (this->isPlaying() || this->currentVolume <= 0.0f)
			{
				this->stopInternal();
			}
		}
	}
}
