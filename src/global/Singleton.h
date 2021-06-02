#pragma once

namespace cs
{
	template <class C>
	class Singleton
	{
	public:
		static C* getInstance()
		{
			if (!instance)
				instance = new C;
			return instance;
		}

	protected:

		static C* instance;

		Singleton() { }
		Singleton(Singleton const &) = delete;
		Singleton& operator = (Singleton const &);
	};

	template <class C>
	C* Singleton<C>::instance = nullptr;

}