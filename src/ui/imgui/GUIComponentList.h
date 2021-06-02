#pragma once

#include "ui/imgui/GUIList.h"
#include "ecs/Entity.h"

namespace cs
{
	CLASS_DEFINITION_DERIVED(GUIComponentListPopulator, GUIListPopulator)
	public:
		GUIComponentListPopulator(EntityPtr& e)
		: entity(e) { }

	virtual void populate(GUIList* list);

	private:

		EntityPtr entity;
	};
}