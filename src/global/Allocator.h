#pragma once

#include <memory>

namespace cs
{
	
	namespace AllocatorInternal
	{
		extern const size_t kDefaultAlignment;

		void* allocate(size_t num_bytes, size_t align = kDefaultAlignment);
		void deallocate(void* ptr, size_t num_bytes);
	}

	template <typename T>
	struct Allocator
	{
		typedef std::ptrdiff_t difference_type;

		template<typename U>
		struct rebind { typedef Allocator<U> other; };

		Allocator() throw() {};
		Allocator(const Allocator& other) throw() {};

		template<typename U>
		Allocator(const Allocator<U>& other) throw() {};

		template<typename U>
		Allocator& operator = (const Allocator<U>& other) { return *this; }
		Allocator<T>& operator = (const Allocator& other) { return *this; }
		~Allocator() {}

		T* allocate(size_t n, const void* hint = 0)
		{
			return static_cast<T*>(AllocatorInternal::allocate(n * sizeof(T)));
		}

		void deallocate(T* ptr, size_t n)
		{
			AllocatorInternal::deallocate((void*)ptr, n);
		}

		void destroy(T* ptr)
		{
			ptr->~T();
		}

	};

	template <typename T, typename U>
	inline bool operator == (const Allocator<T>&, const Allocator<U>&)
	{
		return true;
	}

	template <typename T, typename U>
	inline bool operator != (const Allocator<T>& a, const Allocator<U>& b)
	{
		return !(a == b);
	}
}