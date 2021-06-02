#pragma once

#include <unordered_map>
#include <functional>
#include <string>
#include <typeindex>

#include "ui/imgui/GUIValue.h"
#include "ui/imgui/GUIFields.h"

#include "math/Transform.h"

#define DEFINE_META_GUI_VALUE(type_name) \
	new cs::GUICreator<type_name>(#type_name); 

#define DEFINE_META_GUI_SERIALIZABLE(type_name) \
	new cs::GUICreatorSerializable<type_name>(#type_name);

#define DEFINE_META_GUI_PTR(type_name) \
	new cs::GUICreatorPtr<type_name>(#type_name);

#define DEFINE_META_GUI_SERIALIZABLE_VECTOR(type_name) \
	new cs::GUICreatorSerializableVector<type_name>(#type_name);

#define DEFINE_META_GUI_SERIALIZABLE_MAP(type_name) \
	new cs::GUICreatorSerializableMap<type_name>(#type_name);

#define DEFINE_META_GUI_SERIALIZABLE_MAP_STRING(type_name) \
	new cs::GUICreatorSerializableStringMap<type_name>(#type_name);

namespace cs
{

	class GUIList;

	template <typename T>
	GUIValue* getMetaValue(GUIMemberInfo& info, T* value)
	{
		return getGUIValue<T>(info, value);
	}

	template <typename T>
	GUIValue* getMetaValuePtr(GUIMemberInfo& info, std::shared_ptr<T>* value)
	{
		return getGUIValue<std::shared_ptr<T>>(info, value);
	}

	class GUICreatorBase;

	class GUICreatorFactory : public Singleton<GUICreatorFactory>
	{
	public:

		typedef std::function<GUIValue*(const std::string&, void*)> DefaultGUICreator;
		DefaultGUICreator defaultCreator;

		static void initCreators();

		GUICreatorFactory() 
			: defaultCreator(nullptr) { }

		template <class T>
		void addCreator(GUICreatorBase* creator)
		{
			std::type_index index(typeid(T));
			this->creators[index] = creator;
		}

		template <class T>
		GUICreatorBase* getCreator()
		{
			std::type_index index(typeid(T));
			return this->getCreator(index);
		}

		GUICreatorBase* getCreator(std::type_index& index, bool& useDefault)
		{
			GUICreatorMap::iterator it;
			if ((it = this->creators.find(index)) != this->creators.end())
				return it->second;

			useDefault = true;
			static std::type_index defaultType(typeid(GUINullVal));
			return this->creators.find(defaultType)->second;
		}

		GUIValue* create(const Member* member, std::type_index& index, void* data_ptr, void* parent = nullptr, float32 indent = 0.0f);

		typedef std::map<std::type_index, GUICreatorBase*> GUICreatorMap;
		GUICreatorMap creators;

		static void populate(const MetaData* meta, void* ptr, GUIFields* fields, float32 indent = 0.0f);

	};

	class GUICreatorBase
	{
	public:
		GUICreatorBase(const std::string& n)
			: name(n) { }

		virtual GUIValue* create(const Member* member, void* value, void* parent = nullptr, float32 indent = 0.0f) { return nullptr; }
		
	private:

		std::string name;
	};

	template <class T>
	class GUICreator : public GUICreatorBase
	{
	public:

		GUICreator(const std::string& n)
			: GUICreatorBase(n)
		{
			GUICreatorFactory::getInstance()->addCreator<T>(this);
		}

		virtual GUIValue* create(const Member* member, void* value, void* parent = nullptr, float32 indent = 0.0f)
		{
			T* value_ptr = reinterpret_cast<T*>(value);
			if (member->getIsPointer())
			{
				std::shared_ptr<Serializable>* offsetPointer = reinterpret_cast<std::shared_ptr<Serializable>*>(value_ptr);
				value_ptr = (T*) offsetPointer->get();
			}

			GUIMemberInfo info(member, parent, indent);
			if (parent)
				member->createCallbacks(info.callbacks, parent);
			
			GUIValue* gui_value = getMetaValue<T>(info, value_ptr);
			if (gui_value && !member->getIsPointer())
			{
				T limitValue;
				for (int32 i = 0; i <= Member::MemberValueMax; i++)
				{
					Member::MemberValueType type = (Member::MemberValueType) i;
					if (member->getValue(type, limitValue))
					{
						setGUIValueLimit(gui_value, limitValue, (Member::MemberValueType) i);
					}
				}
			}
			return gui_value;
		}
	};

	template <class T>
	class GUICreatorSerializable : public GUICreatorBase
	{
	public:

		GUICreatorSerializable(const std::string& n)
			: GUICreatorBase(n)
		{
			GUICreatorFactory::getInstance()->addCreator<T>(this);
		}

		virtual GUIValue* create(const Member* member, void* value, void* parent = nullptr, float32 indent = 0.0f)
		{
			if (member->getIsPointer())
			{
				std::shared_ptr<T>* value_ptr = reinterpret_cast<std::shared_ptr<T>*>(value);
				std::shared_ptr<Serializable> serial_ptr = std::static_pointer_cast<Serializable>(*value_ptr);
				GUIMemberInfo info(member, parent, indent);

				if (parent)
					member->createCallbacks(info.callbacks, parent);

				return getMetaValuePtr<Serializable>(info, &serial_ptr);
			}

			Serializable* serial_ptr = reinterpret_cast<T*>(value);
			GUIMemberInfo info(member, parent, indent);

			if (parent)
				member->createCallbacks(info.callbacks, parent);

			return getMetaValue<Serializable>(info, serial_ptr);
			
		}
	};

	template <class T>
	class GUICreatorSerializableVector : public GUICreatorBase
	{
	public:

		GUICreatorSerializableVector(const std::string& n)
			: GUICreatorBase(n)
		{
			GUICreatorFactory::getInstance()->addCreator<T>(this);
		}

		virtual GUIValue* create(const Member* member, void* value, void* parent = nullptr, float32 indent = 0.0f)
		{
			SerializableVector* value_ptr = reinterpret_cast<SerializableVector*>(value);
			GUIMemberInfo info(member, parent, indent);

			if (parent)
				member->createCallbacks(info.callbacks, parent);
			
			return getMetaValue<SerializableVector>(info, value_ptr);
		}
	};

	template <class T>
	class GUICreatorSerializableMap: public GUICreatorBase
	{
	public:

		GUICreatorSerializableMap(const std::string& n)
			: GUICreatorBase(n)
		{
			GUICreatorFactory::getInstance()->addCreator<T>(this);
		}

		virtual GUIValue* create(const Member* member, void* value, void* parent = nullptr, float32 indent = 0.0f)
		{
			SerializableMap* value_ptr = reinterpret_cast<SerializableMap*>(value);
			GUIMemberInfo info(member, parent, indent);

			if (parent)
				member->createCallbacks(info.callbacks, parent);

			return getMetaValue<SerializableMap>(info, value_ptr);
		}
	};

	template <class T>
	class GUICreatorSerializableStringMap : public GUICreatorBase
	{
	public:

		GUICreatorSerializableStringMap(const std::string& n)
			: GUICreatorBase(n)
		{
			GUICreatorFactory::getInstance()->addCreator<T>(this);
		}

		virtual GUIValue* create(const Member* member, void* value, void* parent = nullptr, float32 indent = 0.0f)
		{
			SerializableMapString* value_ptr = reinterpret_cast<SerializableMapString*>(value);
			GUIMemberInfo info(member, parent, indent);

			if (parent)
				member->createCallbacks(info.callbacks, parent);

			return getMetaValue<SerializableMapString>(info, value_ptr);
		}
	};

	template <class T>
	class GUICreatorPtr : public GUICreatorBase
	{
	public:

		GUICreatorPtr(const std::string& n)
			: GUICreatorBase(n)
		{
			GUICreatorFactory::getInstance()->addCreator<T>(this);
		}

		virtual GUIValue* create(const Member* member, void* value, void* parent = nullptr, float32 indent = 0.0f)
		{
			std::shared_ptr<T>* value_ptr = reinterpret_cast<std::shared_ptr<T>*>(value);
			GUIMemberInfo info(member, parent, indent);
			
			if (parent)
				member->createCallbacks(info.callbacks, parent);

			return getMetaValuePtr<T>(info, value_ptr);
		}
	};

	inline GUIValue* GUICreatorFactory::create(const Member* member, std::type_index& index, void* data_ptr, void* parent, float32 indent)
	{
		bool useDefault = false;
		GUICreatorBase* creator = this->getCreator(index, useDefault);
		if (!creator)
		{
			return nullptr;
		}
		if (useDefault)
		{
			const MetaData* meta = member->getMetaData();
			log::print(LogInfo, "No GUI found for ", member->getName(), " with type ", meta->getName());
		}
		GUIValue* value = creator->create(member, data_ptr, parent, indent);
		
		return value;
	}


	
}