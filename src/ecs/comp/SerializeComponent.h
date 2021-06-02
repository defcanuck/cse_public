#pragma once

#include "ecs/comp/Component.h"

#include <fstream>

namespace cs
{

	// Unit test Component Macro
	// unused for other purposes
	CLASS_DEFINITION_DERIVED_REFLECT(TestComponent, Component)
	public:
		TestComponent() : testvalue(1234) { }

		void compileMe();
	private:

		uint32 testvalue;
	};
}