#pragma once


#include "ClassDef.h"
#include "ui/UIElement.h"

#include <map>
#include <typeindex>

namespace cs
{

	typedef std::function<void(UIElement*)> UIElementInitFunc;

	CLASS_DEFINITION_DERIVED(UIWidget, UIElement)
	public:

		typedef std::function<void()> OnValueChangedFunc;
		typedef std::vector<OnValueChangedFunc> OnValueChangedArray;

		UIWidget(const std::string& name) : UIElement(name) { }

		virtual void init() { }

		void registerInitCallback(uint32 index, UIElementInitFunc& func);
		void invokeInitCallback(uint32 index, UIElement* ptr);

		void addOnChangedCallback(const OnValueChangedFunc& func) { this->onChanged.push_back(func); }
		void addOnChangedCallback(const OnValueChangedArray& arr) { this->onChanged.insert(this->onChanged.end(), arr.begin(), arr.end()); }
		void signalOnChanged() { for (auto& it : this->onChanged) it(); }
		const OnValueChangedArray& getOnChangedCallbacks() const { return this->onChanged; }

	protected:

		virtual void addChildImpl(UIElementPtr& ptr);
		virtual void removeChildImpl(UIElementPtr& ptr);

		typedef std::map<uint32, UIElementInitFunc> UIElementInitFuncCallbacks;
		UIElementInitFuncCallbacks callbacks;

		OnValueChangedArray onChanged;
		
	};
}