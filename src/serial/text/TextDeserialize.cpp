#include "PCH.h"

#include "serial/text/TextDeserialize.h"
#include "serial/MetaData.h"

namespace cs
{
	namespace text
	{
		void deserializeImpl(JsonIterator& it, const MetaData* meta, RefVariant& var)
		{
			MetaData::MemberMap members;
			meta->getCompleteMemberMap(members);

			while (it != end(JsonValue()))
			{
				std::string member_key = it->key;
				
				MetaData::MemberMap::const_iterator map_it = members.find(member_key);
				if (map_it != members.end())
				{
					const Member* member = map_it->second;

					const MetaData* member_metadata = member->getMetaData();
					void *offsetData = PTR_ADD(var.getData(), member->getOffset());

					JsonValue json_value = it->value;

					if (member->getIsPointer())
					{
						member_metadata->deserializeNew(json_value, RefVariant(member_metadata, offsetData));
					}
					else
					{
						member_metadata->deserialize(json_value, RefVariant(member_metadata, offsetData));
					}
						
				} else {
					log_error("Nothing serializable for ", member_key);
				}
				++it;
			}
		}

		void deserializeMembers(JsonValue value, RefVariant var)
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
				return;
			}
			++it;

			deserializeImpl(it, meta, var);

		}

		template <>
		void deserializePrim<std::string>(JsonValue value, RefVariant prim)
		{
			std::string& dst = *reinterpret_cast<std::string*>(prim.getData());
			dst = std::string(value.toString());
		}

		template <>
		void deserializePrim<int32>(JsonValue value, RefVariant prim)
		{
			const MetaData* meta = prim.getMetaData();
			int32& dst = *reinterpret_cast<int32*>(prim.getData());

			if (meta->hasToStringValue())
			{
				std::string str = value.toString();
				int32 idx = meta->convertToEnum(str);
				if (idx >= 0)
					dst = idx;
				else 
					log::print(LogError, "Unknown string value: ", str, " for metadata ", meta->getName());
				
			} else 
				dst = int32(value.toNumber());
			
		}

		TEXT_DESERIALIZE_BITFIELD_IMPL(RenderTraversalMask)

		template <>
		void deserializePrim<uint16>(JsonValue value, RefVariant prim)
		{
			uint16& dst = *reinterpret_cast<uint16*>(prim.getData());
			dst = (uint16) value.toNumber();
		}

		template <>
		void deserializePrim<float32>(JsonValue value, RefVariant prim)
		{
			float32& dst = *reinterpret_cast<float32*>(prim.getData());
			dst = (float32)value.toNumber();
		}

		template <>
		void deserializePrim<vec2>(JsonValue value, RefVariant prim)
		{
			vec2& dst = *reinterpret_cast<vec2*>(prim.getData());
			JsonIterator it = begin(value);
			setAndInc<float32>(it, dst.x);
			setAndInc<float32>(it, dst.y);
		}

		template <>
		void deserializePrim<SizeF>(JsonValue value, RefVariant prim)
		{
			SizeF& sz = *reinterpret_cast<SizeF*>(prim.getData());
			JsonIterator it = begin(value);
			setAndInc<float32>(it, sz.w);
			setAndInc<float32>(it, sz.h);
		}

		template <>
		void deserializePrim<SizeI>(JsonValue value, RefVariant prim)
		{
			SizeI& sz = *reinterpret_cast<SizeI*>(prim.getData());
			JsonIterator it = begin(value);
			setAndInc<int32>(it, sz.w);
			setAndInc<int32>(it, sz.h);
		}

		template <>
		void deserializePrim<vec3>(JsonValue value, RefVariant prim)
		{
			vec3& dst = *reinterpret_cast<vec3*>(prim.getData());
			JsonIterator it = begin(value);
			setAndInc<float32>(it, dst.x);
			setAndInc<float32>(it, dst.y);
			setAndInc<float32>(it, dst.z);
		}

		template <>
		void deserializePrim<vec4>(JsonValue value, RefVariant prim)
		{
			vec4& dst = *reinterpret_cast<vec4*>(prim.getData());
			JsonIterator it = begin(value);
			setAndInc<float32>(it, dst.x);
			setAndInc<float32>(it, dst.y);
			setAndInc<float32>(it, dst.z);
			setAndInc<float32>(it, dst.w);
		}

		template <>
		void deserializePrim<quat>(JsonValue value, RefVariant prim)
		{
			quat& dst = *reinterpret_cast<quat*>(prim.getData());
			JsonIterator it = begin(value);
			setAndInc<float32>(it, dst.x);
			setAndInc<float32>(it, dst.y);
			setAndInc<float32>(it, dst.z);
			setAndInc<float32>(it, dst.w);
		}

		template <>
		void deserializePrim<RectF>(JsonValue value, RefVariant prim)
		{
			RectF& dst = *reinterpret_cast<RectF*>(prim.getData());
			JsonIterator it = begin(value);
			setAndInc<float32>(it, dst.pos.x);
			setAndInc<float32>(it, dst.pos.y);
			setAndInc<float32>(it, dst.size.w);
			setAndInc<float32>(it, dst.size.h);
		}

		template <>
		void deserializePrim<ColorB>(JsonValue value, RefVariant prim)
		{
			ColorB& dst = *reinterpret_cast<ColorB*>(prim.getData());
			JsonIterator it = begin(value);
			setAndInc<uchar>(it, dst.r);
			setAndInc<uchar>(it, dst.g);
			setAndInc<uchar>(it, dst.b);
			setAndInc<uchar>(it, dst.a);
		}

		template <>
		void deserializePrim<ColorF>(JsonValue value, RefVariant prim)
		{
			ColorF& dst = *reinterpret_cast<ColorF*>(prim.getData());
			JsonIterator it = begin(value);
			setAndInc<float32>(it, dst.r);
			setAndInc<float32>(it, dst.g);
			setAndInc<float32>(it, dst.b);
			setAndInc<float32>(it, dst.a);
		}

		template <>
		void deserializePrim<bool>(JsonValue value, RefVariant prim)
		{
			bool& dst = *reinterpret_cast<bool*>(prim.getData());
			dst = (std::string(value.toString()) == kTrue) ? true : false;
		}
	}
}