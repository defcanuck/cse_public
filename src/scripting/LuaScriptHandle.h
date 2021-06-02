#pragma once

#include "ClassDef.h"

#include "global/Resource.h"
#include "global/Event.h"

#include "scripting/LuaScript.h"
#include "scripting/LuaScriptHandle.h"

namespace cs
{

	CLASS_DEFINITION_REFLECT(LuaScriptHandle)
	public:

		LuaScriptHandle()
			: reference(nullptr) { }

		LuaScriptHandle(LuaScriptFilePtr& rhs)
			: reference(rhs) { }

		LuaScriptHandle(const std::string& fileName);

		bool load(LuaStatePtr& state);

		bool hasReference() const { return this->reference.get() != nullptr; }
		const std::string& getName() const { return this->reference->getName(); }

		void setReference(LuaScriptFilePtr& ref) { this->reference = ref; }
		const LuaScriptFilePtr& getReference() { return this->reference; }
		
		Event onChanged;
		void onReferenceChanged();

		void clear()
		{
			this->reference = nullptr;
			this->onChanged.invoke();
		}

	private:

		LuaScriptFilePtr reference;

	};
}