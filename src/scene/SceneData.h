#pragma once

#include "ClassDef.h"

#include "scene/Actor.h"
#include "scene/Camera.h"
#include "scene/Light.h"
#include "ecs/ECS_Context.h"
#include "liquid/LiquidContext.h"


namespace cs
{
	enum SceneCreateComponent
	{
		SceneCreateComponentDrawable,
		SceneCreateComponentPhysics,
		SceneCreateComponentAnimation,
		SceneCreateComponentMAX
	};

	typedef BitMask<SceneCreateComponent, SceneCreateComponentMAX> SceneCreateComponentMask;

	class Entity;
	class Scene;

	struct SceneSelected
	{
		SceneSelected()
			: selected(nullptr)
			, scene(nullptr)
			, position(kZero3)
		{ }

		std::shared_ptr<Entity> selected;
		std::shared_ptr<Scene> scene;
		vec3 position;
	};

	typedef std::shared_ptr<SceneSelected> SceneSelectedPtr;

	CLASS_DEFINITION_REFLECT(SceneData)
	
	public:
    
		typedef std::unordered_map<std::string, EntityPtr> EntityLookupMap;
		typedef std::unordered_map<uint32, LightPtr> LightLookupMap;
		typedef std::vector<LightPtr> LightList;

		SceneData();
		SceneData(const std::string& n);
        SceneData(const std::string& n, const ECSSystemMask& mask);
    
		virtual ~SceneData();

        void init(const ECSSystemMask& mask = ECSContext::kDefaultSystems);
		void empty();
		void remap();
		void destroy();

		const std::string& getName() const { return this->name; }

		void getAllEntities(Entity::EntityList& entities);
		
		Entity::EntityList& getEntityList() { return this->entities.value_vec; }
		Entity::Collection& getEntities() { return this->entities; }

		LightList& getAllLights() { return this->lights; }
		void bindLights();

		virtual void setContext();
		ECSContextPtr& getContext() { return this->ecsCxt; }

		template <class T>
		void onPostCreateCallback(std::shared_ptr<T>& entity)
		{

		}

		template <class T>
		std::shared_ptr<T> create(void(*initParams)(T*))
		{
			static EntityPtr kEmptyEntity;
			return this->create<T>(initParams, kEmptyEntity);
		}

		template <class T, class C, typename... Args>
		std::shared_ptr<T> create(
			void(*initParams)(T*, const SceneCreateComponentMask&),
			const SceneCreateComponentMask& mask,
			std::shared_ptr<C>& parent,
			Args... args)
		{
			MetaData* meta = MetaCreator<T>::get();
			std::string meta_name = meta->getName();
			return create(meta_name, initParams, mask, parent, args...);
		}

        std::string getEmptyKey(const std::string& use_name, Entity::Collection& collection)
        {
            std::string name = use_name;
            uint32 inc = 0;
            while (true)
            {
                EntityPtr& val = collection.getByKey(name);
                if (!val)
                    break;
                
                std::stringstream str;
                str << use_name << inc++;
                name = str.str();
            }
            return name;
        }
    
		template <class T, class C, typename... Args>
		std::shared_ptr<T> create(
			const std::string& entityName,
			void(*initParams)(T*, const SceneCreateComponentMask&), 
			const SceneCreateComponentMask& mask,
			std::shared_ptr<C>& parent, 
			Args... args)
		{
			this->setContext();
			EntityPtr& ptr = this->entities.getByKey(name);

			std::string name;
			if (parent)
				name = this->getEmptyKey(entityName, parent->getChildren());
			else
				name = this->getEmptyKey(entityName, this->entities);
			
			std::shared_ptr<T> new_entity = CREATE_CLASS(T, name, this->ecsCxt.get(), args...);
			if (new_entity)
			{
				bool added = false;
				if (parent)
				{
					std::shared_ptr<Entity> entity = std::static_pointer_cast<Entity>(parent);
					assert(entity.get());
					added = Entity::addChild(entity, new_entity);
				} else {
					added = this->entities.add(new_entity, name);
				}
				
				if (added)
				{
					if (initParams)
					{
						(*initParams)(new_entity.get(), mask);
					}

					this->onPostCreateCallback<T>(new_entity);

					// refresh params
					new_entity->refreshNode();
					return new_entity;
				}
			}

			return std::shared_ptr<T>();
		}

		template <class T>
		bool remove(std::shared_ptr<T>& entity_to_remove)
		{
			const std::string& name = entity_to_remove->getName();
			return this->entities.removeByKey(name);
		}

		bool removeByName(const std::string& entity_name)
		{
			return this->entities.removeByKey(entity_name);
		}

		template <class T>
		std::shared_ptr<T> get(const std::string& name)
		{
			EntityPtr& ptr = this->entities.getByKey(name);
			return std::static_pointer_cast<T>(ptr);
		}

		LightPtr& getLightByIndex(uint32 index);
		
		void prepareSceneContext(LuaStatePtr& state);

		virtual void onPreLoad();
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);
		virtual void reset(bool active = true);

		static bool intersects(Entity* entity, const Ray& ray, EntityIntersection& hit_data);

		void printStatus();

		LuaStatePtr& getLuaState() { return this->luaState; }
	protected:

		std::string name;

		ECSContextPtr ecsCxt;
		LuaStatePtr luaState;
		
		LightList lights;
		LightLookupMap lightsMap;

		Entity::Collection entities;


	};

	template <>
	void SceneData::onPostCreateCallback<Light>(std::shared_ptr<Light>& entity);
}
