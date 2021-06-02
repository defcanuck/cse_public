#pragma once

#include "ecs/system/BaseSystem.h"

namespace cs
{
	class AudioSystem : public ECSContextSystemBase<AudioSystem, ECSAudio>, public BaseSystem
	{
	public:
		AudioSystem(ECSContext* cxt);
		virtual ~AudioSystem();

		virtual void processImpl(SystemUpdateParams* params);
		void stopAll();

	};
}