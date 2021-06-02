#pragma once

#include "ClassDef.h"
#include "gfx/Color.h"
#include "geom/Volume.h"
#include "ecs/comp/Component.h"
#include "ecs/comp/ComponentHash.h"
#include "ecs/ECS.h"
#include "scene/SceneNode.h"
#include "scene/SceneCollection.h"
#include "global/SerializableHandle.h"
#include "scripting/LuaState.h"
#include "global/PropertySet.h"

#include <string>
#include <map>
#include <typeindex>
#include <functional>
#include <vector>

namespace cs
{

	struct EntityContext
	{
		EntityContext()
			: context(nullptr)
			, luaState(nullptr)
			, parent(nullptr) { }

		EntityContext(const EntityContext& rhs)
			: context(rhs.context)
			, luaState(rhs.luaState)
			, parent(rhs.parent) { }

		void operator=(const EntityContext& rhs)
		{
			this->context = rhs.context;
			this->luaState = rhs.luaState;
			this->parent = rhs.parent;
		}

		ECSContext* context;
		LuaStatePtr luaState;
		SceneNodePtr parent;

		typedef std::function<void(Component*, LuaStatePtr&)> CallbackDataProto;

		struct CallbackData
		{
			CallbackData()
				: component(nullptr)
				, luaState(nullptr)
			{ }

			CallbackData(const CallbackData& rhs)
				: component(rhs.component)
				, luaState(rhs.luaState)
				, callback(rhs.callback)
			{ }

			CallbackData(Component* c, LuaStatePtr& s, CallbackDataProto proto)
				: component(c)
				, luaState(s)
				, callback(proto)
			{ }
			
			Component* component;
			LuaStatePtr luaState;
			CallbackDataProto callback;
		};

		std::vector<CallbackData> callbackList;
	};

	struct EntityIntersection
	{
		EntityIntersection()
			: hitPosLocal(kZero3)
			, hitPosWorld(kZero3)
			, index(-1)
			, type(SelectableVolumeTypeNone)
			, distance(0.0f)
		{ }
		
		vec3 hitPosLocal;
		vec3 hitPosWorld;
		size_t index;
		CallbackArg2<void, vec3, size_t> onChangedCallback;
		SelectableVolumeType type;
		float32 distance;
	};

	typedef std::map<uint32, ComponentPtr> ComponentMap;
	typedef std::map<std::type_index, bool> ComponentEnabledMap;
	
	CLASS_DEFINITION_DERIVED_REFLECT(Entity, SceneNode)

	public:

		typedef std::function<void(uint32, uint32, ComponentPtr&)> AddComponentFunction;
		typedef std::function<void(uint32, uint32)> RemoveComponentFunction;
		typedef std::pair<AddComponentFunction, RemoveComponentFunction> ComponentListeners;
		typedef std::vector<std::shared_ptr<Entity>> EntityList;

		typedef std::vector<ComponentListeners> ComponentListenerVector;

		typedef SceneCollection<std::string, Entity> Collection;

		static uint32 gIdCounter;

		Entity()
			: id(gIdCounter++)
			, name("")
			, enabled(true)
			, color(ColorB::White)
			, cxt(nullptr)
			, selectable(true)
			, dirtySelection(false)
			, uncoupleRotation(false)
			, propertySetHandle(CREATE_CLASS(PropertySetHandle))
		{ }

		Entity(const std::string& n, ECSContext* entityContext) 
			: id(gIdCounter++)
			, name(n)
			, enabled(true)
			, color(ColorB::White)
			, cxt(entityContext)
			, selectable(true)
			, dirtySelection(false)
			, uncoupleRotation(false)
			, propertySetHandle(CREATE_CLASS(PropertySetHandle))
		{ 
	
		}

		virtual ~Entity()
		{
			ECSContextManager::getInstance()->setContext(this->cxt);
			this->destroy();
		}

		void destroy();
		void destroyAll();

		unsigned int getId() const { return this->id; }

		const std::string& getName() { return this->name; }
		void setName(const std::string& n) { this->name = n; }

		bool getEnabled() const { return this->enabled; }
		void setEnabled(bool e) { this->enabled = e; }
		void setAllEnabled(bool e)
		{
			this->setEnabled(e);
			for (auto& it : this->children.value_vec)
			{
				it->setAllEnabled(e);
			}
		}

		void toggleEnabled() { this->enabled = !this->enabled; }
		void setSelectable(bool s) { this->selectable = s; }
		bool getSelectable() const { return this->selectable; }

		virtual size_t getSelectableVolume(SelectableVolumeList& volumes) { return 0; }

		virtual bool canAddComponent(const std::type_index& t) { return false; }

		template <typename T>
		std::shared_ptr<T> getComponent()
		{
			const std::type_index index(typeid(T));
			return std::static_pointer_cast<T>(this->getComponent(index));
		}

		std::shared_ptr<Component> getComponent(const std::type_index& index)
		{
			uint32 hash_index = getComponentHash(index);
			if (this->components.count(hash_index) != 0)
				return this->components[hash_index];
			else
				return nullptr;	
		}

		template <typename T>
		bool hasComponent() const
		{
			const std::type_index index(typeid(T));
			uint32 hash_index = getComponentHash(index);
			return this->components.count(hash_index) != 0;
		}

		template <typename T>
		void removeComponent()
		{
			std::type_index index(typeid(T));
			uint32 hash_index = getComponentHash(index);
			this->removeComponent(hash_index);
		}

		void removeComponent(uint32 hash_index)
		{
			if (components.count(hash_index) != 0)
			{
				components[hash_index]->clearParent();
				components.erase(hash_index);

				this->onComponentRemove(hash_index);
			}
		}

		template <typename T>
		static void subscribeToComponent(ECSContext* cxt, ComponentListeners& addComponent)
		{
			assert(cxt);

			std::type_index index(typeid(T));
			uint32 hash_index = getComponentHash(index);

			FunctionCallbackKey key((uintptr_t*)cxt, hash_index);
			if (Entity::callbacks.find(key) == Entity::callbacks.end())
			{
				Entity::callbacks[key] = ComponentListenerVector();
			}
			ComponentListenerVector& system_vec = Entity::callbacks[key];
			system_vec.push_back(addComponent);
		}
    
        template <typename T>
        static void removeComponentSubscription(ECSContext* cxt)
        {
            std::type_index index(typeid(T));
            uint32 hash_index = getComponentHash(index);
            
            FunctionCallbackKey key((uintptr_t*)cxt, hash_index);
            FunctionCallbacks::iterator it = Entity::callbacks.find(key);
            if (it != Entity::callbacks.end())
            {
                Entity::callbacks.erase(it);
            }
        }

		template <typename T>
		std::shared_ptr<T> addComponent(std::shared_ptr<T> component)
		{
			this->removeComponent<T>();
			std::type_index index = component->getType();
			uint32 hash_index = getComponentHash(index);

			component->setParent(this);
			components[hash_index] = component;
            std::shared_ptr<Component> compPtr = std::static_pointer_cast<Component>(component);
			this->onComponentAdd(hash_index, compPtr);

			return component;
		}

		ComponentMap& getAllComponents() { return this->components; }

		void enableComponents();
		void disableComponents();

		void traverse(void(*entityFunc)(Entity*, void*), void* data);
		void traverseScope(void(*beginFunc)(Entity*, void*), void(*endFunc)(Entity*, void*), void* data);

		template <class T>
		static bool addChild(EntityPtr& parent, std::shared_ptr<T>& child)
		{
			const EntityPtr& entity = std::static_pointer_cast<Entity>(child);
			bool added = parent->children.add(entity, entity->getName());
			if (added)
			{
				std::shared_ptr<SceneNode> parent_node = std::static_pointer_cast<SceneNode>(parent);
				entity->setParent(parent_node);
			}
			return added;
		}

		bool removeChildByName(const std::string& name);
		bool removeSelf();

		template <class T>
		bool removeChild(const std::shared_ptr<T> child_ptr)
		{
			const std::string& name = child_ptr->getName();
			return this->removeChildByName(name);
		}

		template <class T>
		std::shared_ptr<T> getChild(const std::string& name)
		{
			EntityPtr& ptr = this->children.getByKey(name);
			if (!ptr)
			{
				log::error("Cannot find ", name, " as a child of ", this->getName());
			}
			return std::static_pointer_cast<T>(ptr);
		}

		EntityPtr getEntityAt(size_t index);
		EntityPtr getEntity(const std::string& path);

		EntityPtr getParentEntity();

		virtual void collect(EntityList& list);

		const ColorB& getColor() const;
		void setColor(const ColorB& c, bool applyToChildren = false);
		void remap();

		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);
		virtual void onCustomLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);


		virtual void reset(bool active = false);
		virtual void onEdit() { }
		virtual void onEditBegin() { }
		virtual void onEditEnd() { }
		virtual void clear();

		virtual void onPositionChanged(const vec3& pos, const Transform& transform, SceneNode::UpdateType type = SceneNode::UpdateTypeNone);
		virtual void onScaleChanged(const vec3& scale, const Transform& transform, SceneNode::UpdateType type = SceneNode::UpdateTypeNone);
		virtual void onRotationChanged(const quat& rot, const Transform& transform, SceneNode::UpdateType type = SceneNode::UpdateTypeNone);

		virtual bool intersects(const Transform& transform, const Ray& ray, EntityIntersection& hit_data);

		virtual void setContext(EntityContext& context);
		virtual void setParent(SceneNodePtr& ptr);
		virtual void linkGraph(EntityPtr& parent_ptr, EntityPtr& self_ptr);

		Collection& getChildren() { return this->children; }
		size_t getNumChildren() const { return this->children.getNumElements(); }
		virtual size_t getNumEditableChildren() const { return this->getNumChildren(); }

		void setSelectionDirty(bool val) { this->dirtySelection = val; }
		bool getSelectionDirty() const { return this->dirtySelection; }

		PropertySetHandlePtr& getPropertySetHandle() { return this->propertySetHandle; }
		void setPropertySet(const std::string& propName);
		void setPropertySetHandle(PropertySetHandlePtr& ptr) { this->propertySetHandle = ptr; }
		PropertySet getPropertySet();

		void onPropertyChanged() { }

		virtual bool getRotationUncoupled() const { return this->uncoupleRotation; }
		virtual void setRotationUncoupled(bool uncouple) { this->uncoupleRotation = uncouple; }

	protected:

		friend class EntitySearchParams;

		void onComponentAdd(uint32 hash_index, ComponentPtr& component)
		{
			if (!this->cxt)
				return;

			FunctionCallbackKey key((uintptr_t*) this->cxt, hash_index);
			if (Entity::callbacks.count(key) > 0)
			{
				ComponentListenerVector& calls = Entity::callbacks[key];
				for (auto& it : calls)
                {
                    ComponentPtr compPtr = std::static_pointer_cast<Component>(component);
					it.first(this->id, hash_index, compPtr);
                }
			}
		}

		void onComponentRemove(uint32 hash_index)
		{
			if (!this->cxt)
				return;

			FunctionCallbackKey key((uintptr_t*) this->cxt, hash_index);
			if (Entity::callbacks.count(key) > 0)
			{
				ComponentListenerVector& calls = Entity::callbacks[key];
				for (auto& it : calls)
					it.second(this->id, hash_index);
			}
		}

		typedef std::pair<uintptr_t*, uint32> FunctionCallbackKey;
		typedef std::map<FunctionCallbackKey, ComponentListenerVector> FunctionCallbacks;
		static FunctionCallbacks callbacks;

		std::string name;

		int32 id;
		bool enabled;
		bool selectable;
		bool dirtySelection;
		bool uncoupleRotation;

		ColorB color;
		ComponentMap components;
		ECSContext* cxt;
		Collection children;
		
		PropertySetHandlePtr propertySetHandle;

	};

	template <>
    std::string SerializableHandle<Entity>::getExtension();

    template <>
    std::string SerializableHandle<Entity>::getDescription();

}
