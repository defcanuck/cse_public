#include "PCH.h"

#include "scene/SceneReference.h"
#include "global/ResourceFactory.h"
#include "ecs/ECS_Context.h"

#include "ecs/system/DrawableSystem.h"
#include "ecs/system/PhysicsSystem.h"

#include "os/FileManager.h"

namespace cs
{
	BEGIN_META_CLASS(SceneReferenceHandle)
		ADD_MEMBER_RESOURCE(reference);
			SET_MEMBER_CALLBACK_POST(&SceneReferenceHandle::onReferenceChanged);
	END_META();
	
	BEGIN_META_RESOURCE(SceneReference)

	END_META();

	const std::string SceneReferenceHandle::getName() const
	{ 
		return (this->reference.get()) ? this->reference->getName() : "unset"; 
	}

	SceneReference::SceneReference(const std::string& name, const std::string path, ECSContext* cxt)
		: Resource(name) 
	{ 
		this->reference = cs::SerializableHandle<ReferenceNode>(CREATE_CLASS(ReferenceNode, name, cxt));
		this->reference.setFileName(name);
		this->reference.setFilePath(path);
		this->reference.setLoadFlags(kLoadFlagMaskEmpty);
	}

	bool SceneReference::init()
	{
		return this->reference.refresh(false);
	}

	SceneReferenceHandle::SceneReferenceHandle(const std::string& fileName)
	{
		this->reference = std::static_pointer_cast<SceneReference>(
			ResourceFactory::getInstance()->loadResource<SceneReference>(fileName));
	}
	
	void SceneReferenceHandle::onReferenceChanged()
	{
		this->onChanged.invoke();
	}

	SceneReferenceCache::SceneReferenceCache()
		: ecsCxt(nullptr)
	{

	}

	bool SceneReferenceCache::loadReference(const std::string& fileName)
	{
		return this->loadReferenceInternal(fileName).get() != nullptr;
	}

	SceneReferencePtr SceneReferenceCache::loadReferenceInternal(const std::string& fileName)
	{
		std::string filePath;
		if (cs::FileManager::getInstance()->getPathToFile(fileName, filePath))
		{
			return this->loadReference(fileName, filePath);
		}
		return SceneReferencePtr();
	}

	void SceneReferenceCache::clearAllReferences()
	{
		ECSContextScope scope(this->ecsCxt);
		ResourceFactory::getInstance()->clearResourceByType<SceneReference>();
		this->references.clear();
	}

	void SceneReferenceCache::addMapping(const std::string& to, const std::string& from)
	{
		ReferenceReplaceMap::iterator it = this->replaceMap.find(from);
		if (it == this->replaceMap.end())
		{
			this->replaceMap[from] = to;
			return;
		}

		log::info("Duplicate replace entry found for ", from, " to ", it->second);
	}

	void SceneReferenceCache::clearMapping(const std::string& from)
	{
		ReferenceReplaceMap::iterator it = this->replaceMap.find(from);
		if (it != this->replaceMap.end())
		{
			this->replaceMap.erase(it);
		}
	}

	SceneReferencePtr SceneReferenceCache::loadReference(const std::string& fileName, const std::string& filePath)
	{

		bool spawnChanceScope = ReferenceNode::getUseChanceToSpawn();
		ReferenceNode::toggleSpawnChance(false);

		std::string useFileName = fileName;
		ReferenceReplaceMap::iterator replace = this->replaceMap.find(fileName);
		if (replace != this->replaceMap.end())
		{
			useFileName = replace->second;
		}

		SceneReferenceMap::iterator it = this->references.find(useFileName);

		if (it != this->references.end())
		{
			return it->second;
		}
		
		if (!this->ecsCxt.get())
		{
            this->ecsCxt = ECSContextManager::getInstance()->createContext("ReferenceCache", ECSContext::kDefaultSystems);
			
			ECSContextScope scope(this->ecsCxt);
			DrawableSystem* drawSystem = DrawableSystem::getInstance();
			PhysicsSystem* physSystem = PhysicsSystem::getInstance();
		}
		
		ECSContextScope scope(this->ecsCxt);
		std::string adjPath, adjName;
		
		FileManager::splitPath(filePath, adjName, adjPath);
		adjPath = adjPath + FileManager::getInstance()->separator();

		SceneReferencePtr ptr = CREATE_CLASS(SceneReference, useFileName, adjPath, this->ecsCxt.get());
		
		if (ptr->init())
		{
			ReferenceNode::toggleSpawnChance(spawnChanceScope);
			this->references[useFileName] = ptr;
			return ptr;
		}

		ReferenceNode::toggleSpawnChance(spawnChanceScope);
		return SceneReferencePtr();
	}

	void SceneReferenceCache::clearReference(const std::string& fileName)
	{
		SceneReferenceMap::iterator it = this->references.find(fileName);
		if (it != this->references.end())
		{
			ECSContextScope scope(this->ecsCxt);
			this->references.erase(it);
			return;
		}
		log::error("Cannot find ", fileName, " in SceneReferenceCache!");
	}


}
