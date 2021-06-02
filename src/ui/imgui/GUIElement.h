#pragma once

#include "ui/UIElement.h"
#include "ui/imgui/GUIValue.h"
#include "ui/imgui/GUIFields.h"
#include "math/Rect.h"

namespace cs
{

	CLASS_DEFINITION_DERIVED(GUIElement, UIElement)
	public:
		GUIElement(const std::string& n, CallbackPtr closeCallback = nullptr)
			: UIElement(n)
			, collapsed(false)
			, onCloseCallback(closeCallback)
			, tag("")
		{

		}

		virtual ~GUIElement()
		{
			this->clearFields();
		}

		virtual bool prepareGUI(bool inWindow = false, size_t depth = 0);

		void addFieldPtr(GUIValue* value, float32 indent = 0.0f)
		{
			fields.add(value, indent);
		}

		const std::string& getTag() const { return this->tag; }
		void setTag(const std::string& t) { this->tag = t; }

		virtual void batch(
			BatchDrawList& display_list,
			uint32& numVertices,
			uint16& numIndices,
			const UIBatchProcessData& data,
			UIBatchProcessInfo& info)
		{
			// still need to remove batched elements 
			this->flushRemoved();
		}

		virtual void clearFields();
		virtual bool batchFields(const RectF& rect);

		GUIFields* getFields() { return &this->fields; }

		virtual bool onCursor(ClickInput input, ClickParams& params, UIClickResults& results, uint32 depth);

	protected:

		bool collapsed;
		virtual void doCloseImpl();

	private:

		CallbackPtr onCloseCallback;
		GUIFields fields;
		std::string tag;
		

	};

}