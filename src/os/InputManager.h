#pragma once

#include "global/Values.h"
#include "math/GLM.h"
#include "global/Singleton.h"

#include "os/Input.h"
#include "os/InputField.h"

#include "math/Rect.h"

#include <functional>
#include <unordered_map>

namespace cs
{

	enum InputModifier
	{
		InputModifierShift,
		InputModiferControl,
		InputModifierAlt,
		InputModiferMAX
	};

	const int kModifierShift	= 0x1 << InputModifierShift;
	const int kModifierControl	= 0x1 << InputModiferControl;
	const int kModifierAlt		= 0x1 << InputModifierAlt;

	class InputManager : public Singleton<InputManager>
	{
	public:

		struct ClickListenerInfo
		{
			int32 priority;
			std::string tag;
		};

		struct KeyListenerInfo
		{
			int32 priority;
			std::string tag;
		};

		typedef std::map<uintptr_t*, InputFieldPtr> InputFieldMap;
		typedef std::function<bool(ClickInput, ClickParams&)> InputClickListener;
		
		typedef std::function<bool(uint32, uint32, bool)> InputKeyListener;
		typedef std::pair<InputKeyListener, KeyListenerInfo> InputKeyListenerPriority;

		typedef std::function<void(char*)> InputTextListener;
		typedef std::pair<InputClickListener, ClickListenerInfo> InputClickListenerPriority;

		InputManager();
		~InputManager();

		

		template <class T>
		void addClickListener(void* ptr, bool(T::*func)(ClickInput, ClickParams&), int32 priority = 0, const std::string& tag = "")
		{
			assert(sizeof(void*) == sizeof(uintptr_t));
			uintptr_t key = reinterpret_cast<uintptr_t>(ptr);
			InputClickListenerPriority listener;
			listener.first = std::bind(func, (T*)ptr, std::placeholders::_1, std::placeholders::_2);
			listener.second.priority = priority;
			listener.second.tag = tag;
			this->clickListeners[key] = listener;
			this->updateClickPriorityList();
		}

		template <class T>
		void removeClickListener(void* ptr, bool(T::*func)(ClickInput, ClickParams&))
		{
			assert(sizeof(void*) == sizeof(uintptr_t));
			uintptr_t key = reinterpret_cast<uintptr_t>(ptr);
			InputClickListeners::iterator it = this->clickListeners.find(key);
			if (it != this->clickListeners.end())
				this->clickListeners.erase(it);
			this->updateClickPriorityList();
		}

		template <class T>
		void addKeyListener(void* ptr, bool(T::*func)(uint32, uint32, bool), int32 priority = 0, const std::string& tag = "")
		{
			assert(sizeof(void*) == sizeof(uintptr_t));
			uintptr_t key = reinterpret_cast<uintptr_t>(ptr);
			InputKeyListenerPriority listener;
			listener.first = std::bind(func, (T*)ptr, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			listener.second.priority = priority;
			listener.second.tag = tag;
			this->keyListeners[key] = listener;
			this->updateKeyPriorityList();
		}

		template <class T>
		void removeKeyListener(void* ptr, bool(T::*func)(uint32, uint32, bool))
		{
			assert(sizeof(void*) == sizeof(uintptr_t));
			uintptr_t key = reinterpret_cast<uintptr_t>(ptr);
			InputKeyListeners::iterator it = this->keyListeners.find(key);
			if (it != this->keyListeners.end())
				this->keyListeners.erase(it);
			this->updateKeyPriorityList();
		}

		template <class T>
		void addTextListener(void* ptr, void(T::*func)(char*))
		{
			assert(sizeof(void*) == sizeof(uintptr_t));
			size_t key = reinterpret_cast<size_t>(ptr);
			this->textListeners[key] = std::bind(func, (T*)ptr, std::placeholders::_1);
		}

		void onInputDown(ClickInput input, const vec2& position);
		void onInputUp(ClickInput input, bool forceRelease = false);
		void onInputMove(ClickInput input, const vec2& position);
		void onInputZoom(ClickInput input, float32 zoom);

		void onKeyDown(uint32 key, uint32 flags);
		void onKeyUp(uint32 key, uint32 flags);

		ClickParams& getClickState(ClickInput input);

		void setEnabled(bool e) { this->enabled = e; }

		InputFieldPtr createInputField(uintptr_t* ptr, const RectF& rect, InputFilterPtr& inputFilter);
		void eraseInputField(uintptr_t* ptr);

		bool isEditingText() const { return this->activeInput != nullptr; }

		void startEditingText(uintptr_t* ptr)
		{
			uintptr_t* uptr = static_cast<uintptr_t*>(ptr);
			InputFieldMap::iterator it;
			if ((it = this->inputFields.find(uptr)) != this->inputFields.end())
			{
			
				this->endEditingText();

				this->activeInput = it->second;
				this->activeInput->beginInput(this->textInput);
				this->cursor = this->textInput.length();
			}
		}

		void onBackspace()
		{
			if (!this->isEditingText())
				return;

			if (this->cursor == 0)
				return;
			if (this->cursor == this->textInput.length())
			{
				this->textInput = this->textInput.substr(0, this->textInput.length() - 1);
				this->cursor = this->textInput.length();
			}

			InputTextParams params;
			params.text = this->textInput;
			params.cursorPos = this->cursor;

			this->activeInput->onInput(params);

		}

		void onTextEdit(char* curInput)
		{
			if (!this->isEditingText())
			{
				for (auto& it : this->textListeners)
					it.second(curInput);
				return;
			}

			InputTextParams params;
			params.text = this->textInput + curInput;
			
			this->activeInput->onInput(params);
			this->textInput = params.text;

		}

		void endEditingText()
		{
			if (!this->activeInput)
				return;

			this->activeInput->endInput(this->textInput);
			this->activeInput = nullptr;
		}

	private:

		bool enabled;
		void triggerNotification(ClickInput input);

		void updateKeyPriorityList();
		void updateClickPriorityList();

		typedef std::map<ClickInput, ClickParams> InputState;
		InputState states;

		
		typedef std::map<size_t, InputClickListenerPriority> InputClickListeners;
		typedef std::vector<InputClickListenerPriority> InputClickListenerList;

		InputClickListeners clickListeners;
		InputClickListenerList clickPriorityList;

		typedef std::map<size_t, InputKeyListenerPriority> InputKeyListeners;
		typedef std::vector<InputKeyListenerPriority> InputKeyListenerList;

		InputKeyListeners keyListeners;
		InputKeyListenerList keyPriorityList;

		typedef std::map<size_t, InputTextListener> InputTextListeners;
		InputTextListeners textListeners;

		InputFieldMap inputFields;
		InputFieldPtr activeInput;
		
		std::string textInput;
		size_t cursor;
		bool isButtonPressed;
	};
}
