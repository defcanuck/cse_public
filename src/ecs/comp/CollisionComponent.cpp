#include "PCH.h"
#include "ecs/Entity.h"
#include "ecs/comp/CollisionComponent.h"

namespace cs
{

	Entity* CollisionScriptInstance::getEntity()
	{ 
		return this->entity; 
	}

	BEGIN_META_CLASS(CollisionComponent)
		ADD_MEMBER_PTR(volume);
		ADD_COMBO_META_LABEL(QuadVolume, "Quad");
		ADD_COMBO_META_LABEL(CircleVolume, "Circle");
		ADD_COMBO_META_LABEL(EllipseVolume, "Ellipse");
		SET_MEMBER_CALLBACK_POST(&CollisionComponent::onVolumeChangedCallback);
		ADD_MEMBER(notifyOnMoved);
		ADD_MEMBER_PTR(onHitScript);
			SET_MEMBER_CALLBACK_POST(&CollisionComponent::onScriptChanged);
	END_META()

	CollisionComponent::CollisionComponent()
		: boundingVolume(nullptr)
		, initialized(false)
		, inverseMatrix(1.0f)
		, notifyOnMoved(false)
		, hasMoved(false)
		, luaState(nullptr)
		, onHitScript(CREATE_CLASS(LuaScriptHandle))
		, scriptLoaded(false)
	{ }

	bool CollisionComponent::intersect(const Ray& ray, const Transform& trans, HitParams& params)
	{
		if (this->boundingVolume)
			return this->boundingVolume->intersect(ray, trans, params);

		return false;
	}

	bool CollisionComponent::test(const vec3& pos) const
	{
		if (this->volume.get())
		{
			return this->volume->test(pos);
		}
		return false;
	}


	void CollisionComponent::onScriptChanged()
	{
		this->createInstance();
	}

	void CollisionComponent::onVolumeChangedCallback()
	{
		
	}

	const mat4& CollisionComponent::getInverseMatrix()
	{
		if (!this->initialized)
		{
			Entity* parent = this->getParent();
			if (!parent)
				return this->inverseMatrix;

			Transform transform = parent->getWorldTransform();
			this->inverseMatrix = glm::inverse(transform.getCurrentMatrix());
		}

		return this->inverseMatrix;
	}

	void CollisionComponent::refreshScript()
	{
		if (!this->onHitScript)
		{
			log::error("No script handle to create Script Instance");
			return;
		}

		if (this->scriptLoaded)
		{
			log::info("Script already loaded!");
			return;
		}

		this->scriptLoaded = this->onHitScript->load(this->luaState);
		if (!this->scriptLoaded)
		{
			log::error("Script failed to load");
		}
	}

	void CollisionComponent::createInstance()
	{
		this->refreshScript();

		if (this->scriptLoaded)
		{
			Entity* entity = this->getParent();
			if (entity)
			{
				const LuaScriptFilePtr& reference = this->onHitScript->getReference();
				std::string name = FileManager::stripExtension(reference->getName());

				luabind::object obj = this->luaState->createObject(name, entity);

				if (obj.is_valid())
				{
					this->instance = LuaObjectPtr<CollisionScriptInstance>(obj);
				}
			}
		}
	}

	void CollisionComponent::onPostLoad(const LoadFlagMask& flags)
	{
		if (this->onHitScript)
		{
			this->onHitScript->onChanged += createCallbackArg0(&CollisionComponent::onScriptChanged, this);
		}

		if (this->luaState && !this->scriptLoaded)
			this->createInstance();
	}

	bool CollisionComponent::load(LuaStatePtr& state)
	{
		this->luaState = state;
		this->createInstance();

		return this->scriptLoaded;
	}

	void CollisionComponent::onMoved()
	{
		if (!this->instance.exists())
			return;
 
		this->instance->onMoved();
	}
}