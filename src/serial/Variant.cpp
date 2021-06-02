#include "PCH.h"

#include "serial/Variant.h"
#include "serial/MetaData.h"

#include <cassert>

namespace cs
{
	Variant::Variant(const MetaData *m, void *d) : 
		VariantBase(m, d)
	{
	}

	Variant::Variant() : 
		VariantBase(nullptr, nullptr)
	{
	}

	Variant& Variant::operator=(const Variant& rhs)
	{
		if (this == &rhs)
			return *this;

		if (meta)
		{
			if (meta == rhs.meta)
				meta->copyData(data, rhs.data);
			else
			{
				assert(rhs.meta); // Cannot make an instance of NULL meta!

				meta->erase(data);
				meta = rhs.getMetaData();

				// We require a new copy if meta does not match!
				if (meta)
					data = meta->newCopy(&rhs.data);
			}
		}

		return *this;
	}
    
    void Variant::copyInternal(void* value)
    {
        this->data = this->meta->newCopy(value);
    }
}
