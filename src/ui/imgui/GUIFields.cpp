#include "PCH.h"

#include "ui/imgui/GUIFields.h"
#include "ui/imgui/GUIValue.h"

#include "imgui.h"

namespace cs
{

	GUIFieldEntry::GUIFieldEntry()
		: field(nullptr)
		, indent(0.0f) { }

	GUIFieldEntry::~GUIFieldEntry()
	{
		
	}

	GUIFieldEntry::GUIFieldEntry(const GUIFieldEntry& rhs)
	{
		this->field = rhs.field;
		this->indent = rhs.indent;
	}

	void GUIFieldEntry::operator=(const GUIFieldEntry& rhs)
	{
		this->field = rhs.field;
		this->indent = rhs.indent;
	}

	bool GUIFields::batch(const RectF& rect)
	{
		bool ret = false;
		FieldsList gameFields = this->list;
		for (size_t i = 0; i < gameFields.size(); ++i)
		{
			GUIValue* value = gameFields[i].field;

			float32 max_indent = std::max<float32>(gameFields[i].indent, value->info.indent);
			ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, max_indent);
			ImGui::Indent();

			bool didUpdate = value->batch(rect);
			
			ImGui::Unindent();
			ImGui::PopStyleVar(1);

			ret |= didUpdate;
		}
		return ret;
	}

	void GUIFields::add(GUIValue* value, float32 indent)
	{
		GUIFieldEntry entry;
		entry.field = value;
		entry.indent = indent;
		this->list.push_back(entry);
	}

	void GUIFields::clear()
	{
		for (auto it : this->list)
		{
			delete it.field;
		}
		this->list.clear();
	}
}