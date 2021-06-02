#include "PCH.h"

#include "ecs/ECS_Context.h"
#include "ecs/system/BaseSystem.h"

namespace cs
{
    const ECSSystemMask ECSContext::kDefaultSystems((0x1 << size_t(ECSMAX)) - 1);
    
    const char* kSystemsStr[] =
    {
        "Drawable",
        "Physics",
        "Script",
        "Animation",
        "Particle",
        "Game",
		"Audio"
    };
    
	ECSContext::ECSContext(const std::string& n, const ECSSystemMask& m)
        : name(n)
        , systemsEnabled(m)
	{
		assert(this->name.length() > 0);
		for (uint32 i = 0; i < ECSMAX; i++)
			systems[i] = nullptr;

		log::info("Creating ", this->name, " context");
	}

	ECSContext::~ECSContext()
	{
		assert(ECSMAX == sizeof(kSystemsStr) / sizeof(kSystemsStr[0]));
        log::info("Destroying ", this->name, " context");
        
		for (uint32 i = 0; i < ECSMAX; i++)
        {
            if (systems[i])
            {
                log::info("Destroying ", kSystemsStr[i], " System");
                delete systems[i];
            }
        }
	}

	void ECSContext::resetSystems()
	{
		for (auto& it : this->systems)
		{
			if (it)
			{
				it->reset();
			}
		}
	}

	ECSContextPtr ECSContextManager::createContext(const std::string& n, const ECSSystemMask& m)
	{
		ECSContextPtr cxt = CREATE_CLASS(ECSContext, n, m);
		this->contexts[(uintptr_t) cxt.get()] = cxt;
		return cxt;
	}

	void ECSContextManager::removeContext(ECSContext* cxt)
	{
		ECSContextMap::iterator it = this->contexts.find((uintptr_t) cxt);
		if (it != this->contexts.end())
		{
			this->contexts.erase(it);
		}
	}
}
