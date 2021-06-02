#include "PCH.h"

#include "ui/imgui/GUIElement.h"
#include "ui/imgui/GUIContext.h"
#include "imgui.h"

namespace cs
{

	bool GUIElement::prepareGUI(bool inWindow, size_t depth)
	{

		if (!GUIContext::isContextValid())
			return false;

		if (!this->enabled)
			return false;

		RectF thisRect = this->getScreenRect();

		bool isWindow = false;
		if (!inWindow)
		{
			std::string useName = this->name;
			if (this->tag.length() > 0)
				useName = this->tag;

			this->collapsed = ImGui::Begin(useName.c_str(), &this->shouldClose, ImGuiWindowFlags_NoResize);
			isWindow = true;
			ImGui::SetWindowSize(ImVec2(thisRect.size.w, thisRect.size.h));
			ImGui::SetWindowPos(ImVec2(thisRect.pos.x, thisRect.pos.y));
		}

		bool ret = this->batchFields(thisRect);
		ret |= BASECLASS::prepareGUI(true, depth);

		if (isWindow)
			ImGui::End();
	
		return ret;
	}

	void GUIElement::clearFields()
	{
		this->fields.clear();
	}

	bool GUIElement::batchFields(const RectF& rect)
	{
		if (!this->enabled || !this->visible)
			return false;

		return this->fields.batch(rect);
	}

	void GUIElement::doCloseImpl()
	{
		if (this->onCloseCallback)
			this->onCloseCallback->invoke();
	}

	bool GUIElement::onCursor(ClickInput input, ClickParams& params, UIClickResults& results, uint32 depth)
	{
		if (!this->enabled)
			return false;
		
		for (auto it : this->children)
			(*it).onCursor(input, params, results, depth + 1);

		// ImGUI elements always consume input
		vec2 adjusted_position = params.position;
		if (this->span.getRawY().alignment == VAlignBottom)
		{
			adjusted_position.y = params.screen_dimm.y - params.position.y;
		}

		RectF rect = this->getScreenRect();
		if (rectContains<float32, vec2>(rect, adjusted_position))
		{
			return true;
		}

		return false;
	} 

}