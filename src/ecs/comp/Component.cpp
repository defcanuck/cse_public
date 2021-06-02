#include "PCH.h"

#include "ecs/comp/Component.h"
#include "ecs/Entity.h"

namespace cs
{
	BEGIN_META_CLASS(Component)
		// Nada
	END_META()

	unsigned int Component::getId()
	{
		if (parent)
			return parent->getId();
		return 0;
	}
}