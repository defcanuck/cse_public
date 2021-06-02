#include "PCH.h"

#include "ecs/comp/ScriptComponent.h"
#include "liquid/LiquidGroup.h"
#include "os/FileManager.h"


namespace cs
{

	BEGIN_META_CLASS(ScriptComponent)
		ADD_META_FUNCTION("Reset Script", &ScriptComponent::createInstance);
		ADD_MEMBER_PTR(scriptHandle);
			SET_MEMBER_CALLBACK_POST(&ScriptComponent::onScriptChanged);
		ADD_MEMBER(overrideProperties);
			SET_MEMBER_DEFAULT(true);
		ADD_MEMBER_PTR(propertySetHandle);
			SET_MEMBER_CALLBACK_POST(&ScriptComponent::onPropertyChanged);
		
	END_META()

	ScriptComponent::ScriptComponent()
		: luaState(nullptr)
		, scriptHandle(CREATE_CLASS(LuaScriptHandle))
		, propertySetHandle(CREATE_CLASS(PropertySetHandle))
		, properties(nullptr)
		, overrideProperties(true)
		, scriptLoaded(false)
		, instanceState(ScriptInstanceStateNone)
		, instance()
	{

	}

	void ScriptComponent::process(float32 dt)
	{
		if (!this->instance.exists())
			return;

		this->instance->process(dt);
	}

	void ScriptComponent::reset()
	{
		this->resetScript();
	}

	void ScriptComponent::populateInstance()
	{
		if (this->instanceState == ScriptInstanceStateNone || this->instanceState == ScriptInstanceStateRetry)
		{
			PropertySetPtr prop_ptr = this->getActivePropertySet();
			this->instanceState = this->instance->populate(*prop_ptr.get());
		}
	}
	
	void ScriptComponent::onScriptChanged()
	{
		this->createInstance();
	}

	void ScriptComponent::onPropertyChanged()
	{
		this->createInstance();
	}

	void ScriptComponent::setActivePropertySet(PropertySetPtr& ptr)
	{
		assert(this->canOverrideProperties());
		this->properties = ptr;
	}

	PropertySetPtr ScriptComponent::getActivePropertySet()
	{
		static PropertySetPtr kEmptyPropertySet = CREATE_CLASS(PropertySet);
		if (this->properties.get() != nullptr)
		{
			return this->properties;
		}

		if (this->propertySetHandle.get() && this->propertySetHandle->hasPropertySetResource())
		{
			PropertySetResourcePtr& resource_ptr = this->propertySetHandle->getPropertySetResource();
			assert(resource_ptr.get());
			return resource_ptr->getPropertySetPtr();
		}

		return kEmptyPropertySet;
	}

	void ScriptComponent::onPostLoad(const LoadFlagMask& flags)
	{
		if (!flags.test(LoadFlagsScript))
		{
			return;
		}

		if (this->scriptHandle)
		{
			this->scriptHandle->onChanged += createCallbackArg0(&ScriptComponent::onScriptChanged, this);
		}

		if (this->luaState && !this->scriptLoaded)
			this->createInstance();
	}

	void ScriptComponent::refreshScript()
	{
		if (!this->scriptHandle)
		{
			log::error("No script handle to create Script Instance");
			return;
		}

		if (this->scriptLoaded)
		{
			log::info("Script already loaded!");
			return;
		}

		this->scriptLoaded = this->scriptHandle->load(this->luaState);
		if (!this->scriptLoaded)
		{
			log::error("Script failed to load");
		}
	}

	void ScriptComponent::resetScript()
	{
		this->instanceState = ScriptInstanceStateNone;
		this->scriptLoaded = false;
		this->createInstance();
	}

	void ScriptComponent::createInstance()
	{
		this->refreshScript();

		if (this->scriptLoaded)
		{
			const LuaScriptFilePtr& reference = this->scriptHandle->getReference();
			std::string name = FileManager::stripExtension(reference->getName());

			luabind::object obj = this->luaState->createObject(name, this->getParent());

			if (obj.is_valid())
			{
				this->instance = LuaObjectPtr<ScriptComponentInstance>(obj);
				this->populateInstance();
			}
		}
	}

	bool ScriptComponent::load(LuaStatePtr& state)
	{
		this->luaState = state;
		this->createInstance();

		return this->scriptLoaded;
	}

	void ScriptComponent::onPress(const vec2& screen_pos)
	{
		if (!this->instance.exists())
			return;

		if (this->instanceState != ScriptInstanceStateLoaded)
		{
			log::error("Cannot interface with ", this->getParent()->getName(), " until script component is loaded");
			return;
		}

		//log::info("Use Count:", this->scriptInstance.use_count());
		this->instance->onPress(screen_pos);
	}

	void ScriptComponent::onRelease(const vec2& screen_pos)
	{
		if (!this->instance.exists())
			return;

		if (this->instanceState != ScriptInstanceStateLoaded)
		{
			log::error("Cannot interface with ", this->getParent()->getName(), " until script component is loaded");
			return;
		}

		this->instance->onRelease(screen_pos);
	}

}