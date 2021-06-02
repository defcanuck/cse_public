#pragma once

#include "ecs/Entity.h"
#include "global/PropertySet.h"

#include <map>

namespace cs
{

	class SceneReferenceHandle;

	typedef std::map<std::string, PropertySetPtr> PropertySetOverrides;

	CLASS_DEFINITION_DERIVED_REFLECT(ReferenceNode, Entity)
	public:

		typedef std::vector<PropertySetHandleOverloadPtr> ReferenceOverloadProperty;
		typedef std::map<std::string, PropertySetHandlePtr> ReferenceOverloadPropertyMap;

		ReferenceNode(const std::string& n, ECSContext* cxt);
		ReferenceNode(const std::string& n, ECSContext* cxt, const std::string& refName);
		virtual ~ReferenceNode();

		virtual void reset(bool active = false);
		virtual bool intersects(const Transform& transform, const Ray& ray, EntityIntersection& hit_data);
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);
		size_t getSelectableVolume(SelectableVolumeList& volumes);

		virtual std::shared_ptr<SceneReferenceHandle>& getReferenceHandle() { return this->referenceHandle; }
		const std::string getReferenceName() const;

		void setContext(EntityContext& context);

		virtual void clear();

		virtual size_t getNumEditableChildren() const;

		void collectPropertySets();

		bool hasReferenceProperties() const { return this->referenceProperties.size() > 0; }
		
		virtual bool canAddComponent(const std::type_index& t);
		QuadVolumePtr getReferenceVolume();
		RectF getReferenceRect();

		void forceUpdateReferenceVolume();
		QuadVolumePtr getCachedReferenceVolume() { return this->cachedReferenceVolume; }

		static void toggleSpawnChance(bool toSpawn);
		static bool getUseChanceToSpawn();
		
		bool computeNewChanceToSpawn();

		void setLuaState(LuaStatePtr& state) { this->luaState = state; }
		
	protected:

		ReferenceNode() 
			: Entity("", nullptr)
			, useChanceToSpawn(false)
			, chanceToSpawn(100)
		{ }

		void prepareReference(EntityContext& entity_context);
		
		void copyToNode();
		void updateReferenceVolume();
		void updatePropertyOverrides();
		
		std::shared_ptr<SceneReferenceHandle> referenceHandle;
		std::shared_ptr<SceneReferenceHandle> activeHandle;
		
		PropertySetOverrides referenceProperties;
		
		LuaStatePtr luaState;
		SceneNodePtr self;
		QuadVolumePtr referenceVolume;
		QuadVolumePtr cachedReferenceVolume;

		bool editChildren;

		bool useChanceToSpawn;
		int32 chanceToSpawn;

	};

#if defined(CS_WINDOWS)
    template <>
	std::string SerializableHandle<ReferenceNode>::getExtension()  { return "entity"; }

    template <>
	std::string SerializableHandle<ReferenceNode>::getDescription() { return "CSE Reference Node"; }

#else
	template <>
	std::string SerializableHandle<ReferenceNode>::getExtension();

	template <>
	std::string SerializableHandle<ReferenceNode>::getDescription();
#endif

}
