#pragma once

#include "ecs/comp/Component.h"

#include "scripting/LuaScriptHandle.h"
#include "ecs/Entity.h"
#include "global/PropertySet.h"

#include <luabind/object.hpp>

namespace cs
{
	enum ScriptInstanceState
	{
		ScriptInstanceStateNone = -1,
		ScriptInstanceStateRetry,
		ScriptInstanceStateLoaded,
		ScriptInstanceStateError,
		ScriptInstanceStateMAX
	};

	template <class T>
	struct LuaObjectPtr
	{
		LuaObjectPtr()
		{
			this->luaObject = luabind::object();
			this->ptr = nullptr;
		}

		LuaObjectPtr(luabind::object& obj)
		{
			this->luaObject = obj;
			this->ptr = luabind::object_cast<std::shared_ptr<T>>(obj);
		}

		LuaObjectPtr(const LuaObjectPtr& rhs)
			: ptr(rhs.ptr)
			, luaObject(rhs.luaObject) { }

		void operator=(const LuaObjectPtr& rhs)
		{
			this->luaObject = rhs.luaObject;
			this->ptr = rhs.ptr;
		}

		std::shared_ptr<T>& get() { return this->ptr; }
		T* operator->() { return this->ptr.get(); }
		bool exists() const { return this->ptr.get() != nullptr; }

		std::shared_ptr<T> ptr;
		luabind::object luaObject;
	};

	CLASS_DEFINITION(ScriptComponentInstance)
	public:
		ScriptComponentInstance(Entity* e) : entity(e) { }
		virtual ~ScriptComponentInstance() { }

		virtual ScriptInstanceState populate(PropertySet& propertySet) { return ScriptInstanceStateLoaded; }
		virtual void process(float32 dt) { }
		virtual void onPress(const vec2& screen_pos)   { log::info("ScriptComponentInstance::onPress"); }
		virtual void onRelease(const vec2& screen_pos) { log::info("ScriptComponentInstance::onRelease"); }

		Entity* getEntity() { return entity; }

	private:

		Entity* entity;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(ScriptComponent, Component)
	public:
		ScriptComponent();
		virtual ~ScriptComponent() { }

		bool load(LuaStatePtr& state);

		void onPress(const vec2& screen_pos);
		void onRelease(const vec2& screen_pos);

		virtual void reset();
		virtual void process(float32 dt);
		virtual void onPostLoad(const LoadFlagMask& flags = kLoadFlagMaskAll);

		void onScriptChanged();
		void onPropertyChanged();

		void setLuaState(LuaStatePtr& state) { this->luaState = state; }

		ScriptComponentInstancePtr& getScriptComponentInstance() { return this->instance.get(); }

		void resetScript();

		PropertySetHandlePtr& getPropertySet() { return this->propertySetHandle; }
		void setPropertySet(PropertySetHandlePtr& ptr) { this->propertySetHandle = ptr; }

		void setActivePropertySet(PropertySetPtr& ptr);
		PropertySetPtr getActivePropertySet();

		const bool canOverrideProperties() const { return this->overrideProperties; }

	private:

		void refreshScript();
		void createInstance();
		void initInstance();
		void populateInstance();

		LuaStatePtr luaState;
		LuaScriptHandlePtr scriptHandle;

		PropertySetHandlePtr propertySetHandle;
		PropertySetPtr properties;
		bool overrideProperties;

		bool scriptLoaded;
		ScriptInstanceState instanceState;
		LuaObjectPtr<ScriptComponentInstance> instance;
	};
}