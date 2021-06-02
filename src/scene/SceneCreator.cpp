#include "PCH.h"

#include "scene/SceneCreator.h"

#include "ecs/comp/ComponentList.h"
#include "scene/Scene.h"

namespace cs
{

	SceneCreator::SceneCreator()
		: sceneData(nullptr)
		, parent(nullptr)
		, position(kZero3)
		, loadFlags(kLoadFlagMaskAll)
		, lightIndex(0)
	{
		createMask.set(SceneCreateComponentDrawable);
	}

	SceneCreator::SceneCreator(SceneDataPtr& data)
		: sceneData(data)
		, parent(nullptr)
		, position(kZero3)
		, loadFlags(kLoadFlagMaskAll)
		, lightIndex(0)
	{
		createMask.set(SceneCreateComponentDrawable);
	}

	SceneCreator::SceneCreator(SceneDataPtr& data, EntityPtr& p)
		: sceneData(data)
		, parent(p)
		, position(kZero3)
		, loadFlags(kLoadFlagMaskAll)
		, lightIndex(0)
	{
		createMask.set(SceneCreateComponentDrawable);
	}

	SceneCreator::SceneCreator(SceneSelectedPtr& selected)
		: sceneData(selected->scene->getSceneData())
		, parent(selected->selected)
		, position(selected->position)
		, loadFlags(kLoadFlagMaskAll)
		, lightIndex(0)
	{
		createMask.set(SceneCreateComponentDrawable);
	}

	void SceneCreator::postAddFix(SceneCreatorPtr& params, EntityPtr& entity)
	{
		params->sceneData->remap();

		entity->linkGraph(params->parent, entity);
		entity->setPosition(params->position);
	}

	EntityPtr SceneCreator::createLight(SceneCreatorPtr& params)
	{
		struct local
		{
			static void initLight(Light* light, const cs::SceneCreateComponentMask& mask)
			{

			}
		};

		LightPtr node = params->sceneData->create<Light>(
			params->name,
			&local::initLight,
			params->createMask,
			params->parent,
			params->lightIndex);

		node->onPostLoad(params->loadFlags);

		EntityPtr entity = std::static_pointer_cast<Entity>(node);
		postAddFix(params, entity);

		return entity;
	}

	EntityPtr SceneCreator::createReferenceNode(SceneCreatorPtr& params)
	{
		struct local
		{
			static void initReferenceNode(ReferenceNode* node, const SceneCreateComponentMask& mask) { }
		};


		if (params->resource_name.length() <= 0 || params->name.length() <= 0)
		{
			log::error("Failed to specify name/resource!");
			return EntityPtr();
		}

		ReferenceNodePtr node = params->sceneData->create<ReferenceNode>(
			params->name,
			&local::initReferenceNode, 
			params->createMask, 
			params->parent, 
			params->resource_name);

		node->setLuaState(params->getSceneData()->getLuaState());
		node->onPostLoad(params->loadFlags); 
		
		EntityPtr entity = std::static_pointer_cast<Entity>(node);
		postAddFix(params, entity);
		
		return entity;
	}

	EntityPtr SceneCreator::createSprite(SceneCreatorPtr& params)
	{
		struct local
		{
			static void initActor(cs::Actor* actor, const SceneCreateComponentMask& mask)
			{
				if (mask.test(SceneCreateComponentDrawable))
				{
					DrawableComponentPtr draw = ADD_COMPONENT(actor, DrawableComponent);
				}
				if (mask.test(SceneCreateComponentPhysics))
				{
					PhysicsComponentPtr phys = ADD_COMPONENT(actor, PhysicsComponent);
				}
				if (mask.test(SceneCreateComponentAnimation))
				{
					AnimationComponentPtr anim = ADD_COMPONENT(actor, AnimationComponent);
				}
			}
		};

		if (params->name.length() <= 0 || !params->sceneData.get())
		{
			log::error("Failed to specify name/data");
			return EntityPtr();
		}

		ActorPtr actor = params->sceneData->create<cs::Actor>(
			params->name,
			&local::initActor, 
			params->createMask, 
			params->parent);
		
		const DrawableComponentPtr& drawable = actor->getComponent<DrawableComponent>();
		assert(drawable.get());

		actor->onPostLoad(params->loadFlags);

		EntityPtr entity = std::static_pointer_cast<Entity>(actor);
		postAddFix(params, entity);

		return entity;
	}
}
