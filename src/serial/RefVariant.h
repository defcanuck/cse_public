#pragma once

#include "serial/Variant.h"
#include "serial/VariantBase.h"
namespace cs
{

	class MetaData;

	class RefVariant : public VariantBase
	{
	public:
		
		template <class T>
		RefVariant(void* data);

		template <class T>
		RefVariant(const T& value);

		template <class T>
		RefVariant& operator=(const T& rhs);

		RefVariant(const RefVariant& rhs);
		RefVariant(const Variant& rhs);
		RefVariant(const MetaData *m, void *d);
		RefVariant();

		RefVariant& operator=(const RefVariant& rhs);
		RefVariant& operator=(const Variant& rhs);
		
	};

	template <class T>
	RefVariant::RefVariant(void* data) :
		VariantBase((MetaCreator<typename RemQual<T>::type>::get()), data)
	{
	}

	template <class T>
	RefVariant::RefVariant(const T& value) : 
		VariantBase(META_TYPE(T), const_cast<T*>(&value))
	{
	}

	template <class T>
	RefVariant& RefVariant::operator=(const T& rhs)
	{
		meta = META_TYPE(T);
		data = const_cast<T*>(&rhs);
		return *this;
	}
}
