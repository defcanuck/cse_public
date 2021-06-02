#include "PCH.h"

#include "scene/ReferenceNode.h"
#include "scene/SceneReference.h"

#include "ecs/comp/ComponentList.h"
#include "ecs/comp/ScriptComponent.h"

namespace cs
{
	bool gUseChanceToSpawn = false;

	DEFINE_META_VECTOR_NEW(ReferenceNode::ReferenceOverloadProperty, PropertySetHandleOverload, ReferenceOverloadProperty);

	DEFINE_META_MAP_NEW(PropertySetOverrides, std::string, PropertySet);

	BEGIN_META_CLASS(ReferenceNode)
		ADD_MEMBER_PTR(referenceHandle);
		ADD_MEMBER_OVERRIDE_FLAGS(children.value_vec, Member::MemberFlagIgnoreSerialization);
		ADD_MEMBER(referenceProperties);
		ADD_META_FUNCTION("Collect Properties", &ReferenceNode::collectPropertySets);
		ADD_META_FUNCTION("Refresh Volume", &ReferenceNode::forceUpdateReferenceVolume);
		ADD_MEMBER(editChildren);
			SET_MEMBER_IGNORE_SERIALIZATION();
		ADD_MEMBER_PTR(cachedReferenceVolume);
		ADD_MEMBER(useChanceToSpawn);
			SET_MEMBER_DEFAULT(false);
		ADD_MEMBER(chanceToSpawn);
			SET_MEMBER_DEFAULT(100);
	END_META()

	QuadVolumePtr kReferenceQuadVolume = CREATE_CLASS(QuadVolume, RectF(-50.0f, -50.0f, 100.0f, 100.0f));
	
	void ReferenceNode::toggleSpawnChance(bool toSpawn)
	{
		gUseChanceToSpawn = toSpawn;
	}

	bool ReferenceNode::getUseChanceToSpawn()
	{
		return gUseChanceToSpawn;
	}

	ReferenceNode::ReferenceNode(const std::string& n, ECSContext* cxt)
		: Entity(n, cxt) 
		, referenceHandle(CREATE_CLASS(SceneReferenceHandle))
		, activeHandle(CREATE_CLASS(SceneReferenceHandle))
		, luaState(nullptr)
		, self(nullptr)
		, referenceVolume(nullptr)
		, editChildren(false)
		, useChanceToSpawn(false)
		, chanceToSpawn(100)
	{ 
		this->referenceHandle->onChanged += createCallbackArg0(&ReferenceNode::copyToNode, this);
	}

	ReferenceNode::ReferenceNode(const std::string& n, ECSContext* cxt, const std::string& fileName)
		: Entity(n, cxt)
		, referenceHandle(CREATE_CLASS(SceneReferenceHandle, fileName))
		, activeHandle(CREATE_CLASS(SceneReferenceHandle))
		, luaState(nullptr)
		, self(nullptr)
		, referenceVolume(nullptr)
		, editChildren(false)
		, useChanceToSpawn(false)
		, chanceToSpawn(100)
	{
		this->referenceHandle->onChanged += createCallbackArg0(&ReferenceNode::copyToNode, this);
	}

	ReferenceNode::~ReferenceNode()
	{
		
	}

	const std::string ReferenceNode::getReferenceName() const
	{
		if (!this->referenceHandle.get())
			return "";

		return this->referenceHandle->getName();
	}

	bool ReferenceNode::intersects(const Transform& transform, const Ray& ray, EntityIntersection& hit_data)
	{
		if (this->editChildren)
			return false;

		Ray transRay = ray;
		mat4 transMat = this->getWorldTransform().getCurrentMatrix();
		transRay.transform(glm::inverse(transMat));

		QuadVolumePtr intersectVolume = kReferenceQuadVolume;
		if (this->referenceVolume)
			intersectVolume = this->referenceVolume;

		if (intersectVolume->intersects(transRay, hit_data.hitPosLocal))
		{
			vec4 hp = transMat * vec4(hit_data.hitPosLocal, 1.0f);
			hit_data.hitPosWorld = vec3(hp.x, hp.y, hp.z);
			hit_data.type = SelectableVolumeTypeReference;
			return true;
		}

		return false;
	}

	void ReferenceNode::forceUpdateReferenceVolume()
	{
		this->cachedReferenceVolume = nullptr;
		this->referenceVolume = nullptr;

		this->updateReferenceVolume();

		this->cachedReferenceVolume = referenceVolume;
	}

	void ReferenceNode::updateReferenceVolume()
	{
		std::shared_ptr<SceneReferenceHandle>& handle = this->getReferenceHandle();
		if (!handle.get() || !handle->hasReference())
		{
			log::info("No reference volume available - empty ref handle");
			return;
		}

		if (this->cachedReferenceVolume.get())
		{
			this->referenceVolume = this->cachedReferenceVolume;
			return;
		}

		struct ReferenceNodeChild
		{
			SelectableVolumeList volumes;
			mat4 transform;
		};
		std::vector<ReferenceNodeChild> children_nodes;

		for (auto it : this->children.value_vec)
		{
			children_nodes.push_back(ReferenceNodeChild());
			ReferenceNodeChild& child_node = children_nodes.back();

			it->getSelectableVolume(child_node.volumes);
			child_node.transform = it->getLocalTransform().getCurrentMatrix();
		}

		if (children_nodes.size() > 0)
		{   
			FloatExtentCalculator xVal;
			FloatExtentCalculator yVal;
		
			for (auto it : children_nodes)
			{
				std::vector<vec3> volume_positions;
				ReferenceNodeChild& node = it;

				for (auto vol : node.volumes)
				{
					if (!vol.volume.get())
						continue;

					size_t num_pos = vol.volume->getPositions(volume_positions);
					for (size_t i = 0; i < num_pos; i++)
					{
						vec4 pos_val = it.transform * vec4(volume_positions[i], 1.0f);
						xVal.evaluate(pos_val.x);
						yVal.evaluate(pos_val.y);
					}
				}
			}

			this->referenceVolume = CREATE_CLASS(QuadVolume, xVal.minValue, yVal.minValue, xVal.span(), yVal.span());
		}
		else
		{
			this->referenceVolume = CREATE_CLASS(QuadVolume, 0.0f, 0.0f, 0.0f, 0.0f);
		}
	}

	size_t ReferenceNode::getSelectableVolume(SelectableVolumeList& volumes)
	{
		std::shared_ptr<SceneReferenceHandle>& handle = this->getReferenceHandle();
		if (!this->editChildren)
		{
			if (handle.get() && handle->hasReference())
			{
				if (!this->referenceVolume.get())
				{
					this->updateReferenceVolume();
				}
				assert(this->referenceVolume.get());

				SelectableVolume selectable_volume;
				selectable_volume.volume = this->referenceVolume;
				selectable_volume.type = SelectableVolumeTypeReference;
				volumes.push_back(selectable_volume);
			}
			else
			{
				SelectableVolume selectable_volume;
				selectable_volume.volume = kReferenceQuadVolume;
				selectable_volume.type = SelectableVolumeTypeReference;
				volumes.push_back(selectable_volume);
			}
		}

		return volumes.size();
	}

	size_t ReferenceNode::getNumEditableChildren() const
	{
		if (this->referenceHandle.get() && this->referenceHandle->hasReference())
		{
			if (this->editChildren)
				return BASECLASS::getNumChildren();
			
			return 0;
		}

		return BASECLASS::getNumChildren();
	}

	QuadVolumePtr ReferenceNode::getReferenceVolume() 
	{ 
		if (!this->referenceVolume.get())
		{
			this->updateReferenceVolume();
		}
		return this->referenceVolume; 
	}

	RectF ReferenceNode::getReferenceRect()
	{
		QuadVolumePtr quadPtr = this->getReferenceVolume();
		if (!quadPtr.get())
		{
			this->updateReferenceVolume();
		}
		
		return (quadPtr.get()) ? quadPtr->getRect() : RectF();
	}

	void ReferenceNode::prepareReference(EntityContext& entity_context)
	{
		std::shared_ptr<SceneReferenceHandle>& handle = this->getReferenceHandle();
		if (handle.get())
		{
			if (handle->hasReference())
			{
				this->copyToNode();

				// This is the live scene path
				entity_context.context = this->cxt;
				entity_context.luaState = this->luaState;
				entity_context.parent = this->self;

				this->setContext(entity_context);
			}
			else
			{
				// likely we're in the reference cache now => make sure to bubble up the 
				// reference contexts to other entities before proceeding to post load
				entity_context.context = this->cxt;
				entity_context.luaState = this->luaState;

				this->setContext(entity_context);
			}
		}
	}

	void ReferenceNode::onPostLoad(const LoadFlagMask& flags)
	{
		EntityContext entity_context;
		this->prepareReference(entity_context);
	
		BASECLASS::onPostLoad(flags);
		
		this->updateReferenceVolume();

		// execute callbacks
		for (auto& it : entity_context.callbackList)
		{
			it.callback(it.component, it.luaState);
		}
	
	}

	void ReferenceNode::reset(bool active)
	{
		this->updatePropertyOverrides();
		BASECLASS::reset(active);
	}

	void ReferenceNode::updatePropertyOverrides()
	{
		if (this->referenceProperties.size() == 0)
			return; 

		struct PropetryTraverse
		{
			static void overloadPropertyHandles(Entity* entity, void* data)
			{
				PropertySetOverrides* overloadProperties = reinterpret_cast<PropertySetOverrides*>(data);
				assert(data);

				ScriptComponentPtr scriptComponent = entity->getComponent<ScriptComponent>();
				if (scriptComponent)
				{
					const std::string& entity_name = entity->getName();
					PropertySetOverrides::iterator it;
					if ((it = (*overloadProperties).find(entity_name)) != (*overloadProperties).end())
					{
						PropertySetPtr& prop_ptr = it->second;
						scriptComponent->setActivePropertySet(prop_ptr);
					}
				}
			}
		};

		this->traverse(&PropetryTraverse::overloadPropertyHandles, (void*)&this->referenceProperties);
	}

	bool ReferenceNode::computeNewChanceToSpawn() 
	{ 
		if (!this->useChanceToSpawn)
		{
			return true;
		}
		return (rand() % 100) < this->chanceToSpawn;
	}

	void ReferenceNode::copyToNode()
	{
		std::shared_ptr<SceneReferenceHandle>& handle = this->getReferenceHandle();

		if (!handle->hasReference())
		{
			log::info("No reference to copy!");
			return;
		}

		if (!this->activeHandle || 
			!this->activeHandle->hasReference() ||
			(this->activeHandle->getReference().get() != handle->getReference().get()))
		{

			for (auto it : this->children.value_vec)
			{
				it->clear();
			}
			this->children.clear();

			const MetaData* metadata = MetaCreator<ReferenceNode>::get();
			const SceneReferencePtr& sceneReference = handle->getReference();
			const ReferenceNodePtr& referenceNode = sceneReference->getReferenceNode();

			// Inject a chance param to spawn this node
			bool spawnChance = !gUseChanceToSpawn || this->computeNewChanceToSpawn();
			if (!spawnChance)
			{
				return;
			}

			StringList members_to_copy;
			members_to_copy.push_back("children.value_vec");

			this->cachedReferenceVolume = referenceNode->getCachedReferenceVolume();
			
			if (this->referenceProperties.size() == 0 &&
				referenceNode->hasReferenceProperties())
			{
				log::info("Copying over Reference properties for Reference ", sceneReference->getName(), " to agent ", this->getName());
				members_to_copy.push_back("referenceProperties");
			}

			void* ref = (void*)referenceNode.get();
			for (auto it : members_to_copy)
			{
				const std::string& memberName = it;
				const Member* member = metadata->getMember(memberName);
				assert(member);
				const MetaData* member_metadata = member->getMetaData();
				assert(member_metadata);

				void* dst = PTR_ADD(this, member->getOffset());
				void* src = PTR_ADD(ref, member->getOffset());

				member_metadata->copy(dst, src);
			}

			this->activeHandle = CREATE_CLASS(SceneReferenceHandle, handle.get());

			// Remap all subsequent nodes
			this->remap();

			// Reset all properties
			this->reset();

		}
	}

	void ReferenceNode::collectPropertySets()
	{
		struct local
		{
			static void collectScriptProperties(Entity* entity, void* data)
			{
				PropertySetOverrides* overload_map = reinterpret_cast<PropertySetOverrides*>(data);

				ScriptComponentPtr script_component = entity->getComponent<ScriptComponent>();
				if (script_component.get() && script_component->canOverrideProperties())
				{
					const std::string& entity_name = entity->getName();
					PropertySetHandlePtr& property_handle = script_component->getPropertySet();
					if (property_handle->hasPropertySetResource())
					{
						PropertySetResourcePtr& property_set_resource = property_handle->getPropertySetResource();
						assert(property_set_resource.get());

						const PropertySetPtr property_set = property_set_resource->getPropertySetPtr();
						(*overload_map)[entity_name] = CREATE_CLASS(PropertySet, property_set);
					}
				}
			}
		};
		this->referenceProperties.clear();
		this->traverse(&local::collectScriptProperties, (void*) &this->referenceProperties);
	}

	void ReferenceNode::setContext(EntityContext& context)
	{
		this->luaState = context.luaState;
		this->self = context.parent;
		
		BASECLASS::setContext(context);
	}

	void ReferenceNode::clear()
	{
		this->self.reset();
		BASECLASS::clear();
	}

	bool ReferenceNode::canAddComponent(const std::type_index& t)
	{
		ComponentEnabledMap componentEnableMap;
		componentEnableMap[typeid(GameComponent)] = true;

		return componentEnableMap.count(t) != 0;
	}
    
#if !defined(CS_WINDOWS)
    template <>
    std::string SerializableHandle<ReferenceNode>::getExtension()
    {
        return "entity";
    }
    
    template <>
	std::string SerializableHandle<ReferenceNode>::getDescription()
	{
		return "CSE Reference Node";
	}
#endif

}
