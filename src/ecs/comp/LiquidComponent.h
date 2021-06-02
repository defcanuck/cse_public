#pragma once

#include "ClassDef.h"

#include "ecs/comp/Component.h"

namespace cs
{
	CLASS_DEFINITION_DERIVED_REFLECT(LiquidComponent, Component)
	public:
		LiquidComponent();
		virtual ~LiquidComponent() { }

		void process(float32 dt);

	private:

	};
}