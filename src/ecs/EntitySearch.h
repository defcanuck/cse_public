#pragma once

#include "ClassDef.h"
#include "ecs/Entity.h"

namespace cs
{
	
	typedef SharedList<Entity> EntitySharedList;

	CLASS_DEFINITION(EntitySearchParams)
	public:
		EntitySearchParams() { }

		LuaCallbackPtr onCheck;
		EntitySharedList foundList;

		static void traverseAndCollect(EntityPtr& entity, EntitySearchParams& params);
};
}