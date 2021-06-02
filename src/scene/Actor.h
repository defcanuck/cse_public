#pragma once

#include "ClassDef.h"
#include "math/Transform.h"
#include "ecs/Entity.h"

#include <string>

namespace cs
{
	class SceneData;

	CLASS_DEFINITION_DERIVED_REFLECT(Actor, Entity)
	
		public:

			Actor(const std::string& nname, ECSContext* entityContext);
			virtual ~Actor();

			virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);

			virtual size_t getSelectableVolume(SelectableVolumeList& volumes);

			virtual bool canAddComponent(const std::type_index& t);

			virtual void setRenderableSize(vec2 sz, AnimationScaleType type);
			virtual void setRenderableUV(RectF uv);

		private:

			// hack
			Actor() : Entity("", nullptr) { }

	};
}

