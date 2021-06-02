#include "PCH.h"

#include "os/InputField.h"

namespace cs
{
	void InputField::beginInput(std::string& str)
	{
		this->beginInputImpl();
		this->isActive = true;

		if (this->inputFilter)
		{
			str = this->inputFilter->get();
		}
	}

	void InputField::endInput(const std::string& str)
	{
		this->endInputImpl();

		this->isActive = false;
		if (this->onInputEnd)
			this->onInputEnd(str);
	}

	void InputField::onInput(InputTextParams& params)
	{
		// filter
		std::string newStr = this->inputFilter->filter(params.text);

		// Update the text
		params.text = newStr;
		if (this->onInputText)
			this->onInputText(params);
		
	}
}