#include "PCH.h"

#include "ui/imgui/GUIEntityList.h"

#include "ui/behavior/UIClickBehavior.h"
#include "ecs/comp/ScriptComponent.h"

#include "imgui.h"

#include <utility>

namespace cs
{
	const float32 kEntityListIndent = 15.0f;
	void populateEntityCollection(
		GUIList* gui_list, 
		cs::Entity::Collection* list, 
		GUIEntityListPopulator::GUIEntityListPopulatorCallbacks& callback, 
		GUIEntityListPopulator::GUIEntityListCallbackType callbackType,
		float32 indent = 0.0f)
	{
		RectF thisRect = gui_list->getScreenRect();

		size_t num_elements = list->getNumElements();
		for (size_t i = 0; i < num_elements; i++)
		{
			EntityPtr& entity = list->getAt(i);
			std::stringstream str;
			str << entity->getName() << "[" << entity->getId() << "]";
			const std::string& elemName = str.str();

			CallbackArg1<void, EntityPtr>* ptr = nullptr;
			GUIValue::GUIStyleColors styleColors;

			if (entity->hasComponent<ScriptComponent>())
			{
				styleColors.push_back(std::pair<int32, ImVec4>(ImGuiCol_Button, ImVec4(1.0f, 0.5f, 0.0f, 1.0f)));
			}

			switch (callbackType)
			{
				case GUIEntityListPopulator::GUIEntityListCallbackType_Add:
					ptr = new CallbackArg1<void, EntityPtr>(callback.onSelect, entity);
					break;
				case GUIEntityListPopulator::GUIEntityListCallbackType_Delete:
					ptr = new CallbackArg1<void, EntityPtr>(callback.onDelete, entity);
					styleColors.clear();
					styleColors.push_back(std::pair<int32, ImVec4>(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)));
					break;
			}

			CallbackPtr add_callback = std::static_pointer_cast<Callback>(
				std::shared_ptr<CallbackArg1<void, EntityPtr>>(ptr));

			GUIValueButton* button = new GUIValueButton(elemName, add_callback, vec2(thisRect.size.w * 0.8f, 0.0f));
			button->colors = styleColors;

			gui_list->addFieldPtr(button, indent);

			if (entity->getNumEditableChildren() > 0)
			{
				cs::Entity::Collection& child_collection = entity->getChildren();
				populateEntityCollection(gui_list, &child_collection, callback, callbackType, indent + kEntityListIndent);
			}
		}
	}

	void GUIEntityListPopulator::populate(GUIList* gui_list)
	{
		populateEntityCollection(gui_list, this->list, this->callback, this->callbackType);
	}
}