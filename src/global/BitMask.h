#pragma once

#include <vector>

namespace cs
{

	template <class T, int MaxValue>
	class BitMask
	{

	public:

		typedef std::vector<T> Flags;

		BitMask() : mask(0) { }
		BitMask(const size_t& m) : mask(m) { }
		BitMask(const BitMask<T, MaxValue>& rhs) : mask(rhs.mask) { }
		BitMask(std::vector<T> flags)
		{
			this->mask = 0;
			this->set(flags);
		}

		bool all() const { return this->mask == MaxValueParam; }
		bool none() const { return this->mask == 0; }

		void operator=(const BitMask<T, MaxValue>& rhs)
		{
			this->mask = rhs.mask;
		}

		bool operator==(const BitMask<T, MaxValue>& rhs) const
		{
			return this->mask == rhs.mask;
		}

		bool operator!=(const BitMask<T, MaxValue>& rhs) const
		{
			return this->mask != rhs.mask;
		}

		inline void set(const T& value)
		{
			this->mask |= (size_t(0x1) << size_t(value));
		}

		inline void set(std::vector<T> flags)
		{
			for (auto& it : flags)
				this->set(it);
		}

		inline void unset(const T&  value)
		{
			this->mask &= ~(0x1 << size_t(value));
		}

		inline bool test(const T&  value) const
		{
			return (this->mask & (size_t(0x1) << size_t(value))) > 0;
		}

		inline void setAll()
		{
			mask = MaxValueParam;
		}

		inline void toggle(const T& value)
		{
			if (this->test(value))
				this->unset(value);
			else
				this->set(value);
		}

		inline void clear()
		{
			this->mask = 0;
		}

		static size_t gen(std::vector<T> flags)
		{
			size_t mask_value = 0;
			for (auto& it : flags)
				mask_value |= (size_t(0x1) << size_t(it));
			return mask_value;
		}

		enum { MaxValueParam = ((0x1 << MaxValue) - 1) };
		size_t mask;
	};

	template <class T, int MaxValue >
	std::ostream& operator<<(std::ostream& os, const BitMask<T, MaxValue>& rhs)
	{
		os << rhs.mask;
		return os;
	}
}
