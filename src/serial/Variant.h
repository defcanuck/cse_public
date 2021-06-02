#pragma once

#include "serial/VariantBase.h"
#include "serial/MetaData.h"
#include "serial/MetaMacro.h"

namespace cs
{
	class MetaData;

	class Variant : public VariantBase
	{
	public:
		
		template <typename T>
		Variant(const T& value);

		Variant(const MetaData *m, void *d);
		Variant();

		Variant& operator=(const Variant& rhs);
		
		template <typename T>
		Variant& operator=(const T& rhs);
        
        void copyInternal(void* value);
	};

	template <typename T>
    Variant::Variant(const T& value)
        : VariantBase(META_TYPE(T), NULL)
	{
		this->copyInternal(&value);
	}

	template <typename T>
	Variant& Variant::operator=(const T& rhs)
	{
		// We require a new copy if meta does not match!
		if (meta != META_TYPE(T))
		{
			assert(META_TYPE(T)); // Cannot create instance of NULL meta!

			meta->erase(data);
			meta = META_TYPE(T);
			data = meta->newCopy(&rhs);
		}
		else
		{
			meta->copy(data, &rhs);
		}
		return *this;
	}
}
