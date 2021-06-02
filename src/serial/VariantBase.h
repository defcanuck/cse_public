#pragma once

#include "serial/JSON.h"
#include <iostream>

namespace cs
{
	class MetaData;

	class VariantBase
	{
	public:
		const MetaData *getMetaData() const;

		template <typename T>
		T& getValue(void);
		
		void *getData(void) const;
		
		template <typename T>
		const T& getValue(void) const;
		
		void serialize(std::ostream& os) const;
		void deserialize(JsonValue& value);
		const MetaData* deserializeNew(JsonValue& value);

	protected:
		VariantBase();
		VariantBase(const MetaData* meta, void* data);
		const MetaData *meta;
		void *data;
	};

	template <typename T>
	T& VariantBase::getValue(void)
	{
		return *reinterpret_cast<T *>(data);
	}

	template <typename T>
	const T& VariantBase::getValue(void) const
	{
		return *reinterpret_cast<T *>(data);
	}
}