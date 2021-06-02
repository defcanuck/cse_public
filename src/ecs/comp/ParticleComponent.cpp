#include "PCH.h"

#include "ecs/comp/ParticleComponent.h"
#include "gfx/RenderInterface.h"
#include "ecs/Entity.h"

namespace cs
{

	BEGIN_META_CLASS(ParticleComponent)
		ADD_MEMBER_PTR(emitter);
	END_META()

	ParticleComponent::ParticleComponent() 
		: emitter(CREATE_CLASS(ParticleEmitter))
	{
		this->init();
	}

	ParticleComponent::~ParticleComponent()
	{
		if (this->emitter.get())
		{
			this->emitter->clearCallbacks();
		}
	}

	void ParticleComponent::onNew()
	{
		if (this->emitter)
		{
			this->emitter->onNew();
		}
		BASECLASS::onNew();
	}

	void ParticleComponent::process(float32 dt)
	{
		if (this->emitter.get())
		{
			this->emitter->process(dt);
		}
	}

	void ParticleComponent::onPostLoad(const LoadFlagMask& flags)
	{
		this->init();

		if (this->emitter.get())
		{
			this->emitter->onPostLoad(flags);
			this->emitter->onChanged += createCallbackArg0(&ParticleComponent::onEmitterChanged, this);
		
			Entity* parent = this->getParent();
			if (parent)
			{
				this->emitter->setWorldPosCallbackFunc(std::bind(&Entity::getWorldPosition, parent));
				this->emitter->setWorldOrientCallbackFunc(std::bind(&Entity::getWorldRotation, parent));
			}
		}
	}

	void ParticleComponent::init()
	{
	
	}

	void ParticleComponent::onEmitterChanged()
	{

	}

}