#include "PCH.h"

#include "ClassDef.h"

#include "serial/text/TextSerialize.h"
#include "serial/MetaData.h"
#include "serial/RefVariant.h"
#include "serial/RemQual.h"

namespace cs
{
	namespace text
	{

		const float kVerySmallFloatValue = 0.00001f;
		inline float cap(float val)
		{
			if (val > -kVerySmallFloatValue && val < kVerySmallFloatValue)
				return 0.0f;
			return val;
		}

		template <>
		void serializePrim<int32>(std::ostream& oss, RefVariant prim)
		{
			const MetaData* meta = prim.getMetaData();
			if (meta->hasToStringValue())
			{
				uint32 idx = (uint32)prim.getValue<RemQual<int32>::type>();
				serializeWQ(oss, meta->convertToString(idx));
				return;
			}
			oss << prim.getValue<RemQual<int32>::type>();

		}

		template <>
		void serializePrim<uint32>(std::ostream& oss, RefVariant prim)
		{
			const MetaData* meta = prim.getMetaData();
			if (meta->hasToStringValue())
			{
				uint32 idx = (uint32)prim.getValue<RemQual<int32>::type>();
				serializeWQ(oss, meta->convertToString(idx));
				return;
			}
			oss << prim.getValue<RemQual<int32>::type>();

		}

		TEXT_SERIALIZE_BITFIELD_IMPL(RenderTraversalMask)

		template <>
		void serializePrim<uint16>(std::ostream& oss, RefVariant prim)
		{
			uint16& uival = *reinterpret_cast<uint16*>(prim.getData());
			oss << uival;
		}

		template <>
		void serializePrim<float32>(std::ostream& oss, RefVariant prim)
		{
			float32& fval = *reinterpret_cast<float32*>(prim.getData());
			oss << fval;
		}

		template <>
		void serializePrim<bool>(std::ostream& oss, RefVariant prim)
		{
			bool& bval = *reinterpret_cast<bool*>(prim.getData());
			if (bval) serializeWQ(oss, kTrue); else serializeWQ(oss, kFalse);
		}

		template <>
		void serializePrim<vec2>(std::ostream& oss, RefVariant prim)
		{
			vec2& vec = *reinterpret_cast<vec2*>(prim.getData());
			oss << "[" << cap(vec.x) << ", " << cap(vec.y) << "]";
		}

		template <>
		void serializePrim<SizeF>(std::ostream& oss, RefVariant prim)
		{
			SizeF& sz = *reinterpret_cast<SizeF*>(prim.getData());
			oss << "[" << sz.w << ", " << sz.h << "]";
		}

		template <>
		void serializePrim<SizeI>(std::ostream& oss, RefVariant prim)
		{
			SizeI& sz = *reinterpret_cast<SizeI*>(prim.getData());
			oss << "[" << sz.w << ", " << sz.h << "]";
		}

		template <>
		void serializePrim<vec3>(std::ostream& oss, RefVariant prim)
		{
			vec3& vec = *reinterpret_cast<vec3*>(prim.getData());
			oss << "[" << cap(vec.x) << ", " << cap(vec.y) << ", " << cap(vec.z) << "]";
		}

		template <>
		void serializePrim<vec4>(std::ostream& oss, RefVariant prim)
		{
			vec4& vec = *reinterpret_cast<vec4*>(prim.getData());
			oss << "[" << cap(vec.x) << ", " << cap(vec.y) << ", " << cap(vec.z) << ", " << cap(vec.w) << "]";
		}

		template <>
		void serializePrim<quat>(std::ostream& oss, RefVariant prim)
		{
			quat& q = *reinterpret_cast<quat*>(prim.getData());
			oss << "[" << q.x << ", " << q.y << ", " << q.z << ", " << q.w << "]";
		}

		template <>
		void serializePrim<RectF>(std::ostream& oss, RefVariant prim)
		{
			RectF& r = *reinterpret_cast<RectF*>(prim.getData());
			oss << "[ " << r.pos << ", " << r.size << " ]";
		}

		template <>
		void serializePrim<ColorB>(std::ostream& oss, RefVariant prim)
		{
			ColorB& color = *reinterpret_cast<ColorB*>(prim.getData());
			uint32 r = static_cast<uint32>(color.r);
			uint32 g = static_cast<uint32>(color.g);
			uint32 b = static_cast<uint32>(color.b);
			uint32 a = static_cast<uint32>(color.a);
			oss << "[ " << r << ", " << g << ", " << b << ", " << a << " ]";
		}

		template <>
		void serializePrim<ColorF>(std::ostream& oss, RefVariant prim)
		{
			ColorF& color = *reinterpret_cast<ColorF*>(prim.getData());
			float32 r = color.r;
			float32 g = color.g;
			float32 b = color.b;
			float32 a = color.a;
			oss << "[ " << r << ", " << g << ", " << b << ", " << a << " ]";
		}

		template <>
		void serializePrim<std::string>(std::ostream& oss, RefVariant prim)
		{
			serializeWQ(oss, prim.getValue<RemQual<std::string>::type>());
		}

		void padding(std::ostream& os)
		{
			for (unsigned i = 0; i < level; ++i)
				os << "      ";
		}

		void serializeMembersImpl(std::ostream& os, RefVariant var, bool isResource)
		{
			const MetaData* meta = var.getMetaData();
			void* data = var.getData();

			MetaData::MemberList members;
			meta->getAllMembers(members);

			os << "{" << std::endl;

			++level;
			padding(os);
			serializeWQ(os, kClassTypeStr);
			os << " : ";
			serializeWQ(os, meta->getName());

			if (members.size() > 0)
				os << "," << std::endl;

			MetaData::MemberList::const_iterator it = members.begin();
			while (true && members.size() > 0)
			{
				const Member* member = (*it);
				bool hasMember = false;

				bool hasObjectOverride = meta->hasObjectOverride(data);
				bool shouldIgnoreSerialization = (hasObjectOverride) ? meta->isOverrideSet(member, Member::MemberFlagIgnoreSerialization, data) : member->getIgnoreSerialization() || meta->isOverrideSet(member, Member::MemberFlagIgnoreSerialization);
				if (isResource && member->getName() != "name")
				{
					shouldIgnoreSerialization = true;
					log::info("Ignoring resource serialization for member ", member->getName());
				}

				if (!shouldIgnoreSerialization)
				{
					void* offsetData = PTR_ADD(var.getData(), member->getOffset());
					const MetaData* metadata = member->getMetaData();
					if (member->getIsPointer())
					{
						std::shared_ptr<Serializable>* offsetPointer = reinterpret_cast<std::shared_ptr<Serializable>*>(offsetData);
						offsetData = (void*)offsetPointer->get();
						Serializable* serial = reinterpret_cast<Serializable*>(offsetData);
						if (serial)
						{
							padding(os);
							serializeWQ(os, member->getName());
							os << " : ";
							serial->serialize(os);
							hasMember = true;
						}
					}
					else 
					{
						bool skipMember = 
							member->hasValue(Member::MemberValueDefault) && 
							member->equalsValue(Member::MemberValueDefault, offsetData);

						if (!skipMember)
						{
							padding(os);
							serializeWQ(os, member->getName());
							os << " : ";

							metadata->serialize(os, RefVariant(metadata, offsetData));
							hasMember = true;
						}
						else
						{
							log::info("Skipping ", member->getName(), " no changes to default value!");
						}
					}
				}

				it++;
				if (it == members.end())
				{
					os << std::endl;
					break;
				}

				if (hasMember)
					os << "," << std::endl;
			}

			--level;
			padding(os);
			os << "}";
		}

		void serializeResource(std::ostream& oss, RefVariant var)
		{
			serializeMembersImpl(oss, var, true);
		}

		void serializeMembers(std::ostream& oss, RefVariant var)
		{
			serializeMembersImpl(oss, var, false);
		}
	}
}