#include "PCH.h"

#include "scene/behaviors/CameraMoveBehavior.h"
#include "scene/Scene.h"
#include "ecs/comp/PhysicsComponent.h"
#include "scene/Camera.h"
#include "math/Plane.h"
#include "math/GLM.h"

namespace cs
{

	void CameraMoveBehavior::reset()
	{
		this->active = false;
	}

	bool CameraMoveBehavior::onInputDown(Scene* scene, const BehaviorData& data)
	{
		// REVISE ME - THIS IS STUPID
		/*
		SceneDataPtr& scene_data = scene->getSceneData();
		for (auto it : scene_data->getEntityList())
		{
			EntityPtr& actor = it;
			PhysicsComponentPtr phys = actor->getComponent<PhysicsComponent>();
			if (!phys.get())
				continue;

			vec3 hit_pos;
			if (phys->intersects(data.ray, hit_pos))
				return false;
		}
		*/
		this->active = true;
		return true;
	}

	bool CameraMoveBehavior::onInputMove(Scene* scene, const BehaviorData& data)
	{
		const vec3 k2DNormal = vec3(0.0f, 0.0f, -1.0f);

		if (this->active)
		{
			CameraPtr camera = scene->getCamera();
			vec3 camPos = camera->getTransform().getPosition();
			
			float32 xDiff = data.screenPos.x - data.lastSceenPos.x;
			float32 yDiff = data.screenPos.y - data.lastSceenPos.y;

			vec3 vel = vec3(xDiff, yDiff, 0.0f);
			camera->translate(camPos + vel * 2.0f);

			return true;
		}
		return false;
	}

	bool CameraMoveBehavior::onInputUp(Scene* scene, const BehaviorData& data)
	{
		this->active = false;
		return false;
	}

	bool CameraMoveBehavior::onInputWheel(Scene* scene, const BehaviorData& data)
	{
		CameraPtr camera = scene->getCamera();
		
		switch (camera->getProjectionType())
		{
			case ProjectionOrthographic:
				camera->scale(data.zoom, data.zoom, data.zoom);
				break;
			case ProjectionPerspective:
			{
				float32 zoomScale = lerp(this->minCameraZoom, this->maxCameraZoom, data.zoom);
				Transform trans = camera->getTransform();
				vec3 p = trans.getPosition();
				camera->translate(vec3(p.x, p.y, zoomScale));
			} break;
		}
		
		return false;
	}
}
