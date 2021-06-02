#include "PCH.h"

#include "serial/VariantBase.h"
#include "serial/MetaData.h"
#include "serial/RefVariant.h"

namespace cs
{

	VariantBase::VariantBase() : 
		meta(nullptr), 
		data(nullptr)
	{
	}

	VariantBase::VariantBase(const MetaData* m, void* d) : 
		meta(m), 
		data(d)
	{
	}

	const MetaData *VariantBase::getMetaData() const
	{
		return meta;
	}

	void VariantBase::serialize(std::ostream& os) const
	{
		meta->serialize(os, RefVariant(this->meta, this->data));
	}

	void VariantBase::deserialize(JsonValue& value)
	{
		meta->deserialize(value, RefVariant(this->meta, this->data));
	}

	const MetaData* VariantBase::deserializeNew(JsonValue& value)
	{
		return meta->deserializeNew(value, RefVariant(this->meta, this->data));
	}

	void *VariantBase::getData() const
	{
		return data;
	}
}