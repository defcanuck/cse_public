#include "PCH.h"

#include "ui/imgui/GUIList.h"

#include "imgui.h"

namespace cs
{
	const float kIndentLength = 10.0f;
	bool GUIList::prepareGUI(bool inWindow, size_t depth)
	{
		if (!this->getEnabled())
			return false;

		if (!this->initialized)
		{
			this->populator->populate(this);
			this->initialized = true;
		}

		RectF thisRect = this->getScreenRect();
		bool isChildWindow = false;

		if (thisRect.size.w > 0.0f && thisRect.size.h > 0.0f)
		{
			isChildWindow = true;
			this->collapsed = ImGui::BeginChild(this->name.c_str(), ImVec2(thisRect.size.w, thisRect.size.h), true);
		} else {
			ImGui::BeginGroup();
			ImColor color(128, 256, 0);
			ImGui::TextColored(color, this->name.c_str());
		}

		if (this->numColumns > 1)
		{
			ImGui::Columns(this->numColumns, nullptr, true);
		}

		bool ret = this->batchFields(thisRect);

		if (this->numColumns > 1)
		{
			ImGui::Columns(1);
		}

		depth++;
		
		UIElementVector frameChildren = this->children;
		for (auto it : frameChildren)
			(*it).prepareGUI(inWindow, depth);

		ImGui::Separator();

		if (isChildWindow)
			ImGui::EndChild();
		else
			ImGui::EndGroup();

		if (this->shouldRefresh)
		{
			this->setPopulatorImpl();
			this->shouldRefresh = false;
		}


		return ret;
	}

	void GUIList::setPopulatorImpl()
	{
		this->clearFields();
		if (this->populator)
			this->populator->populate(this);
	}

	
}