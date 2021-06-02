#include "PCH.h"

#include "ecs/Entity.h"

#include "ecs/comp/DrawableComponent.h"
#include "ecs/comp/PhysicsComponent.h"
#include "ecs/comp/ScriptComponent.h"
#include "ecs/comp/CollisionComponent.h"

namespace cs
{
	uint32 Entity::gIdCounter = 0;

	struct
	{
		bool operator()(const EntityIntersection& a, const EntityIntersection& b)
		{
			return a.distance < b.distance;
		}
	} EntityIntersectionSort;

	DEFINE_META_VECTOR_NEW(Entity::Collection::ValueVector, Entity, EntityValueVector);

	DEFINE_META_MAP_NEW(ComponentMap, size_t, Component)

	BEGIN_META_CLASS(Entity)
		
		ADD_MEMBER(enabled);
		
		ADD_MEMBER(selectable);
			//SET_MEMBER_IGNORE_SERIALIZATION();
		
		ADD_MEMBER(name);
			SET_MEMBER_IGNORE_GUI();
		
		ADD_MEMBER(components);
			SET_MEMBER_IGNORE_GUI(); // Note this value is handled manually!
		
		ADD_MEMBER(children.value_vec);
			SET_MEMBER_IGNORE_GUI();

		ADD_MEMBER_PTR(propertySetHandle);
			SET_MEMBER_CALLBACK_POST(&Entity::onPropertyChanged);
			SET_MEMBER_COLLAPSEABLE();
			SET_MEMBER_START_COLLAPSED();

		ADD_MEMBER(uncoupleRotation);
			SET_MEMBER_DEFAULT(false);
	END_META()

	Entity::FunctionCallbacks Entity::callbacks = Entity::FunctionCallbacks();

	void Entity::onPositionChanged(const vec3& pos, const Transform& transform, SceneNode::UpdateType type)
	{
		for (auto& it : this->components)
			it.second->onPositionChanged(pos, transform, type);

		for (auto& it : this->children.value_vec)
		{
			const vec3 child_pos = it->getWorldPosition();
			const Transform& trans = it->getWorldTransform();

			it->onPositionChanged(child_pos, trans, type);
		}
	}
	
	void Entity::onScaleChanged(const vec3& scale, const Transform& transform, SceneNode::UpdateType type)
	{
		for (auto& it : this->components)
			it.second->onScaleChanged(scale, transform, type);

		for (auto& it : this->children.value_vec)
		{
			const vec3 child_scale = it->getWorldScale();
			const Transform& trans = it->getWorldTransform();

			it->onScaleChanged(child_scale, trans, type);
		}
	}

	void Entity::onRotationChanged(const quat& rot, const Transform& transform, SceneNode::UpdateType type)
	{
		for (auto& it : this->components)
			it.second->onRotationChanged(rot, transform, type);

		for (auto& it : this->children.value_vec)
		{
			const quat child_rot = it->getWorldRotation();
			const Transform& trans = it->getWorldTransform();

			it->onRotationChanged(child_rot, trans, type);
		}
	}

	void Entity::destroy()
	{
		if (!this->cxt)
		{
			// assert(this->components.size() == 0);
			// assert(this->children.getNumElements() == 0);
			return;
		}

		logMetaDeletion(this->getName().c_str());
		ECSContextScope scope(this->cxt);

		for (auto& it : this->components)
		{
			it.second->destroy();
			it.second->clearParent();
			this->onComponentRemove(it.first);
		}
		this->components.clear();
		this->clear();
		this->cxt = nullptr;
	}

	void Entity::destroyAll()
	{
		for (auto& it : this->children.value_vec)
		{
			it->destroyAll();
		}
		this->destroy();
	}

	void Entity::clear()
	{
		for (auto& it : this->children.value_vec)
		{
			it->clear();
		}
		this->children.clear();
		BASECLASS::clear();
	}

	void Entity::reset(bool active)
	{
		for (auto& it : this->components)
		{
			it.second->setEnabled();
			it.second->reset(active);
		}

		for (auto& it : this->children.value_vec)
		{
			it->reset(active);
		}
	}

	void Entity::onCustomLoad(const LoadFlagMask& flags)
	{
		// Load components with custom flags
		for (auto& it : this->components)
		{
			it.second->onPostLoad(flags);
		}

		// Load children with custom flags
		for (auto& it : this->children.value_vec)
		{
			it->onPostLoad(flags);
		}
	}

	void Entity::onPostLoad(const LoadFlagMask& flags)
	{

		BASECLASS::onPostLoad(flags);

		for (auto& it : this->components)
		{
			it.second->setParent(this);
			this->onComponentAdd(it.first, it.second);

			it.second->onPostLoad(flags);
		}
		

		for (auto& it : this->children.value_vec)
		{
			it->onPostLoad(flags);
		}

		this->children.remap(&Entity::getName);
	}

	void Entity::setParent(SceneNodePtr& ptr)
	{
		BASECLASS::setParent(ptr);
	}

	void Entity::linkGraph(EntityPtr& parent_ptr, EntityPtr& self_ptr)
	{
		SceneNodePtr node_ptr = std::static_pointer_cast<SceneNode>(parent_ptr);
		this->setParent(node_ptr);
		for (auto& it : this->children.value_vec)
		{
			it->linkGraph(self_ptr, it);
		}
	}

	void Entity::setContext(EntityContext& context)
	{
		// set the ECS context if necessary
		this->cxt = context.context;

		for (auto& it : this->children.value_vec)
		{
			SceneNodePtr self_node = std::static_pointer_cast<SceneNode>(context.parent);
			if (context.parent.get())
			{
				it->setParent(self_node);
			}

			EntityContext childContext(context);
			childContext.parent = it;
			it->setContext(childContext);

			if (childContext.callbackList.size() > 0)
			{
				context.callbackList.insert(context.callbackList.end(), childContext.callbackList.begin(), childContext.callbackList.end());
			}
		}

		struct local
		{
			static void onScriptCallback(Component* component, LuaStatePtr& luaState)
			{
				ScriptComponent* scriptComponent = reinterpret_cast<ScriptComponent*>(component);
				scriptComponent->setLuaState(luaState);
			}

			static void onCollisionCallback(Component* component, LuaStatePtr& luaState)
			{
				CollisionComponent* collComponent = reinterpret_cast<CollisionComponent*>(component);
				collComponent->setLuaState(luaState);
				collComponent->createInstance();
			}
		};
	
		// set the Lua state
		if (context.luaState.get() != nullptr)
		{
			ScriptComponentPtr scriptComponent = this->getComponent<ScriptComponent>();
			if (scriptComponent != nullptr)
			{
				context.callbackList.emplace_back((Component*) scriptComponent.get(), context.luaState, std::bind(&local::onScriptCallback, std::placeholders::_1, std::placeholders::_2));
			}

			CollisionComponentPtr collComponent = this->getComponent<CollisionComponent>();
			if (collComponent)
			{
				context.callbackList.emplace_back((Component*) collComponent.get(), context.luaState, std::bind(&local::onCollisionCallback, std::placeholders::_1, std::placeholders::_2));
			}
		}
	}

	bool Entity::intersects(const Transform& transform, const Ray& ray, EntityIntersection& hit_data)
	{
		bool intersects = false;

		// Intersect with the drawable component first
		DrawableComponentPtr draw = this->getComponent<DrawableComponent>();
		if (draw)
		{

			Ray transRay = ray;
			mat4 transMat = this->getWorldTransform().getCurrentMatrix();
			transRay.transform(glm::inverse(transMat));

			RenderablePtr& rend = draw->getRenderable();
			SelectableVolumeList selectable_volumes;
			std::vector<EntityIntersection> hit_volumes;
			rend->getSelectableVolume(selectable_volumes);

			for (auto& it : selectable_volumes)
			{
				if (!it.volume)
					continue;
				
				EntityIntersection data;
				if (it.volume->intersects(transRay, data.hitPosLocal))
				{
					
					vec4 hp = transMat * vec4(data.hitPosLocal, 1.0f);
					data.hitPosWorld = vec3(hp.x, hp.y, hp.z);
					data.index = it.indexData;
					data.type = it.type;
					data.onChangedCallback = it.onChangedCallback;
					data.distance = glm::distance(data.hitPosLocal, it.volume->getCenter());
					
					hit_volumes.push_back(data);
				}				
			}

			if (hit_volumes.size() == 0)
				return false;

			std::sort(hit_volumes.begin(), hit_volumes.end(), EntityIntersectionSort);
			hit_data = hit_volumes[0];
			return true;
		}

		// fall back to physics component
		PhysicsComponentPtr phys = this->getComponent<PhysicsComponent>();
		if (phys)
			return phys->intersects(ray, hit_data.hitPosWorld);

		return false;
	}

	void Entity::setPropertySet(const std::string& propName)
	{
		this->propertySetHandle = CREATE_CLASS(PropertySetHandle, propName);
	}

	PropertySet Entity::getPropertySet()
	{
		if (this->propertySetHandle.get())
			return this->propertySetHandle->getPropertySet();
		return PropertySet::kEmptyPropertySet;
	}

	void Entity::enableComponents()
	{
		for (auto& it : this->components)
		{
			it.second->setDisabled();
		}
	}

	void Entity::disableComponents()
	{
		for (auto& it : this->components)
		{
			it.second->setEnabled();
		}
	}

	void Entity::traverse(void(*entityFunc)(Entity*, void*), void* data)
	{
		entityFunc(this, data);
		for (auto& it : this->children.value_vec)
		{
			it->traverse(entityFunc, data);
		}
	}

	void Entity::traverseScope(void(*beginFunc)(Entity*, void*), void(*endFunc)(Entity*, void*), void* data)
	{
		beginFunc(this, data);
		for (auto& it : this->children.value_vec)
		{
			it->traverseScope(beginFunc, endFunc, data);
		}
		endFunc(this, data);
	}

	bool Entity::removeChildByName(const std::string& name)
	{
		EntityPtr entity = this->getChild<Entity>(name);
		if (!entity.get())
		{
			log::error("No child ", name, " found in ", this->getName());
			return false;
		}
		bool removed = this->children.removeByKey(name);
		if (removed)
		{
			entity->clearParent();
		}
		return removed;
	}

	bool Entity::removeSelf()
	{
		EntityPtr parentEntity = this->getParentEntity();
		if (!parentEntity.get())
		{
			log::info("Cannot remove self - no parent entity");
			return false;
		}
		return parentEntity->removeChildByName(this->getName());
	}

	EntityPtr Entity::getEntityAt(size_t index)
	{
		if (index >= this->children.getNumElements())
		{
			log::error("Index out of range: ", index, " of size ", this->children.getNumElements());
			return EntityPtr();
		}

		return this->children.getAt(index);
	}

	EntityPtr Entity::getEntity(const std::string& path)
	{
		size_t delim = path.find_first_of("/");
		if (delim != std::string::npos)
		{
			std::string lhs = path.substr(0, delim);
			std::string rhs = path.substr(delim + 1, std::string::npos);
			if (lhs == "..")
			{
				EntityPtr parent_ptr = this->getParentEntity();
				if (parent_ptr.get())
					return parent_ptr->getEntity(rhs);
				else
				{
					log::error("Parent was null - did you use too many '..' in your path?");
					return EntityPtr();
				}
			}

			EntityPtr child = this->getChild<Entity>(lhs);
			if (!child.get())
			{
				log::error("No child found for ", lhs);
				return EntityPtr();
			}

			return child->getEntity(rhs);
		}

		return this->getChild<Entity>(path);
	}

	EntityPtr Entity::getParentEntity()
	{
		return std::static_pointer_cast<Entity>(this->parent);
	}

	void Entity::collect(EntityList& list)
	{
		for (auto& it : this->children.value_vec)
		{
			list.push_back(it);
			it->collect(list);
		}
	}

	const ColorB& Entity::getColor() const
	{
		return this->color;
	}

	void Entity::setColor(const ColorB& c, bool applyToChildren)
	{
		this->color = c;

		if (applyToChildren)
		{
			for (auto& it : this->children.value_vec)
			{
				it->setColor(this->color);
			}
		}
	}

	void Entity::remap()
	{
		this->children.remap(&Entity::getName);
		for (auto& it : this->children.value_vec)
		{
			it->remap();
		}
	}


    template <>
    std::string SerializableHandle<Entity>::getExtension()
    {
        return "entity";
    }
    
    template <>
    std::string SerializableHandle<Entity>::getDescription()
    {
        return "CSE Entity";
    }
	
}
