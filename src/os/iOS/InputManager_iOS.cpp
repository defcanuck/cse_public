#include "PCH.h"

#include "os/InputManager.h"
#include "os/iOS/InputField_iOS.h"

namespace cs
{
	InputFieldPtr InputManager::createInputField(uintptr_t* ptr, const RectF& rect, InputFilterPtr& inputFilter)
	{
		InputFieldMap::iterator it;
		uintptr_t* uptr = static_cast<uintptr_t*>(ptr);
		if ((it = this->inputFields.find(uptr)) != this->inputFields.end())
			return it->second;

		InputField_iOSPtr inputField = CREATE_CLASS(InputField_iOS, rect, inputFilter);
		this->inputFields[uptr] = inputField;
		return inputField;
	}

	void InputManager::eraseInputField(uintptr_t* ptr)
	{
		InputFieldMap::iterator it;
		uintptr_t* uptr = static_cast<uintptr_t*>(ptr);
		if ((it = this->inputFields.find(uptr)) != this->inputFields.end())
		{
			if (this->activeInput.get() == it->second.get())
				this->activeInput = nullptr;

			this->inputFields.erase(it);
		}
	}
}
