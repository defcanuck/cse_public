#include "PCH.h"

#include "ecs/ECS_Utils.h"
#include "ecs/system/AudioSystem.h"
#include "ecs/comp/AudioComponent.h"

namespace cs
{
	AudioSystem::AudioSystem(ECSContext* cxt)
		: BaseSystem(cxt)
	{
		this->subscribeForComponent<AudioComponent>(this->parentContext);
	}

	AudioSystem::~AudioSystem()
	{
		this->removeComponentSubscription<AudioComponent>(this->parentContext);
	}

	void AudioSystem::processImpl(SystemUpdateParams* params)
	{
		BaseSystem::ComponentIdMap scripts;
		this->getEnabledComponents<AudioComponent>(scripts);

		AudioComponentUpdateParams audioParams;
		audioParams.camera = params->camera;
		audioParams.active = params->active;
		audioParams.dt = params->updateDt;

		for (const auto it : scripts)
		{
			const AudioComponentPtr& audioComp =
				std::static_pointer_cast<AudioComponent>(it.second);

			audioComp->process(audioParams);
		}
	}

	void AudioSystem::stopAll()
	{
		BaseSystem::ComponentIdMap scripts;
		this->getEnabledComponents<AudioComponent>(scripts);

		for (const auto it : scripts)
		{
			const AudioComponentPtr& audioComp =
				std::static_pointer_cast<AudioComponent>(it.second);
			audioComp->forceStop();
		}
	}
}