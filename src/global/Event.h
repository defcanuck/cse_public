#pragma once

#include "global/Callback.h"
#include "scripting/LuaCallback.h"

namespace cs
{
	class Event
	{
	public:

		Event() { }
		
		Event(const Event& rhs)
			: callbacks(rhs.callbacks)
			, keyCallbacks(rhs.keyCallbacks)
		{ }

		Event(CallbackPtr ptr)
		{
			this->callbacks.push_back(ptr);
		}

		Event(std::initializer_list<CallbackPtr> ptrs)
			: callbacks(ptrs) 
		{ }
		
		Event(CallbackList& calls)
			: callbacks(calls) 
		{ }

		void clear()
		{
			this->callbacks.clear();
		}

		void addCallback(CallbackPtr ptr)
		{
			this->callbacks.push_back(ptr);
		}

		void addLuaCallback(LuaCallbackPtr ptr)
		{
			this->callbacks.push_back(std::static_pointer_cast<Callback>(ptr));
		}

		void addCallback(const luabind::object& object)
		{
			std::shared_ptr<LuaCallback> ptr = std::make_shared<LuaCallback>(object);
			callbacks.push_back(ptr);
		}

		void addCallback(const luabind::object& object, const luabind::object& arg0)
		{
			std::shared_ptr<LuaCallback> ptr = std::make_shared<LuaCallback>(object, arg0);
			callbacks.push_back(ptr);
		}

		void addCallback(const luabind::object& object, const luabind::object& arg0, const luabind::object& arg1)
		{
			std::shared_ptr<LuaCallback> ptr = std::make_shared<LuaCallback>(object, arg0, arg1);
			callbacks.push_back(ptr);
		}

		void addCallback(const luabind::object& object, const luabind::object& arg0, const luabind::object& arg1, const luabind::object& arg2)
		{
			std::shared_ptr<LuaCallback> ptr = std::make_shared<LuaCallback>(object, arg0, arg1, arg2);
			callbacks.push_back(ptr);
		}

		virtual void invoke()
		{
			CallbackList callbacksToInvoke = this->callbacks;
			for (auto& it : callbacksToInvoke)
				it->invoke();

			KeyCallbacks keyCallbacksToInvoke = this->keyCallbacks;
			for (auto& it : keyCallbacksToInvoke)
				it.second->invoke();
		}

		void operator+=(CallbackPtr callback)
		{
			callbacks.push_back(callback);
		}

		bool keyCallbackExists(void* func, void* this_ptr = nullptr)
		{
			KeyFunctionSignature signature(reinterpret_cast<size_t>(func), reinterpret_cast<size_t>(this_ptr));
			return this->keyCallbacks.find(signature) != this->keyCallbacks.end();
		}

		void addKeyCallback(CallbackPtr ptr, void* func, void* this_ptr = nullptr)
		{
			KeyFunctionSignature signature(reinterpret_cast<size_t>(func), reinterpret_cast<size_t>(this_ptr));
			if (this->keyCallbacks.find(signature) != this->keyCallbacks.end())
			{
				log::info("Duplicate key callback already found!");
				return;
			}

			this->keyCallbacks[signature] = ptr;
		}

		void removeKeyCallback(void* func, void* this_ptr = nullptr)
		{
			KeyFunctionSignature signature(reinterpret_cast<size_t>(func), reinterpret_cast<size_t>(this_ptr));
			KeyCallbacks::iterator it = this->keyCallbacks.find(signature);
			if (it != this->keyCallbacks.end())
			{
				this->keyCallbacks.erase(it);
				return;
			}
		}

		bool exists(CallbackPtr& callback)
		{
			for (auto& it : this->callbacks)
			{
				if (callback.get() == it.get() || callback->getSignature() == it->getSignature())
				{
					return true;
				}
			}
			return false;
		}

		void operator+=(CallbackList& callbacks)
		{
			this->callbacks.insert(this->callbacks.end(), callbacks.begin(), callbacks.end());
		}

	private:

		CallbackList callbacks;

		typedef std::pair<size_t, size_t> KeyFunctionSignature;
		typedef std::map<KeyFunctionSignature, CallbackPtr> KeyCallbacks;
		KeyCallbacks keyCallbacks;
	};
}
