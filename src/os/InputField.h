#pragma once

#include "ClassDef.h"
#include "math/Rect.h"

#include "os/InputFilter.h"

namespace cs
{

	struct InputTextParams
	{
		std::string text;
		size_t cursorPos;
		size_t sel_begin;
		size_t sel_end;
	};

	CLASS_DEFINITION(InputField)
	public:

		typedef std::function<void(const std::string&)> OnInputEndedFunc;
		typedef std::function<void(InputTextParams&)> OnInputTextFunc;

		InputField(const RectF& rect, InputFilterPtr ptr = nullptr) :
			screenRect(rect),
			isActive(false),
			onInputEnd(),
			onInputText(),
			inputFilter(ptr) { }

		void setOnInputEnded(OnInputEndedFunc& func) { this->onInputEnd = func; }
		void setOnInputText(OnInputTextFunc& func) { this->onInputText = func; }

		void beginInput(std::string& str);
		void endInput(const std::string& str);
		void onInput(InputTextParams& params);
		bool getIsActive() const { return this->isActive; }

	protected:

		virtual void beginInputImpl() = 0;
		virtual void endInputImpl() = 0;
		
		bool isActive;
		RectF screenRect;
		OnInputEndedFunc onInputEnd;
		OnInputTextFunc onInputText;

		InputFilterPtr inputFilter;

	};
}