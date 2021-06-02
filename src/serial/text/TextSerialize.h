#pragma once

#include "serial/text/TextSerial.h"

namespace cs
{
	
	namespace text
	{

		inline void serializeWQ(std::ostream& oss, const std::string& str)
		{
			oss << "\"" << str << "\"";
		}

		void padding(std::ostream& os);
		void serializeMembers(std::ostream& os, RefVariant var);
		void serializeResource(std::ostream& oss, RefVariant var);
		
		template <class T>
		void serializePrim(std::ostream& oss, RefVariant prim)
		{
			const MetaData* meta = prim.getMetaData();
			log::print(LogError, "Cannot find Serialization routine for type ", meta->getName());
		}

		template <>
		void serializePrim<uint16>(std::ostream& oss, RefVariant prim);

		template <>
		void serializePrim<int32>(std::ostream& oss, RefVariant prim);

		template <>
		void serializePrim<uint32>(std::ostream& oss, RefVariant prim);

		template <>
		void serializePrim<float32>(std::ostream& oss, RefVariant prim);

		template <>
		void serializePrim<bool>(std::ostream& oss, RefVariant prim);

		template <>
		void serializePrim<std::string>(std::ostream& oss, RefVariant prim);

		template <>
		void serializePrim<vec2>(std::ostream& oss, RefVariant prim);

		template <>
		void serializePrim<vec3>(std::ostream& oss, RefVariant prim);

		template <>
		void serializePrim<vec4>(std::ostream& oss, RefVariant prim);

		template <>
		void serializePrim<quat>(std::ostream& oss, RefVariant prim);

		template <>
		void serializePrim<RectF>(std::ostream& oss, RefVariant prim);

		template <>
		void serializePrim<SizeF>(std::ostream& oss, RefVariant prim);

		template <>
		void serializePrim<SizeI>(std::ostream& oss, RefVariant prim);

		template <>
		void serializePrim<ColorB>(std::ostream& oss, RefVariant prim);

		template <>
		void serializePrim<ColorF>(std::ostream& oss, RefVariant prim);

		TEXT_SERIALIZE(RenderTraversalMask);

		template <class T>
		void serializeMap(std::ostream& oss, RefVariant value)
		{
			oss << "[" << std::endl;
			++level;
			padding(oss);

			size_t numElements = value.getValue<typename RemQual<T>::type>().size();
			typename RemQual<T>::type::iterator it = value.getValue<typename RemQual<T>::type>().begin();
			while (numElements != 0)
			{
				oss << "{ \"" << it->first << "\" : " << std::endl;
				++level;
				padding(oss);

				it->second->serialize(oss);
				it++;

				if (it == value.getValue<typename RemQual<T>::type>().end())
				{
					oss << "}" << std::endl;
					--level;
					break;
				}

				--level;
				oss << "}," << std::endl;
				padding(oss);
			}
			--level;
			padding(oss);
			oss << "]";
		}

		template <class T>
		void serializeVector(std::ostream& oss, RefVariant value)
		{
			oss << "[" << std::endl;
			++level;
			padding(oss);

			size_t numElements = value.getValue<typename RemQual<T>::type>().size();
			typename RemQual<T>::type::iterator it = value.getValue<typename RemQual<T>::type>().begin();

			while (numElements != 0)
			{
				const MetaData* meta = MetaCreator<T>::get();
				RefVariant ref = RefVariant(*it);
				ref.serialize(oss);

				it++;
				if (it == value.getValue<typename RemQual<T>::type>().end())
				{
					oss << std::endl;
					break;
				}

				oss << "," << std::endl;
				padding(oss);
			}

			--level;
			padding(oss);
			oss << "]";
		}

		template <class T>
		void serializeVectorNew(std::ostream& oss, RefVariant value)
		{
			oss << "[" << std::endl;
			++level;
			padding(oss);

			size_t numElements = value.getValue<typename RemQual<T>::type>().size();
			typename RemQual<T>::type::iterator it = value.getValue<typename RemQual<T>::type>().begin();

			while (numElements != 0)
			{
				(*it)->serialize(oss);

				it++;
				if (it == value.getValue<typename RemQual<T>::type>().end())
				{
					oss << std::endl;
					break;
				}

				oss << "," << std::endl;
				padding(oss);
			}

			--level;
			padding(oss);
			oss << "]";
		}
	}
}