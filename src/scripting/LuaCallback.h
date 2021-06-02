#pragma once

#include "global/Callback.h"
#include "os/LogManager.h"
#include "scripting/LuaState.h"

#include <luabind/luabind.hpp>

namespace cs
{

	class LuaCallback : public Callback
	{
	public:
		LuaCallback()
			: func()
		{ }

		LuaCallback(const luabind::object& obj)
			: func(obj) 
		{ }

		LuaCallback(const luabind::object& obj, const luabind::object& arg0)
			: func(obj)
		{ 
			args.push_back(arg0);
		}

		LuaCallback(const luabind::object& obj, const luabind::object& arg0, const luabind::object& arg1)
			: func(obj)
		{ 
			args.push_back(arg0);
			args.push_back(arg1);
		}

		LuaCallback(const luabind::object& obj, const luabind::object& arg0, const luabind::object& arg1, const luabind::object& arg2)
			: func(obj)
		{ 
			args.push_back(arg0);
			args.push_back(arg1);
			args.push_back(arg2);
		}

		LuaCallback(const luabind::object& obj, const luabind::object& arg0, const luabind::object& arg1, const luabind::object& arg2, const luabind::object& arg3)
			: func(obj)
		{ 
			args.push_back(arg0);
			args.push_back(arg1);
			args.push_back(arg2);
			args.push_back(arg3);
		}

		LuaCallback(const luabind::object& obj, const luabind::object& arg0, const luabind::object& arg1, const luabind::object& arg2, const luabind::object& arg3, const luabind::object& arg4)
			: func(obj)
		{ 
			args.push_back(arg0);
			args.push_back(arg1);
			args.push_back(arg2);
			args.push_back(arg3);
			args.push_back(arg4);
		}

		LuaCallback(const LuaCallback& rhs)
			: func(rhs.func) { }

		void operator=(const LuaCallback& rhs)
		{
			this->func = rhs.func;
		}

		template <typename Ret, typename ...Args>
		Ret call(Args...args)
		{
			Ret ret;
			try
			{
				ret = luabind::call_function<Ret>(func, args...);
			}
			catch (luabind::error& err)
			{
				checkLuaError(err.state(), -1);
			}
			return ret;
		}

		template <typename ...Args>
		void operator()(Args...vargs)
		{
			int32 lua_type = luabind::type(this->func);
			if (lua_type != LUA_TFUNCTION)
			{
				log::print(LogError, "invalid lua object - expected function, got ", kLuaTypeStr[lua_type]);
				return;
			}

            try
			{
				switch (this->args.size())
				{
					case 0:
						luabind::call_function<void>(func, vargs...);
						break;
					case 1:
						luabind::call_function<void>(func, this->args[0], vargs...);
						break;
					case 2:
						luabind::call_function<void>(func, this->args[0], this->args[1], vargs...);
						break;
					case 3:
						luabind::call_function<void>(func, this->args[0], this->args[1], this->args[2], vargs...);
						break;
					case 4:
						luabind::call_function<void>(func, this->args[0], this->args[1], this->args[2], this->args[3], vargs...);
						break;
					case 5:
						luabind::call_function<void>(func, this->args[0], this->args[1], this->args[2], this->args[3], this->args[4], vargs...);
						break;
				}
			}
			catch (luabind::error& err)
			{
				checkLuaError(err.state(), -1);
			}
		}

		virtual void invoke(CallbackRetBase* ret_base = nullptr)
		{
			int32 lua_type = luabind::type(this->func);
			if (lua_type != LUA_TFUNCTION)
			{
				log::print(LogError, "invalid lua object - expected function, got ", kLuaTypeStr[lua_type]);
				return;
			}

			try
			{
				switch (args.size())
				{
					case 0:
						luabind::call_function<void>(func);
						break;
					case 1:
						luabind::call_function<void>(func, args[0]);
						break;
					case 2:
						luabind::call_function<void>(func, args[0], args[1]);
						break;
					case 3:
						luabind::call_function<void>(func, args[0], args[1], args[2]);
						break;
					case 4:
						luabind::call_function<void>(func, args[0], args[1], args[2], args[3]);
						break;
					case 5:
						luabind::call_function<void>(func, args[0], args[1], args[2], args[3], args[4]);
						break;
				}
			}
			catch (luabind::error& err)
			{
				checkLuaError(err.state(), -1);
			}
		}
		
		luabind::object func;
		std::vector<luabind::object> args;
	};

	typedef std::shared_ptr<LuaCallback> LuaCallbackPtr;

}
