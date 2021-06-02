#include "PCH.h"

#include "ui/UIInputElement.h"

#include "os/InputManager.h"
#include "ui/behavior/UIClickBehavior.h"

namespace cs
{
	UIInputElement::UIInputElement(const std::string& name, InputFilterPtr& ptr) :
		UITextElement(name),
		inputField(nullptr),
		highlightColor(ColorB::White),
		inputFilter(ptr),
		onEndInput()
	{ 
		if (this->inputFilter)
		{
			this->setText(this->inputFilter->get());
		}
	}

	UIInputElement::~UIInputElement()
	{
		if (this->inputField)
		{
			InputManager::getInstance()->eraseInputField((uintptr_t*) this);
			this->inputField = nullptr;
		}
	}

	void UIInputElement::batch(
		BatchDrawList& display_list,
		uint32& numVertices,
		uint16& numIndices,
		const UIBatchProcessData& data,
		UIBatchProcessInfo& info)
	{
		BASECLASS::batch(display_list, numVertices, numIndices, data, info);

		if (!this->inputField)
		{
			this->inputField = InputManager::getInstance()->createInputField((uintptr_t*) this, this->screen_rect, this->inputFilter);

			CallbackArg0<void>::CallbackFunc func = std::bind(&UIInputElement::beginInput, this);
			UIClickBehaviorPtr click = CREATE_CLASS(cs::UIClickBehavior, func);
			this->addBehavior(click);
		}
	}

	ColorB UIInputElement::getColorByIndex(int32 i)
	{
		if (this->inputField && this->inputField->getIsActive())
		{
			return this->highlightColor;
		}
		int32 idx = std::min(i, kNumVertexColors - 1);
		return this->vertexColor[idx];
	}

	void UIInputElement::beginInput()
	{
		// Set input on end
		InputField::OnInputEndedFunc funcEnd = std::bind(&UIInputElement::endInput, this, std::placeholders::_1);
		this->inputField->setOnInputEnded(funcEnd);
		
		InputField::OnInputTextFunc funcText = std::bind(&UIInputElement::onInput, this, std::placeholders::_1);
		this->inputField->setOnInputText(funcText);

		InputManager::getInstance()->startEditingText((uintptr_t*) this);
	}

	void UIInputElement::endInput(const std::string& str)
	{
		if (!this->inputFilter)
			return;

		log::print(LogInfo, "Final Input:", str);
		this->inputFilter->set(str);

		if (this->onEndInput)
			this->onEndInput();
	}

	void UIInputElement::onInput(InputTextParams& params)
	{
		
		this->setText(params.text);
		
	}
}