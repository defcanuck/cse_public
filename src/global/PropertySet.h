#pragma once

#include "ClassDef.h"

#include "global/Resource.h"
#include "global/SerializableHandle.h"
#include "global/Event.h"
#include "math/GLM.h"

#include <string>
#include <vector>

namespace cs
{
	enum PropertyType
	{
		PropertyTypeNone = -1,
		PropertyTypeString,
		PropertyTypeTable,
		PropertyTypeColor,
		PropertyTypeInteger,
		PropertyTypeDouble,
		PropertyTypeVector,
		//...
		PropertyTypeMAX
	};

	CLASS_DEFINITION_REFLECT(PropertyBase)
	public:
		PropertyBase() : propName("empty_name") { }
		PropertyBase(const std::string& str) : propName(str) { }

		virtual PropertyType getType() const { return PropertyTypeNone; }

		std::string propName;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(PropertyString, PropertyBase)
	public:
		PropertyString() 
			: PropertyBase("empty_string")
			, propString("unset") 
		{ }

		PropertyString(const std::string& str, const std::string& val) 
			: PropertyBase(str)
			, propString(val) 
		{ }

		PropertyString(PropertyStringPtr& rhs)
			: PropertyBase(rhs->propName)
			, propString(rhs->propString)
		{ }
		
		virtual PropertyType getType() const { return PropertyTypeString; }
		
		std::string propString;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(PropertyInteger, PropertyBase)
	public:
		PropertyInteger()
			: PropertyBase("empty_string")
			, propNumber(0)
		{ }

		PropertyInteger(const std::string& str, int32 num)
			: PropertyBase(str)
			, propNumber(num)
		{ }

		PropertyInteger(PropertyIntegerPtr& rhs)
			: PropertyBase(rhs->propName)
			, propNumber(rhs->propNumber)
		{ }

		virtual PropertyType getType() const { return PropertyTypeInteger; }

		int32 propNumber;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(PropertyDouble, PropertyBase)
	public:
		PropertyDouble()
			: PropertyBase("empty_string")
			, propNumber(0)
		{ }

		PropertyDouble(const std::string& str, float32 num)
			: PropertyBase(str)
			, propNumber(num)
		{ }

		PropertyDouble(PropertyDoublePtr& rhs)
			: PropertyBase(rhs->propName)
			, propNumber(rhs->propNumber)
		{ }

		virtual PropertyType getType() const { return PropertyTypeDouble; }

		float32 propNumber;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(PropertyColor, PropertyBase)
	public:
		PropertyColor()
			: PropertyBase("empty_string")
			, propColor(ColorB::White)
		{ }

		PropertyColor(const std::string& str, const ColorB &val)
			: PropertyBase(str)
			, propColor(val)
		{ }

		PropertyColor(PropertyColorPtr& rhs)
			: PropertyBase(rhs->propName)
			, propColor(rhs->propColor)
		{ }

		virtual PropertyType getType() const { return PropertyTypeColor; }

		ColorB propColor;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(PropertyVector3, PropertyBase)
	public:
		PropertyVector3()
			: PropertyBase("empty_string")
			, propVector(kZero3)
		{ }

		PropertyVector3(const std::string& str, const vec3 &val)
			: PropertyBase(str)
			, propVector(val)
		{ }

		PropertyVector3(PropertyVector3Ptr& rhs)
			: PropertyBase(rhs->propName)
			, propVector(rhs->propVector)
		{ }

		virtual PropertyType getType() const { return PropertyTypeVector; }

		vec3 propVector;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(PropertySet, PropertyBase)
	public:
		
		static PropertySet kEmptyPropertySet;
		static PropertySetPtr kEmptyPropertySetPtr;

		PropertySet() 
			: PropertyBase("root") 
		{ }
		
		PropertySet(const std::string& str) 
			: PropertyBase(str) 
		{ }
		
		PropertySet(const PropertySetPtr& rhs);
		PropertySet(const PropertySet& rhs);

		virtual PropertyType getType() const { return PropertyTypeTable; }

		static PropertyBasePtr copy(const PropertyBasePtr& ptr);

		template <class T>
		bool addProperty(std::shared_ptr<T>& prop)
		{
			this->propVector.push_back(std::static_pointer_cast<PropertyBase>(prop));
			return true;
		}

		template <class T>
		bool addPropertySafe(std::shared_ptr<T>& prop)
		{
			for (auto& it : this->propVector)
			{
				if (prop->propName == it->propName)
				{
					log::error("Dupilcate names!");
					return false;
				}
			}

			return addProperty(prop);
		}

		typedef std::vector<PropertyBasePtr> PropertyVector;
		PropertyVector propVector;
	};

	CLASS_DEFINITION_DERIVED_REFLECT(PropertySetResource, Resource)
	public:

		PropertySetResource() : Resource("Error") { }
		PropertySetResource(const std::string& fname, const std::string& path);

		PropertySetPtr& getPropertySetPtr() { return this->propertySet.get(); }
		const PropertySet& getPropertySet()
		{
			if (this->propertySet.get()) return *(this->propertySet.get());
			return PropertySet::kEmptyPropertySet;
		}

		SerializableHandle<PropertySet>& getPropertySetHandle() { return this->propertySet; }

		void save();
		void update(PropertySet prop);

	private:

		SerializableHandle<PropertySet> propertySet;

	};

	CLASS_DEFINITION_REFLECT(PropertySetHandle)
	public:
		PropertySetHandle()
			: resource(nullptr) { }
		PropertySetHandle(PropertySetResourcePtr& ptr)
			: resource(ptr) { }

		PropertySetHandle(const std::string& fileName);

		Event onChanged;
		void onPropertySetChanged();

		PropertySetResourcePtr& getPropertySetResource() { return this->resource; }
		PropertySet& getPropertySet() 
		{
			if (this->resource.get()) return *(this->resource->getPropertySetPtr().get());
			return PropertySet::kEmptyPropertySet;
		}

		void clear()
		{
			this->resource = nullptr;
			this->onChanged.invoke();
		}

		bool hasPropertySetResource() const { return this->resource.get() != nullptr; }

	private:

		PropertySetResourcePtr resource;

	};

	CLASS_DEFINITION_REFLECT(PropertySetHandleOverload)
	public:

		PropertySetHandleOverload()
			: propHandle(CREATE_CLASS(PropertySetHandle))
			, overloadName("") { }

		PropertySetHandlePtr propHandle;
		std::string overloadName;
	};

#if defined(CS_WINDOWS)

	template <>
	std::string SerializableHandle<PropertySet>::getExtension()  { return "prop"; }

	template <>
	std::string SerializableHandle<PropertySet>::getDescription()  { return "CSE PropertySet"; }

#else
	
	template <>
	std::string SerializableHandle<PropertySet>::getExtension();

	template <>
	std::string SerializableHandle<PropertySet>::getDescription();

#endif
}
