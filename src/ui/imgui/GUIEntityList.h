#pragma once

#include "scene/SceneCollection.h"
#include "ui/imgui/GUIList.h"
#include "scene/SceneData.h"

namespace cs
{

	CLASS_DEFINITION_DERIVED(GUIEntityListPopulator, GUIListPopulator)
	public:

		enum GUIEntityListCallbackType
		{
			GUIEntityListCallbackType_Add,
			GUIEntityListCallbackType_Delete,
			//...
			GUIEntityListCallbackType_MAX
		};

		struct GUIEntityListPopulatorCallbacks
		{
			GUIEntityListPopulatorCallbacks() { }
			GUIEntityListPopulatorCallbacks(const GUIEntityListPopulatorCallbacks& rhs)
				: onSelect(rhs.onSelect)
				, onDelete(rhs.onDelete) { }

			CallbackArg1<void, EntityPtr>::CallbackFunc onSelect;
			CallbackArg1<void, EntityPtr>::CallbackFunc onDelete;
		};
		
		GUIEntityListPopulator(Entity::Collection* l, GUIEntityListPopulatorCallbacks& func)
			: callbackType(GUIEntityListCallbackType_Add)
			, list(l)
			, callback(func) { }

		virtual void populate(GUIList* list);

		GUIEntityListCallbackType getCallbackType() { return this->callbackType; }
		void setCallbackType(GUIEntityListCallbackType callType) { this->callbackType = callType; }

	private:
	
		GUIEntityListCallbackType callbackType;
		Entity::Collection* list;
		GUIEntityListPopulatorCallbacks callback;
	};


}