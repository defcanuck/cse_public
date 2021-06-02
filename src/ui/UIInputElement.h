#pragma once

#include "ui/UITextElement.h"
#include "os/InputField.h"

namespace cs
{

	CLASS_DEFINITION_DERIVED(UIInputElement, UITextElement)
	public:

		typedef std::function<void()> OnInputEndFunc;

		UIInputElement(const std::string& name, InputFilterPtr& ptr);

		virtual ~UIInputElement();

		virtual void batch(
			BatchDrawList& display_list,
			uint32& numVertices,
			uint16& numIndices,
			const UIBatchProcessData& data,
			UIBatchProcessInfo& info);

		void beginInput();
		void endInput(const std::string& str);
		void setHighlightColor(const ColorB& color) { this->highlightColor = color; }

		void onInput(InputTextParams& params);
		virtual ColorB getColorByIndex(int32 i);

		void setOnInputEndCallback(OnInputEndFunc& func) { this->onEndInput = func; }

	protected:

		ColorB highlightColor;
		InputFieldPtr inputField;
		InputFilterPtr inputFilter;

		OnInputEndFunc onEndInput;
	};
}