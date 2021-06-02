#pragma once

#include "serial/text/TextSerial.h"
//#include "global/ResourceFactory.h"

namespace cs
{
	namespace text
	{
		void deserializeMembers(JsonValue value, RefVariant var);
		void deserializeImpl(JsonIterator& it, const MetaData* meta, RefVariant& var);

		template <typename T>
		void setAndInc(JsonIterator& it, T& value)
		{
			value = static_cast<T>(it->value.toNumber());
			++it;
		}

		inline void setAndIncBool(JsonIterator& it, bool& value)
		{
			value = (static_cast<int32>(it->value.toNumber()) == 1);
			++it;
		}

		template <typename T>
		const MetaData* deserializeMembersNew(JsonValue value, RefVariant var)
		{
			JsonIterator it = begin(value);

			std::string key = it->key;
			assert(key == kClassTypeStr);

			assert(it->value.getTag() == JSON_STRING);
			std::string class_type = it->value.toString();
			const MetaData* meta = MetaManager::getInstance()->get(class_type);
			if (!meta)
			{
				log::print(LogError, "Unknown class found: ", class_type);
				return nullptr;
			}

			std::shared_ptr<T>& dst = *reinterpret_cast<std::shared_ptr<T>*>(var.getData());
			dst = std::shared_ptr<T>(reinterpret_cast<T*>(meta->createNew()));
			
			++it;

			dst->onPreLoad();
            RefVariant ref(meta, dst.get());
			deserializeImpl(it, meta, ref);
			
			return meta;
		}

		template <typename T>
		const MetaData* deserializeResource(JsonValue value, RefVariant var)
		{
			JsonIterator it = begin(value);

			std::string key = it->key;
			assert(key == kClassTypeStr);

			assert(it->value.getTag() == JSON_STRING);
			std::string class_type = it->value.toString();
			const MetaData* meta = MetaManager::getInstance()->get(class_type);
			if (!meta)
			{
				log::print(LogError, "Unknown class found: ", class_type);
				return nullptr;
			}

			++it;

			std::shared_ptr<T>& dst = *reinterpret_cast<std::shared_ptr<T>*>(var.getData());
			const MetaData::MemberMap& members = meta->getMembers();
			while (it != end(JsonValue()))
			{
				std::string member_key = it->key;
				
				if (member_key == "name")
				{
					MetaData::MemberMap::const_iterator map_it = members.find(member_key);
					JsonValue json_value = it->value;
					const std::string& name = json_value.toString();

					std::shared_ptr<Resource> resource = meta->createFromString(name);
					dst = std::static_pointer_cast<T>(resource);
				}
				++it;
			}

			return nullptr;
		}

		template <class T>
		void deserializePrim(JsonValue value, RefVariant prim)
		{
			const MetaData* meta = prim.getMetaData();
			log::print(LogError, "Cannot find Deserialization routine for type ", meta->getName());
		}

		TEXT_DESERIALIZE(RenderTraversalMask);

		template <>
		void deserializePrim<int32>(JsonValue value, RefVariant prim);

		template <>
		void deserializePrim<uint16>(JsonValue value, RefVariant prim);

		template <>
		void deserializePrim<float32>(JsonValue value, RefVariant prim);

		template <>
		void deserializePrim<std::string>(JsonValue value, RefVariant prim);

		template <>
		void deserializePrim<vec2>(JsonValue value, RefVariant prim);

		template <>
		void deserializePrim<SizeF>(JsonValue value, RefVariant prim);

		template <>
		void deserializePrim<SizeI>(JsonValue value, RefVariant prim);

		template <>
		void deserializePrim<vec3>(JsonValue value, RefVariant prim);

		template <>
		void deserializePrim<vec4>(JsonValue value, RefVariant prim);

		template <>
		void deserializePrim<quat>(JsonValue value, RefVariant prim);

		template <>
		void deserializePrim<RectF>(JsonValue value, RefVariant prim);

		template <>
		void deserializePrim<bool>(JsonValue value, RefVariant prim);

		template <>
		void deserializePrim<ColorB>(JsonValue value, RefVariant prim);

		template <>
		void deserializePrim<ColorF>(JsonValue value, RefVariant prim);

		template <class Key, class T>
		void deserializeMapNew(JsonValue object, RefVariant value)
		{
			std::map<Key, std::shared_ptr<T>>& dst =
				*reinterpret_cast<std::map<Key, std::shared_ptr<T>>*>(value.getData());

			MetaData* meta = MetaCreator<T>::get();
			size_t ctr = 0;
			for (auto i : object)
			{
				JsonIterator it = begin(i->value);
				
				std::string key = it->key;
				JsonValue map_object = it->value;
				
				std::shared_ptr<T> newObject;
				const MetaData* object_meta = 
					meta->deserializeNew(map_object, RefVariant(meta, (void*)&newObject));
				
				if (object_meta)
				{
					std::stringstream sstream(key);
					Key key_value;
					sstream >> key_value;

					assert(dst.count(key_value) == 0);
					dst[key_value] = std::static_pointer_cast<T>(newObject);
				}
			}
		}

		template <class T>
		void deserializeVectorNew(JsonValue object, RefVariant value)
		{
			std::vector<std::shared_ptr<T>>& dst = 
				*reinterpret_cast<std::vector<std::shared_ptr<T>>*>(value.getData());

			MetaData* meta = MetaCreator<T>::get();
			for (auto i : object)
			{
				JsonValue map_object = i->value;
				std::shared_ptr<T> newObject;

				meta->deserializeNew(map_object, RefVariant(meta, (void*)&newObject));
				dst.push_back(newObject);
			}
		}

		template <class T>
		void deserializeVector(JsonValue object, RefVariant value)
		{
			std::vector<T>& dst =
				*reinterpret_cast<std::vector<T>*>(value.getData());

			MetaData* meta = MetaCreator<T>::get();
			if (!meta->hasDeserialization())
			{
				log::error("Cannot deserialize type ", meta->getName());
				return;
			}

			size_t cnt = 0;
			for (auto i : object)
			{
				JsonValue map_object = i->value;
				T newObject;

				meta->deserialize(map_object, RefVariant(meta, (void*)&newObject));
				dst.push_back(newObject);
				cnt++;
			}
			// log::info("Vector Deserialize count for ", meta->getName(), " : ", cnt);
		}
	}
}