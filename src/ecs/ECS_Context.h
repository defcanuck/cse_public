#pragma once

#include "ClassDef.h"
#include "global/Singleton.h"
#include "global/Values.h"
#include "global/BitMask.h"

#include <map>
#include <string>
#include <stack>

namespace cs
{
	enum ECSSystems
	{
		ECSNone = -1,
		ECSDraw,
		ECSPhysics,
		ECSScript,
		ECSAnimation,
		ECSParticle,
		ECSGame,
		ECSAudio,
		//...
		ECSMAX
	};

	typedef BitMask<ECSSystems, ECSMAX> ECSSystemMask;

	class BaseSystem;

	template <class S, ECSSystems SN>
	class ECSContextSystemBase;

	CLASS_DEFINITION(ECSContext)

	public:
    
        const static ECSSystemMask kDefaultSystems;

		ECSContext(const std::string& name, const ECSSystemMask& mask);
		~ECSContext();

		void setName(const std::string& n) { this->name = n; }
		const std::string& getName() const { return this->name; }

		void resetSystems();

	private:

		template <class S, ECSSystems SN>
		friend class ECSContextSystemBase;

		template<class S, ECSSystems SN>
		BaseSystem* getSystemInternal()
		{
			if (this->systems[SN] == nullptr)
				this->systems[SN] = reinterpret_cast<BaseSystem*>(new S(this));
			return this->systems[SN];
		}
    
        std::string name;
		BaseSystem* systems[ECSMAX];
        ECSSystemMask systemsEnabled;
    };

	class ECSContextManager : public Singleton<ECSContextManager>
	{
	public:

		ECSContextPtr createContext(const std::string& n, const ECSSystemMask& m);
		void removeContext(ECSContext* cxt);

		ECSContextManager()
		{
			this->defaultContext = this->createContext("default", ECSContext::kDefaultSystems);
			this->current = this->defaultContext;
		}

		void setContext(ECSContext* ptr)
		{
			if (!ptr || ptr == current.get())
				return;

			for (auto& it : this->contexts)
			{
				if ((ECSContext*) it.first == ptr)
				{
					this->setContext(it.second);
					break;
				}
			}
		}

		void setContext(ECSContextPtr& ptr)
		{
            ECSContextPtr oldContext = this->current;
			this->current = ptr;
		}
		
		ECSContextPtr& getCurrentContext()
		{
			return this->current;
		}

	private:
		
		typedef std::map<uintptr_t, ECSContextPtr> ECSContextMap;

		ECSContextPtr defaultContext;
		ECSContextPtr current;
		ECSContextMap contexts;
	};

	struct ECSContextScope
	{
		ECSContextScope(ECSContextPtr& ptr)
		{
			this->old = ECSContextManager::getInstance()->getCurrentContext();
			ECSContextManager::getInstance()->setContext(ptr);
		}

		ECSContextScope(ECSContext* ptr)
		{
			this->old = ECSContextManager::getInstance()->getCurrentContext();
			ECSContextManager::getInstance()->setContext(ptr);
		}

		~ECSContextScope()
		{
			ECSContextManager::getInstance()->setContext(this->old);
		}

		ECSContextPtr old;
	};

	template <class S, ECSSystems SN>
	class ECSContextSystemBase
	{
	public:
		static S* getInstance()
		{
			ECSContextManager* mgr = ECSContextManager::getInstance();
			ECSContextPtr& cxt = mgr->getCurrentContext();
			if (!cxt)
			{
				log::print(LogError, "No context bound!");
				GTFO(-1);
			}
            if (cxt->systemsEnabled.test(SN))
            {
                return reinterpret_cast<S*>(cxt->getSystemInternal<S, SN>());
            }
            else
            {
                log::error("Error this system is not enabled for this context!");
				return nullptr;
            }
        }

	protected:

		ECSContextSystemBase() { }
		ECSContextSystemBase(ECSContextSystemBase const &) = delete;
		ECSContextSystemBase& operator = (ECSContextSystemBase const &);

	};

}
