#pragma once

#include "ClassDef.h"
#include "os/InputManager.h"
#include "math/Ray.h"

namespace cs
{
	class Scene;

	struct BehaviorData
	{
		Ray ray;
		float32 zoom;
		vec2 screenPos;
		vec2 lastSceenPos;
		ClickInput input;
	};

	CLASS_DEFINITION(Behavior)
	
	public:

		Behavior() : enabled(true) { }
		virtual ~Behavior() { }

		virtual bool onInputDown(Scene* scene, const BehaviorData& data) { return false; }
		virtual bool onInputUp(Scene* scene, const BehaviorData& data) { return false; }
		virtual bool onInputMove(Scene* scene, const BehaviorData& data) { return false; }
		virtual bool onInputPressed(Scene* scene, const BehaviorData& data) { return false; }
		virtual bool onInputReleased(Scene* scene, const BehaviorData& data) { return false; }
		virtual bool onInputWheel(Scene* scene, const BehaviorData& data) { return false; }
		virtual void reset() { }

		void setEnabled(bool e) { this->enabled = e; }
		bool getEnabled() const { return this->enabled; }

	protected:

		bool enabled;
	};
}