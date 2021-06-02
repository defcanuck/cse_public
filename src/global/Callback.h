#pragma once

#include <memory>
#include <functional>
#include <vector>
#include <assert.h>

namespace cs
{
	template<typename T, typename... U>
	size_t getAddress(std::function<T(U...)> f) 
	{
		typedef T(fnType)(U...);
		fnType ** fnPointer = f.template target<fnType*>();
		if (fnPointer)
			return (size_t)*fnPointer;
		return 0;
	}

	class CallbackRetBase { };
	
	template <class R0>
	class CallbackRetTyped : public CallbackRetBase
	{
	public:
		void call(std::function<R0()>& func)
		{
			this->retval = func();
		}

		template <class T0>
		void call(std::function<R0(T0)>& func, T0& arg0)
		{
			this->retval = func(arg0);
		}

		R0 retval;
	};

	template <>
	class CallbackRetTyped<void>
	{
	public:
		void call(std::function<void()>& func) { assert(false); }
		
		template <class T0>
		void call(std::function<void(T0)>& func, T0& arg0)
		{
			func(arg0);
		}
	};

	class Callback
	{
	public:
		Callback() { }

		virtual void invoke(CallbackRetBase* ret_base = nullptr) { assert(false); }
		
		template <class R0>
		R0 invoke()
		{
			CallbackRetTyped<R0> typed_ret_value;
			this->invoke((CallbackRetBase*) &typed_ret_value);
			return typed_ret_value.retval;
		}

		virtual size_t getSignature() const { return 0; }
		
		bool operator==(const Callback& callback) const
		{
			return this->getSignature() == callback.getSignature();
		}

	private:

	};

	typedef std::shared_ptr<Callback> CallbackPtr;
	typedef std::vector<CallbackPtr> CallbackList;

	template <class R0>
	class CallbackArg0 : public Callback
	{
	public:
		typedef R0(*callbackFunc)();
		typedef std::function<R0()> CallbackFunc;

		CallbackArg0() 
			: func() { }
		CallbackArg0(const CallbackArg0& rhs)
			: func(rhs.func) { }
		CallbackArg0(CallbackFunc fn)
			: func(fn) { }
		CallbackArg0(callbackFunc fn)
			: func(std::bind(fn)) { }

		void operator=(const CallbackArg0& rhs)
		{
			this->func = rhs.func;
		}

		virtual void invoke(CallbackRetBase* ret_base = nullptr)
		{
			if (ret_base)
			{
				CallbackRetTyped<R0>* callback_ret_typed = reinterpret_cast<CallbackRetTyped<R0>*>(ret_base);
				callback_ret_typed->call(this->func);
				return;
			}
			if (this->func)
				this->func();
		}

		void operator()()
		{
			this->invoke();
		}

		virtual size_t getSignature() const
		{
			return getAddress(this->func);
		}

	private:
		CallbackFunc func;
	};

	template <class R0, class T0>
	class CallbackArg1 : public Callback
	{
	public:
		typedef R0(*callbackFunc)(T0);
		typedef std::function<R0(T0)> CallbackFunc;

		CallbackArg1() 
			: func()
			, argument0(T0()) { }
		CallbackArg1(CallbackFunc& fn) 
			: func(fn)
			, argument0(T0()) { }
		CallbackArg1(callbackFunc fn)
			: func(std::bind(fn, std::placeholders::_1))
			, argument0(T0()) { }
		CallbackArg1(const CallbackArg1<R0, T0>& rhs)
			: func(rhs.func)
			, argument0(rhs.argument0) { }
		CallbackArg1(CallbackFunc& fn, const T0& arg0)
			: func(fn)
			, argument0(arg0) { }
		CallbackArg1(callbackFunc fn, const T0& arg0)
			: func(std::bind(fn, std::placeholders::_1))
			, argument0(arg0) { }

		void operator=(const CallbackArg1& rhs)
		{
			this->func = rhs.func;
			this->argument0 = rhs.argument0;
		}

		virtual void invoke(CallbackRetBase* ret_base = nullptr)
		{
			if (this->func)
				this->func(this->argument0);
		}

		void invoke(const T0& arg0)
		{
			if (this->func)
				this->func(arg0);
		}

		void operator()(const T0& arg0)
		{
			this->invoke(arg0);
		}

		virtual size_t getSignature() const
		{ 
			return getAddress(this->func);
		}

	private:

		CallbackFunc func;
		T0 argument0;
	};

	template <class R0, class T0, class T1>
	class CallbackArg2 : public Callback
	{
	public:
		typedef R0(*callbackFunc)(T0, T1);
		typedef std::function<R0(T0, T1)> CallbackFunc;

		CallbackArg2() 
			: func() { }
		CallbackArg2(CallbackFunc& fn) 
			: func(fn) { }
		CallbackArg2(callbackFunc fn)
			: func(std::bind(fn, std::placeholders::_1, std::placeholders::_2)) { }
		CallbackArg2(CallbackFunc& fn, const T0& arg0, const T1& arg1)
			: func(fn)
			, argument0(arg0)
			, argument1(arg1) { }
		CallbackArg2(callbackFunc fn, const T0& arg0, const T1& arg1)
			: func(std::bind(fn, std::placeholders::_1, std::placeholders::_2))
			, argument0(arg0)
			, argument1(arg1) { }

		void operator=(const CallbackArg2& rhs)
		{
			this->func = rhs.func;
			this->argument0 = rhs.argument0;
			this->argument1 = rhs.argument1;
		}

		virtual void invoke(CallbackRetBase* ret_base = nullptr)
		{
			if (this->func)
				this->func(this->argument0, this->argument1);
		}

		void invoke(const T0& arg0, const T1& arg1)
		{
			if (this->func)
				this->func(arg0, arg1);
		}

		void operator()(const T0& arg0, const T1& arg1)
		{
			this->invoke(arg0, arg1);
		}

		virtual size_t getSignature() const
		{
			return getAddress(this->func);
		}

	private:

		CallbackFunc func;
		T0 argument0;
		T1 argument1;
	};

	template <class R0>
	CallbackPtr createCallbackArg0(R0(*func_ptr)())
	{
		typename CallbackArg0<R0>::CallbackFunc func = std::bind(func_ptr);
		std::shared_ptr<CallbackArg0<R0>> callback =
			std::make_shared<CallbackArg0<R0>>(func);

		return callback;
	}

	template <class R0, class Caller>
	CallbackPtr createCallbackArg0(R0(Caller::*func_ptr)(), Caller* caller_ptr)
	{
		typename CallbackArg0<R0>::CallbackFunc func = std::bind(func_ptr, caller_ptr);
		std::shared_ptr<CallbackArg0<R0>> callback =
			std::make_shared<CallbackArg0<R0>>(func);

		return callback;
	}

	template <class R0, class T0, typename... Args>
	CallbackPtr createCallbackArg1(R0(*func_ptr)(T0), Args... args)
	{
		typename CallbackArg1<R0, T0>::CallbackFunc func = std::bind(func_ptr, std::placeholders::_1);
		std::shared_ptr<CallbackArg1<R0, T0>> callback =
			std::make_shared<CallbackArg1<R0, T0>>(func, args...);

		return callback;
	}

	template <class R0, class T0, class Caller, typename... Args>
	CallbackPtr createCallbackArg1(R0(Caller::*func_ptr)(T0), Caller* caller_ptr, Args... args)
	{
		typename CallbackArg1<R0, T0>::CallbackFunc func = std::bind(func_ptr, caller_ptr, std::placeholders::_1);
		std::shared_ptr<CallbackArg1<R0, T0>> callback =
			std::make_shared<CallbackArg1<R0, T0>>(func, args...);

		return callback;
	}

	template <class R0, class T0, class T1, typename... Args>
	CallbackPtr createCallbackArg2(R0(*func_ptr)(T0, T1), Args... args)
	{
		typename CallbackArg2<R0, T0, T1>::CallbackFunc func = std::bind(func_ptr, std::placeholders::_1, std::placeholders::_2);
		std::shared_ptr<CallbackArg2<R0, T0, T1>> callback =
			std::make_shared<CallbackArg2<R0, T0, T1>>(func, args...);

		return callback;
	}

	template <class R0, class T0, class T1, class Caller, typename... Args>
	CallbackPtr createCallbackArg2(R0(Caller::*func_ptr)(T0, T1), Caller* caller_ptr, Args... args)
	{
		typename CallbackArg2<R0, T0, T1>::CallbackFunc func = std::bind(func_ptr, caller_ptr, std::placeholders::_1, std::placeholders::_2);
		std::shared_ptr<CallbackArg2<R0, T0, T1>> callback =
			std::make_shared<CallbackArg2<R0, T0, T1>>(func, args...);

		return callback;
	}

}
