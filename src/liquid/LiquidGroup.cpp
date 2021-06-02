#include "PCH.h"

#include "liquid/LiquidGroup.h"
#include "ecs/comp/LiquidComponent.h"
#include "ecs/comp/ScriptComponent.h"

namespace cs
{

	BEGIN_META_CLASS(LiquidGroup)

		ADD_MEMBER(maxParticles);
			SET_MEMBER_MIN(0);
			SET_MEMBER_MAX(500);
		
		//ADD_MEMBER_ENUM(generatorType);
		ADD_MEMBER_PTR(generator);
			ADD_COMBO_META(LiquidGeneratorRandom);
			ADD_COMBO_META(LiquidGeneratorPreset);
			ADD_COMBO_META(LiquidGeneratorGrid);

		ADD_META_FUNCTION("Convert Preset", &LiquidGroup::convertToPreset);

	END_META()

	LiquidGroup::LiquidGroup(const std::string& n, ECSContext* entityContext)
		: Entity(n, entityContext)
		, generator(nullptr)
		, context(nullptr)
		, numParticles(0)
		, maxParticles(100)
	{
		this->setSelectable(true);
	}

	LiquidGroup::~LiquidGroup()
	{
		this->removeAllParticles();
	}

	void LiquidGroup::onPostLoad(const LoadFlagMask& flags)
	{
		BASECLASS::onPostLoad(flags);

		SceneNodePtr& parent = this->getParent();
		LiquidGroup::linkToContext(parent, this);

		if (this->generator)
			this->generator->onPostLoad();
		
		this->generateParticles();

	}

	void LiquidGroup::clear()
	{
		BASECLASS::clear();
		this->context = nullptr;
	}

	void LiquidGroup::reset(bool active)
	{
		BASECLASS::reset(active);
		this->particles.clear();
		this->generateParticles();
	}

	void LiquidGroup::setLiquidContext(LiquidContextPtr& cxt)
	{
		this->context = cxt;
	}

	void LiquidGroup::linkToContext(SceneNodePtr& parent, LiquidGroup* group)
	{
		if (!parent)
		{
			log_error("Parent invalid!");
			return;
		}

		LiquidContextPtr context;
		SceneNodePtr parent_to_search = parent;
		while (!context && parent_to_search)
		{
			context = std::static_pointer_cast<LiquidContext>(parent);
			if (context)
				break;
			parent_to_search = parent_to_search->getParent();
		}

		group->setLiquidContext(context);
	}

	void LiquidGroup::setGenerator(LiquidGeneratorPtr& gen)
	{
		this->generator = gen;
		this->generateParticles();
	}

	void LiquidGroup::generateParticles()
	{
		if (!this->generator)
			return;

		vec3 offset = this->getInitialPosition();

		std::vector<vec2> positions;
		this->generator->populate(positions, this->maxParticles);
		for (auto it : positions)
		{
			vec2 pos = it;
			this->addParticle(vec2(pos.x, pos.y), vec2(offset.x, offset.y));
		}
	}

	void LiquidGroup::removeAllParticles()
	{
		if (!this->context)
			return;
		
		for (auto it : this->particles)
			this->context->removeParticle(it.first);

		this->particles.clear();
		this->numParticles = 0;
	}

	void LiquidGroup::updateParticles(float32 dt)
	{
		for (auto it : this->toDelete)
		{
			const b2ParticleHandle* handle = it;
			this->particles.erase(handle);
			this->context->removeParticle(handle);
		}

		ParticleWarpList::iterator it = this->toWarp.begin();
		while (it != this->toWarp.end())
		{
			it->second.curTime += dt;
			if (it->second.curTime > it->second.maxTime)
			{
				const b2ParticleHandle* handle = it->first;
				b2Vec2 b2NewSpawn = it->second.getSpawn();

				// "warp" the particle to it's destination
				this->context->setParticlePosition(handle, b2NewSpawn);

				b2Vec2 b2NewVelocity = it->second.getVelocity();
				this->context->setParticleVelocity(handle, b2NewVelocity);

				// remove the ownership over this movement
				++it;
				this->toWarp.erase(handle);
			}
			else 
			{
				b2Vec2 tmpPos(it->second.tempPosition.x, it->second.tempPosition.y);
				this->context->setParticlePosition(it->first, tmpPos);
				++it;
			}
		}

		this->toDelete.clear();
	}

	void LiquidGroup::removeParticle(const b2ParticleHandle* handle)
	{
		toDelete.push_back(handle);
	}

	void LiquidGroup::addParticle(const vec2 pos, const vec2 offset)
	{
		if (!this->context)
		{
			log_error("Cannot add particles without a context!");
			return;
		}

		if (!this->context->canAddParticles())
		{
			log_error("Context exceeded particle count!");
			return;
		}

		const b2ParticleHandle* handle = this->context->addParticle(pos + offset, this);
		if (handle != 0)
		{
			this->numParticles++;
			this->particles[handle] = pos;
		}
	}

	size_t LiquidGroup::getSelectableVolume(SelectableVolumeList& volumes)
	{
		if (!this->generator)
			return 0;

		SelectableVolume selectableVolume;
		selectableVolume.volume = this->generator->getVolume();
		selectableVolume.type = SelectableVolumeTypePhysics;

		volumes.push_back(selectableVolume);
		return volumes.size();
	}

	void LiquidGroup::onPositionChanged(const vec3& pos, const Transform& transform, SceneNode::UpdateType type)
	{
		if (!this->context)
			return;

		vec2 offset_vec2(pos.x, pos.y);
		ParticlePositionMap updateMap;

		for (auto it : this->particles)
		{
			const b2ParticleHandle* handle = it.first;
			updateMap[handle] = it.second + offset_vec2;
		}

		this->context->updateParticlePositions(updateMap);

		BASECLASS::onPositionChanged(pos, transform, type);
	}	

	bool LiquidGroup::getParticlePosition(const b2ParticleHandle* handle, b2Vec2& position)
	{
		if (!this->context.get())
			return false;

		return this->context->getParticlePosition(handle, position);
	}

	bool LiquidGroup::intersects(const Transform& transform, const Ray& ray, EntityIntersection& hit_data)
	{
		if (this->generator)
		{
			// return the bounds of the generator
			Ray transRay = ray;
			mat4 transMat = transform.getCurrentMatrix();
			transRay.transform(glm::inverse(transMat));

			if (this->generator->intersects(transRay, hit_data.hitPosLocal))
			{
				vec4 hp = transMat * vec4(hit_data.hitPosLocal, 1.0f);
				hit_data.hitPosWorld = vec3(hp.x, hp.y, hp.z);
				hit_data.type = SelectableVolumeTypePhysics;
				return true;
			}
		}

		// TODO:
		// return bounds of all particles in the context
		return false;
	}

	void LiquidGroup::convertToPreset()
	{
		if (!this->generator)
		{
			log_error("Cannot convert to preset - no generator defined!");
			return;
		}
		

		LiquidGeneratorPresetPtr presetPtr = CREATE_CLASS(LiquidGeneratorPreset);
		for (auto it : this->particles)
			this->context->removeParticle(it.first);

		for (auto it : this->particles)
		{
			const b2ParticleHandle* handle = it.first;
			b2Vec2 position;
			if (this->context->getParticlePosition(handle, position))
			{
				presetPtr->addParticle(vec2(position.x, position.y));
			}
		}

		this->generator = presetPtr;
	}

	bool LiquidGroup::canAddComponent(const std::type_index& t)
	{
		ComponentEnabledMap componentEnableMap;
		componentEnableMap[typeid(LiquidComponent)] = true;
		componentEnableMap[typeid(ScriptComponent)] = true;

		return componentEnableMap.count(t) != 0;
	}

	void LiquidGroup::warpParticle(const b2ParticleHandle* handle, float32 time, const VolumePtr& onEnd, const TransformPtr& transform, const vec2& onEndVelocity)
	{
		ParticleWarpList::iterator it = this->toWarp.find(handle);
		if (it != this->toWarp.end())
			return;

		WarpToVolume warpTo;
		warpTo.curTime = 0.0f;
		warpTo.maxTime = time;
		warpTo.onEndSpawn = onEnd;
		warpTo.onEndVelocity = onEndVelocity;
		warpTo.tempPosition = vec2(randomRange(-1000.0f, 1000.0f), 10000.0f);
		warpTo.onEndTransform = transform;

		this->toWarp[handle] = warpTo;
	}

}
