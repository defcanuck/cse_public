#include "PCH.h"

#include "ui/imgui/GUIComponentList.h"
#include "ui/imgui/GUIMetaList.h"

namespace cs
{
	void GUIComponentListPopulator::populate(GUIList* list)
	{
		list->removeAllChildren();

		cs::ComponentMap components = this->entity->getAllComponents();
		for (auto it : components)
		{
			ComponentPtr& comp = it.second;
			void* ptr = comp.get();
			const cs::MetaData* meta = comp->getMetaData();
			
			GUIMetaListPopulatorPtr& compPop = CREATE_CLASS(GUIMetaListPopulator, meta, ptr);
			const char* compName = meta->getName().c_str();
			GUIListPtr listPtr = CREATE_CLASS(GUIList, compName, std::static_pointer_cast<GUIListPopulator>(compPop));
			list->addChild(listPtr);
		}
	}
}