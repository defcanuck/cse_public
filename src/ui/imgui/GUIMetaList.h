#pragma once


#include "ui/imgui/GUIList.h"
#include "ecs/comp/Component.h"
#include "ui/imgui/GUIMeta.h"

#include "imgui.h"

namespace cs
{


	CLASS_DEFINITION_DERIVED(GUIMetaListPopulator, GUIListPopulator)
	public:

		GUIMetaListPopulator(const MetaData* meta, void* ptr)
			: netadata(meta) 
			, data_ptr(ptr) { }

		virtual void populate(GUIList* list);

	private:

		const MetaData* netadata;
		void* data_ptr;
	};
}