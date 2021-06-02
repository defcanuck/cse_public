#pragma once

#include "scene/behaviors/Behavior.h"

namespace cs
{

	CLASS_DEFINITION_DERIVED(CameraMoveBehavior, Behavior)

	public:
		CameraMoveBehavior()
			: active(false)
			, scale(1.0f, 1.0f, 1.0f)
			, minCameraZoom(-10.0f)
			, maxCameraZoom(-200.0f)
		{ }
		virtual ~CameraMoveBehavior() { }

		virtual bool onInputDown(Scene* scene, const BehaviorData& data);
		virtual bool onInputMove(Scene* scene, const BehaviorData& data);
		virtual bool onInputUp(Scene* scene, const BehaviorData& data);
		virtual bool onInputWheel(Scene* scene, const BehaviorData& data);
		virtual void reset();

	private:

		bool active;
		vec3 scale;
		float32 minCameraZoom;
		float32 maxCameraZoom;
	};
}