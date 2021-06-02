#include "PCH.h"

#include "ecs/EntitySearch.h"

namespace cs
{
	void EntitySearchParams::traverseAndCollect(EntityPtr& entity, EntitySearchParams& params)
	{
		assert(entity.get());
		for (auto& it : entity->children.value_vec)
		{
			if ((params.onCheck.get()) && params.onCheck->call<bool, Entity*>(it.get()))
			{
				params.foundList.push_back(it);
			}
			traverseAndCollect(it, params);
		}	
	}
}