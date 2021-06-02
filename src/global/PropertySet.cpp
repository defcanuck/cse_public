#include "PCH.h"

#include "global/PropertySet.h"

#include "global/ResourceFactory.h"

namespace cs
{

	PropertySet PropertySet::kEmptyPropertySet;
	PropertySetPtr PropertySet::kEmptyPropertySetPtr = CREATE_CLASS(PropertySet);

	DEFINE_META_VECTOR_NEW(PropertySet::PropertyVector, PropertyBase, PropertyVector);

	BEGIN_META_CLASS(PropertyBase)

		ADD_MEMBER(propName);

	END_META();

	BEGIN_META_CLASS(PropertyString)

		ADD_MEMBER(propString);

	END_META();

	BEGIN_META_CLASS(PropertyColor)

		ADD_MEMBER(propColor);

	END_META();

	BEGIN_META_CLASS(PropertyInteger)

		ADD_MEMBER(propNumber);

	END_META();

	BEGIN_META_CLASS(PropertyDouble)

		ADD_MEMBER(propNumber);

	END_META();

	BEGIN_META_CLASS(PropertyVector3)

		ADD_MEMBER(propVector);

	END_META();

	BEGIN_META_CLASS(PropertySet)

		ADD_MEMBER(propVector);
			ADD_COMBO_META_LABEL(PropertyString, "String");
			ADD_COMBO_META_LABEL(PropertyColor, "Color");
			ADD_COMBO_META_LABEL(PropertySet, "Set");
			ADD_COMBO_META_LABEL(PropertyVector3, "Vector");
			SET_MEMBER_COLLAPSEABLE();
			SET_MEMBER_START_COLLAPSED();

	END_META();

	BEGIN_META_RESOURCE(PropertySetResource)

	END_META();

	BEGIN_META_CLASS(PropertySetHandle)

		ADD_MEMBER_RESOURCE(resource);
			SET_MEMBER_CALLBACK_POST(&PropertySetHandle::onPropertySetChanged);

	END_META();

	BEGIN_META_CLASS(PropertySetHandleOverload)
		ADD_MEMBER_PTR(propHandle);
		ADD_MEMBER(overloadName);
	END_META();

	PropertySet::PropertySet(const PropertySetPtr& rhs)
		: PropertyBase(rhs->propName)
	{
		for (auto& it : rhs->propVector)
		{
			PropertyBasePtr ptr = PropertySet::copy(it);
			if (ptr.get())
			{
				this->propVector.push_back(ptr);
			}
		}
	}

	PropertySet::PropertySet(const PropertySet& rhs)
	{
		for (auto& it : rhs.propVector)
		{
			PropertyBasePtr ptr = PropertySet::copy(it);
			if (ptr.get())
			{
				this->propVector.push_back(ptr);
			}
		}
	}

	PropertyBasePtr PropertySet::copy(const PropertyBasePtr& ptr)
	{
		PropertyBasePtr retPtr;
		PropertyType propType = ptr->getType();
		switch (propType)
		{
			case PropertyTypeString:
			{
				PropertyStringPtr string_prop = std::dynamic_pointer_cast<PropertyString>(ptr);
				assert(string_prop.get());
				retPtr = CREATE_CLASS_CAST(PropertyBase, PropertyString, string_prop);
			} break;
			case PropertyTypeTable:
			{
				PropertySetPtr table_prop = std::dynamic_pointer_cast<PropertySet>(ptr);
				assert(table_prop.get());
				retPtr = CREATE_CLASS_CAST(PropertyBase, PropertySet, table_prop);
			} break;
			case PropertyTypeColor:
			{
				PropertyColorPtr color_prop = std::dynamic_pointer_cast<PropertyColor>(ptr);
				assert(color_prop.get());
				retPtr = CREATE_CLASS_CAST(PropertyBase, PropertyColor, color_prop);
			} break;
			case PropertyTypeInteger:
			{
				PropertyIntegerPtr int_prop = std::dynamic_pointer_cast<PropertyInteger>(ptr);
				assert(int_prop.get());
				retPtr = CREATE_CLASS_CAST(PropertyBase, PropertyInteger, int_prop);
			} break;
			case PropertyTypeDouble:
			{
				PropertyDoublePtr dbl_prop = std::dynamic_pointer_cast<PropertyDouble>(ptr);
				assert(dbl_prop.get());
				retPtr = CREATE_CLASS_CAST(PropertyBase, PropertyDouble, dbl_prop);
			} break;
			case PropertyTypeVector:
			{
				PropertyVector3Ptr vec_prop = std::dynamic_pointer_cast<PropertyVector3>(ptr);
				assert(vec_prop.get());
				retPtr = CREATE_CLASS_CAST(PropertyBase, PropertyVector3, vec_prop);
			} break;
			default:
				assert(false);
		}
		return retPtr;
	}

	PropertySetResource::PropertySetResource(const std::string& fname, const std::string& path)
		: Resource(fname) 
	{ 
		this->propertySet = SerializableHandle<PropertySet>(CREATE_CLASS(PropertySet, "root"));
		this->propertySet.setFullPath(path);
		if (!this->propertySet.refresh())
		{
            PropertySetPtr tempProp = CREATE_CLASS(PropertySet);
			this->propertySet.set(tempProp);
		}
	}

	void PropertySetResource::save() 
	{ 
		this->propertySet.save(); 
	}

	void PropertySetResource::update(PropertySet prop)
	{
		PropertySetPtr update_ptr = CREATE_CLASS(PropertySet, prop);
		this->propertySet.set(update_ptr);
	}

	PropertySetHandle::PropertySetHandle(const std::string& fileName)
	{
		this->resource = std::static_pointer_cast<PropertySetResource>(
			ResourceFactory::getInstance()->loadResource<PropertySetResource>(fileName));
	}

	void PropertySetHandle::onPropertySetChanged()
	{
		this->onChanged.invoke();
	}
    
#if !defined(CS_WINDOWS)
    template <>
    std::string SerializableHandle<PropertySet>::getExtension()
    {
        return "prop";
    }
    
    template <>
    std::string SerializableHandle<PropertySet>::getDescription()
    {
        return "CSE PropertySet";
    }
#endif

}
