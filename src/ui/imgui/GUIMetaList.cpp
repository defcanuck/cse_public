#include "PCH.h"

#include "ui/imgui/GUIMetaList.h"
#include "global/Utils.h"

namespace cs
{
	void GUIMetaListPopulator::populate(GUIList* list)
	{
		GUICreatorFactory::populate(this->netadata, this->data_ptr, list->getFields(), 0.0f);
	}
}