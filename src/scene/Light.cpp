#include "PCH.h"

#include "scene/Light.h"

#include "ecs/ECS.h"
#include "scene/Scene.h"

namespace cs
{
	BEGIN_META_CLASS(Light)

		ADD_MEMBER(index);
			SET_MEMBER_IGNORE_GUI();
		ADD_MEMBER(color);
		ADD_MEMBER(ambient_intensity);
			SET_MEMBER_MIN(0.0f);
			SET_MEMBER_MAX(1.0f);
		ADD_MEMBER(diffuse_intensity);
			SET_MEMBER_MIN(0.0f);
			SET_MEMBER_MAX(8.0f);
			SET_MEMBER_DEFAULT(1.0f);
	}

	Light::Light(ECSContext* entityContext) 
		: Entity("", entityContext)
		, enabled(true)
		, index(0)
		, color(1.0f, 1.0f, 1.0f, 1.0f)
		, ambient_intensity(0.0f)
		, diffuse_intensity(1.0f)
	{

	}

	Light::Light(const std::string& name, ECSContext* entityContext, int32 idx)
		: Entity(name, entityContext)
		, enabled(true)
		, index(idx)
		, color(1.0f, 1.0f, 1.0f, 1.0f)
		, ambient_intensity(0.0f)
		, diffuse_intensity(1.0f)
	{
		
	}

	Light::~Light()
	{

	}

	void Light::onSceneAdd(SceneData* scene_ptr)
	{
		
	}

	void Light::onSceneRemove(SceneData* scene_ptr)
	{

	}

	size_t Light::getSelectableVolume(SelectableVolumeList& volumes)
	{	
		SelectableVolume vol;

		float32 adjRadius = (this->diffuse_intensity + this->ambient_intensity) * 20.0f;
		vol.volume = CREATE_CLASS(CircleVolume, kZero3, adjRadius);
		vol.type = SelectableVolumeTypeLight;

		volumes.push_back(vol);
		return volumes.size();
	}

}