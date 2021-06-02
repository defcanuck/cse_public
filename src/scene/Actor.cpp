#include "PCH.h"

#include "scene/Actor.h"
#include "scene/Scene.h"

#include "ecs/comp/ComponentList.h"
#include "ecs/ECS.h"

#include "ecs/comp/AnimationComponent.h"

namespace cs
{

	BEGIN_META_CLASS(Actor)

	}

	Actor::Actor(const std::string& n, ECSContext* entityContext) 
		: Entity(n, entityContext)
	{

	}

	Actor::~Actor()
	{

	}

	void Actor::onPostLoad(const LoadFlagMask& flags)
	{
		BASECLASS::onPostLoad(flags);
	}

	size_t Actor::getSelectableVolume(SelectableVolumeList& volumes)
	{
		
		const PhysicsComponentPtr& phys = this->getComponent<PhysicsComponent>();
		if (phys)
		{
			phys->getSelectableShapes(volumes);
		}

		const DrawableComponentPtr& draw = this->getComponent<DrawableComponent>();
		if (draw)
		{
			draw->getSelectableVolume(volumes);
		}

		const AnimationComponentPtr& anim = this->getComponent<AnimationComponent>();
		if (anim)
		{
			anim->getSelectableVolume(volumes);
		}

		if (volumes.size() > 0)
			return volumes.size();

		SelectableVolume vol;
		vol.volume = CREATE_CLASS(CircleVolume);
		volumes.push_back(vol);
		return volumes.size();	
	}

	void Actor::setRenderableSize(vec2 sz, AnimationScaleType type)
	{
		DrawableComponentPtr draw = this->getComponent<DrawableComponent>();
		if (!draw.get())
		{
			log::info("No DrawableComponent for setRenderableSize");
			return;
		}
		RenderablePtr& rend = draw->getRenderable();
		if (!rend.get())
		{
			log::info("No Renderable in DrawableComponent for setRenderableSize");
			return;
		}
		rend->adjustSize(sz, type);
	}

	void Actor::setRenderableUV(RectF uv)
	{
		DrawableComponentPtr draw = this->getComponent<DrawableComponent>();
		if (!draw.get())
		{
			log::info("No DrawableComponent for setRenderableUV");
			return;
		}
		RenderablePtr& rend = draw->getRenderable();
		if (!rend.get())
		{
			log::info("No Renderable in DrawableComponent for setRenderableUV");
			return;
		}
		rend->adjustTexRect(uv);
	}

	bool Actor::canAddComponent(const std::type_index& t)
	{
		ComponentEnabledMap componentEnableMap;
		componentEnableMap[typeid(DrawableComponent)] = true;
		componentEnableMap[typeid(PhysicsComponent)] = true;
		componentEnableMap[typeid(ScriptComponent)] = true;
		componentEnableMap[typeid(AnimationComponent)] = true;
		componentEnableMap[typeid(ParticleComponent)] = true;
		componentEnableMap[typeid(GameComponent)] = true;
		componentEnableMap[typeid(CollisionComponent)] = true;
		componentEnableMap[typeid(AudioComponent)] = true;

		return componentEnableMap.count(t) != 0;
	}
}
