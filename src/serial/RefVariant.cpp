#include "PCH.h"

#include "serial/RefVariant.h"

namespace cs
{
	RefVariant::RefVariant(void) : 
		VariantBase(nullptr, nullptr)
	{
	}

	RefVariant::RefVariant(const MetaData *m, void *d) : 
		VariantBase(m, d)
	{
	}

	RefVariant::RefVariant(const RefVariant& rhs) : 
		VariantBase(rhs.getMetaData(), rhs.getData())
	{
	}

	RefVariant::RefVariant(const Variant& rhs) : 
		VariantBase(rhs.getMetaData(), rhs.getData())
	{
	}

	RefVariant& RefVariant::operator=(const RefVariant& rhs)
	{
		meta = rhs.meta;
		data = rhs.data;
		return *this;
	}

	RefVariant& RefVariant::operator=(const Variant& rhs)
	{
		meta = rhs.getMetaData();
		data = rhs.getData();
		return *this;
	}
}