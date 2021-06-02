#pragma once

#include "global/Resource.h"
#include "ecs/Entity.h"
#include "global/Event.h"

#include "scene/ReferenceNode.h"

namespace cs
{
	CLASS_DEFINITION_DERIVED_REFLECT(SceneReference, Resource)
	public:

		SceneReference() : Resource("Error"){ }
		SceneReference(const std::string& name, const std::string path, ECSContext* cxt);

		bool init();

		const ReferenceNodePtr& getReferenceNode()  { return this->reference.get(); }

	private:

		SerializableHandle<ReferenceNode> reference;
	};

	CLASS_DEFINITION_REFLECT(SceneReferenceHandle)
	public:
		
		SceneReferenceHandle()
			: reference(nullptr) { }

		SceneReferenceHandle(SceneReferencePtr& rhs)
			: reference(rhs) { }

		SceneReferenceHandle(const std::string& fileName);
		SceneReferenceHandle(SceneReferenceHandle* rhs)
			: reference((rhs && rhs->hasReference()) ? rhs->getReference() : nullptr) { }

		bool hasReference() const { return this->reference.get() != nullptr; }
		const std::string getName() const;

		void setReference(SceneReferencePtr& ref) { this->reference = ref; }
		const SceneReferencePtr& getReference() { return this->reference; }
		Event onChanged;

		void onReferenceChanged();

	private:

		SceneReferencePtr reference;
	
	};

	class SceneReferenceCache : public Singleton<SceneReferenceCache>
	{
	public:
		SceneReferenceCache();
		bool loadReference(const std::string& fileName);

		SceneReferencePtr loadReference(const std::string& fileName, const std::string& filePath);

		void clearReference(const std::string& fileName);
		void clearAllReferences();

		void addMapping(const std::string& to, const std::string& from);
		void clearMapping(const std::string& from);

	private:

		SceneReferencePtr loadReferenceInternal(const std::string& fileName);

		typedef std::map<std::string, std::string> ReferenceReplaceMap;
		typedef std::map<std::string, SceneReferencePtr> SceneReferenceMap;
		
		ReferenceReplaceMap replaceMap;
		SceneReferenceMap references;
		ECSContextPtr ecsCxt;
	};

}
