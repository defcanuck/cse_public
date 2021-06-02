#include "PCH.h"

#include "scene/SceneData.h"

#include "ecs/comp/DrawableComponent.h"
#include "ecs/comp/PhysicsComponent.h"

#include "ecs/system/DrawableSystem.h"
#include "ecs/system/PhysicsSystem.h"
#include "ecs/system/ScriptSystem.h"
#include "ecs/system/AnimationSystem.h"
#include "ecs/system/ParticleSystem.h"
#include "ecs/system/GameSystem.h"
#include "ecs/system/AudioSystem.h"

namespace cs
{
	
	BEGIN_META_CLASS(SceneData)
		ADD_MEMBER(entities.value_vec);
	}

	template <>
	void SceneData::onPostCreateCallback<Light>(std::shared_ptr<Light>& entity)
	{
		this->lights.push_back(entity);
		this->lightsMap[entity->getIndex()] = entity;
	}
	
	SceneData::SceneData() 
		: name("default_data")
	{
		this->init();
	}

	SceneData::SceneData(const std::string& n) 
		: name(n)
	{
		this->init();
	}

    SceneData::SceneData(const std::string& n, const ECSSystemMask& mask)
    : name(n)
    {
        this->init(mask);
    }

	SceneData::~SceneData()
	{
		log::print(LogInfo, "Unloading Scene Data: ", this->name);
		this->empty();
		ECSContextManager::getInstance()->removeContext(this->ecsCxt.get());
	}

	void SceneData::init(const ECSSystemMask& mask)
	{
		this->ecsCxt = ECSContextManager::getInstance()->createContext(this->name, mask);
		this->setContext();

		// Initialize our Systems
        if (mask.test(ECSDraw)) { DrawableSystem* drawSystem = DrawableSystem::getInstance(); }
        if (mask.test(ECSPhysics)) { PhysicsSystem* physSystem = PhysicsSystem::getInstance(); }
        if (mask.test(ECSScript)) { ScriptSystem* scriptSystem = ScriptSystem::getInstance(); }
        if (mask.test(ECSAnimation)) { AnimationSystem* animationSystem = AnimationSystem::getInstance(); }
        if (mask.test(ECSParticle)) { ParticleSystem* particleSystem = ParticleSystem::getInstance(); }
        if (mask.test(ECSGame)) { GameSystem* gameSystem = GameSystem::getInstance(); }
		if (mask.test(ECSAudio)) { AudioSystem* audioSystem = AudioSystem::getInstance(); }
	}

	void SceneData::empty()
	{
		this->setContext();
		for (auto it : this->entities.value_vec)
		{
			it->clear();
		}
		this->entities.clear();
		this->lights.clear();
	}

	void SceneData::prepareSceneContext(LuaStatePtr& state)
	{
		this->setContext();
		EntityContext cxt;
		cxt.context = this->ecsCxt.get();
		cxt.luaState = state;

		this->luaState = state;
		for (auto it : this->entities.value_vec)
		{
			cxt.parent = it;
			it->setContext(cxt);
		}
	}

	void SceneData::destroy()
	{
        this->setContext();
		for (auto& it : this->entities.value_vec)
		{
			it->destroyAll();
		}
		this->entities.clear();
	}

	void SceneData::onPreLoad()
	{
		
	}

	void SceneData::onPostLoad(const LoadFlagMask& flags)
	{
		this->remap();
		for (auto it : this->entities.value_vec)
		{
			it->onPostLoad();

			const MetaData* meta = it->getMetaData();
			if (meta == MetaCreator<Light>::get())
			{
				LightPtr lightPtr = std::static_pointer_cast<Light>(it);
				this->onPostCreateCallback<Light>(lightPtr);
			}
		}
	}

	void SceneData::reset(bool active)
	{
		for (auto it : this->entities.value_vec)
		{
			it->reset(active);
		}
	}

	void SceneData::remap()
	{
		this->lightsMap.clear();
		for (auto it : this->lights)
			this->lightsMap[it->getIndex()] = it;

		this->entities.remap(&Entity::getName);
		for (auto it : this->entities.value_vec)
		{
			it->remap();
		}
	}

	LightPtr& SceneData::getLightByIndex(uint32 index)
	{
		static LightPtr kEmptyLightPtr(nullptr);
		auto it = this->lightsMap.find(index);
		if (it != this->lightsMap.end())
		{
			log::print(LogError, "Duplicate Light Index Found!");
			return it->second;
		}
		return kEmptyLightPtr;
	}

	void SceneData::setContext()
	{
		assert(this->ecsCxt);
		ECSContextManager::getInstance()->setContext(this->ecsCxt);
	}

	void SceneData::getAllEntities(Entity::EntityList& entities)
	{
		for (auto it : this->entities.value_vec)
		{
			entities.push_back(std::static_pointer_cast<Entity>(it));
			it->collect(entities);
		}
	}

	bool SceneData::intersects(Entity* entity, const Ray& ray, EntityIntersection& hit_data)
	{
		return entity->intersects(entity->getWorldTransform(), ray, hit_data);
	}

	void SceneData::printStatus()
	{
		for (auto& it : this->entities.value_vec)
		{
			log::info("[", it.use_count(), "] - ", it->getName());
		}
	}

	void SceneData::bindLights()
	{
		for (auto& light : this->lights)
		{
			std::stringstream lp;
			lp << "light_position" << light->getIndex();
			UniformPtr lightUniform = SharedUniform::getInstance().getUniform(lp.str());
			assert(lightUniform);
			lightUniform->setValue(light->getWorldPosition());

			std::stringstream di;
			di << "diffuse_intensity" << light->getIndex();
			UniformPtr diffuseIntensityUniform = SharedUniform::getInstance().getUniform(di.str());
			assert(diffuseIntensityUniform);
			diffuseIntensityUniform->setValue(light->getDiffuseIntensity());

			std::stringstream ai;
			ai << "ambient_intensity" << light->getIndex();
			UniformPtr ambientIntensityUniform = SharedUniform::getInstance().getUniform(ai.str());
			assert(ambientIntensityUniform);
			ambientIntensityUniform->setValue(light->getAmbientIntensity());
		}
	}
}
