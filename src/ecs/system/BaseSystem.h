#pragma once

#include "global/Singleton.h"
#include "scene/Camera.h"
#include "ecs/comp/Component.h"
#include "ecs/Entity.h"
#include "ecs/ECS_Context.h"
#include "global/Timer.h"
#include "ecs/comp/ComponentList.h"

#include <typeindex>
#include <map>


namespace cs
{

	struct SystemUpdateParams;

	class BaseSystem
	{

	public:
        
        BaseSystem() { }
        virtual~BaseSystem() { }
        
		typedef std::map<uint32, ComponentPtr> ComponentIdMap;
		
	protected:

		typedef std::map<uint32, ComponentIdMap> ComponentCollection;

	public:

		void process(SystemUpdateParams* params)
		{
			ScopedTimer timer(&this->updateTime);
			this->processImpl(params);
		}

		virtual void processImpl(SystemUpdateParams* params) = 0;
		
		template <class T>
		std::shared_ptr<T> getComponent(uint32 id)
		{
			std::type_index index(typeid(T));
			uint32 hash_index = getComponentHash(index);
			if (components.count(hash_index) > 0)
			{
				ComponentIdMap& comp = this->components[hash_index];
				if (comp.count(id) != 0)
					return std::static_pointer_cast<T>(comp[id]);
			}
			return nullptr;
		}

		template <class T>
		size_t getComponentList(SharedList<T>& component_list)
		{
			std::type_index index(typeid(T));
			uint32 hash_index = getComponentHash(index);
			if (this->components.count(hash_index) > 0)
			{
				ComponentIdMap& comp = this->components[hash_index];
				for (auto& it : comp)
				{
					component_list.elements.push_back(std::static_pointer_cast<T>(it.second));
				}
			}
			return component_list.size();
		}

		template <class T>
		ComponentIdMap& getAllComponents()
		{
			std::type_index index(typeid(T));
			uint32 hash_index = getComponentHash(index);
			if (components.count(hash_index) > 0)
				return components[hash_index];
			return emptyMap;
		}

		template <class T>
		size_t getEnabledComponents(ComponentIdMap& enabled_components)
		{
			std::type_index index(typeid(T));
			uint32 hash_index = getComponentHash(index);

			if (this->components.count(hash_index) > 0)
			{
				for (auto& it : this->components[hash_index])
				{
					Entity* parent = it.second->getParent();
					if (parent && parent->getEnabled())
					{
  						enabled_components[it.first] = it.second;
					}
				}
				return enabled_components.size();
			}

			return 0;
		}

		template <class T>
		void subscribeForComponent(ECSContext* context)
		{
			Entity::AddComponentFunction addFunction = std::bind(&BaseSystem::onComponentAdded, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			Entity::RemoveComponentFunction removeFunction = std::bind(&BaseSystem::onComponentRemoved, this, std::placeholders::_1, std::placeholders::_2);
			Entity::ComponentListeners listeners(addFunction, removeFunction);
			Entity::subscribeToComponent<T>(context, listeners);
		}
        
        template <class T>
        void removeComponentSubscription(ECSContext* cxt)
        {
            Entity::removeComponentSubscription<T>(cxt);
        }

		uint32 getUpdateTime() const { return this->updateTime; }
		size_t getNumComponents() const 
		{
			size_t sz = 0;
			for (auto it : this->components) sz += it.second.size();
			return sz;
		}

		virtual void reset() { }
	
	protected:

		static ComponentIdMap emptyMap;

		BaseSystem(ECSContext* cxt)
			: parentContext(cxt)
		{

		}

		void onComponentAdded(uint32 id, uint32 index, ComponentPtr& component)
		{
			if (this->components.count(index) == 0)
			{
				this->components[index] = ComponentIdMap();
			}
			ComponentIdMap& cmap = this->components[index];
			cmap[id] = component;
			this->onComponentSystemAdd(id, component);
		}
		virtual void onComponentSystemAdd(uint32 id, ComponentPtr& component) { }

		void onComponentRemoved(uint32 id, uint32 index)
		{
			if (this->components.count(index) > 0)
			{
				ComponentIdMap& cmap = this->components[index];
				ComponentIdMap::iterator it = cmap.find(id);
				if (it != cmap.end())
				{
					this->onComponentSystemRemove(id, it->second);
					cmap.erase(id);
				}
				else
				{
					log::error("Cannot find component for id: ", id);
				}
			}
			else {
				const MetaData* metadata = MetaManager::getInstance()->get(index);
				log::print(LogError, "Cannot find type ", metadata->getName(), " for key ", id);
			}
		}
		virtual void onComponentSystemRemove(uint32 id, ComponentPtr& component) { }

		ComponentCollection components;
		ECSContext* parentContext;
		uint32 updateTime;
	};
}
