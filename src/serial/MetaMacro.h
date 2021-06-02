#pragma once

#include "serial/RemQual.h"
#include "serial/MetaData.h"
#include "serial/Member.h"
#include "serial/NameGenerator.h"

#include <string>
#include <typeindex>
#include <typeinfo>

#define TEXT_SERIALIZATION_TYPE text

class MetaData;

#define META_TYPE(type_name) (MetaCreator<typename RemQual<type_name>::type>::get())
#define META_OBJECT(object_name) (MetaCreator<typename RemQual<decltype(object_name)>::type>::get())
#define META_STRING(str) (MetaManager::get(str));

#define DEFINE_META_CLASS_SHARED(type_name) \
	static RemQual<type_name>::type *nullCast(); \
	static MemberTyped<type_name>* addMember(const std::string& name, size_t offset, MetaData* data, size_t flags = 0, uint8 index = 0); \
	static MetaFunctionTyped<type_name>* addMetaFunction(const std::string& text, void(type_name::*func)()); \
	static uchar* create(); \
	virtual const MetaData* serialize(std::ostream& oss); \
	virtual const MetaData* getMetaData(); \
	static void registerMetaData(); \
	static void forceInclude();

#define DEFINE_META_CLASS(type_name) \
	DEFINE_META_CLASS_SHARED(type_name) \
	inline static MetaData* getDerivedMeta() { return nullptr; }

#define DEFINE_META_CLASS_DERIVED(type_name) \
	DEFINE_META_CLASS_SHARED(type_name) \
	inline static MetaData* getDerivedMeta() { return MetaCreator<RemQual<BASECLASS>::type>::get(); }

#define BASE_REFLECT_IMPL(type_name) \
	RemQual<type_name>::type* type_name::nullCast() \
	{ \
        static RemQual<type_name>::type* type_name_##null = nullptr; \
		return type_name_##null; \
	} \
	MemberTyped<type_name>* type_name::addMember(const std::string& name, size_t offset, MetaData *data, size_t flags, uint8 index) \
	{ \
		MemberTyped<type_name>* member = new MemberTyped<type_name>(name, offset, data, flags, (uint8) index); \
		MetaCreator<type_name>::get()->addMember(member); \
		return member; \
	} \
	MetaFunctionTyped<type_name>* type_name::addMetaFunction(const std::string& text, void(type_name::*func)()) \
	{ \
		MetaFunctionTyped<type_name>* meta_function = new MetaFunctionTyped<type_name>(func, text); \
		MetaCreator<type_name>::get()->addMetaFunction(meta_function); \
		return meta_function; \
	} \
	const MetaData* type_name::serialize(std::ostream& oss) \
	{ \
		const MetaData* meta = MetaCreator<type_name>::get(); \
		RefVariant ref = RefVariant(*this); \
		ref.serialize(oss); \
		return meta; \
	} \
	const MetaData* type_name::getMetaData() \
	{ \
		return MetaCreator<type_name>::get(); \
	} \
    template<> \
	void MetaCreator<RemQual<type_name>::type>::registerMetaData() \
	{ \
		type_name::registerMetaData(); \
	} \
	void type_name::forceInclude() \
	{ \
		assert(MetaCreator<type_name>::get()); \
	}

#define BASE_CREATE_IMPL(type_name) \
	uchar* type_name::create() \
	{ \
		logMetaCreation(#type_name); \
		return reinterpret_cast<uchar*>(new type_name()); \
	}

#define RESOURCE_CREATE_IMPL(type_name) \
	uchar* type_name::create() \
	{ \
		assert(false); \
		return nullptr; \
	}

#define FORCE_META_REGISTER(type_name) type_name::forceInclude()

#define BASE_REGISTER_META_DATA(type_name) \
	void type_name::registerMetaData() \
	{ \
		MetaCreator<RemQual<type_name>::type>::addDerived(type_name::getDerivedMeta());

#define END_META() }
#define BEGIN_META_CLASS(type_name) \
    MetaCreator<RemQual<type_name>::type> NAME_GENERATOR_META(type_name)(#type_name, sizeof(type_name)); \
	BASE_REFLECT_IMPL(type_name) \
	BASE_CREATE_IMPL(type_name) \
	BASE_REGISTER_META_DATA(type_name) \
		MetaData* meta = META_TYPE(type_name); \
		meta->setInitNew(type_name::create); \
		meta->setSerialize(TEXT_SERIALIZATION_TYPE::serializeMembers); \
		meta->setDeserialize(TEXT_SERIALIZATION_TYPE::deserializeMembers); \
		meta->setDeserializeNew(TEXT_SERIALIZATION_TYPE::deserializeMembersNew<RemQual<type_name>::type>); \
		meta->setCopy(copyMembers<RemQual<type_name>::type>); \
		MemberTyped<type_name>* memberPtr = nullptr;

#define BEGIN_META_RESOURCE(type_name) \
    MetaCreator<RemQual<type_name>::type> NAME_GENERATOR_META(type_name)(#type_name, sizeof(type_name)); \
	BASE_REFLECT_IMPL(type_name) \
	RESOURCE_CREATE_IMPL(type_name) \
	void type_name::registerMetaData() \
	{ \
		MetaData* meta = META_TYPE(type_name); \
		MetaCreator<RemQual<type_name>::type>::addDerived(type_name::getDerivedMeta()); \
		MetaData::InitFromStrFunc initFunc = std::bind(&ResourceFactory::loadResource<RemQual<type_name>::type>, ResourceFactory::getInstance(), std::placeholders::_1); \
		meta->setInitFromString(initFunc); \
		meta->setSerialize(TEXT_SERIALIZATION_TYPE::serializeResource); \
		meta->setDeserializeNew(TEXT_SERIALIZATION_TYPE::deserializeResource<RemQual<type_name>::type>); \
		meta->setCopy(copyResource<RemQual<type_name>::type>); \
		MemberTyped<type_name>* memberPtr = nullptr; 

#define DEFINE_META_PRIMITIVE(type_name, tag) \
    MetaCreator<RemQual<type_name>::type> NAME_GENERATOR_META(tag)(#type_name, sizeof(type_name)); \
    template<> \
	void MetaCreator<RemQual<type_name>::type>::registerMetaData() \
	{ \
		MetaData* meta = META_TYPE(type_name); \
		meta->setSerialize(TEXT_SERIALIZATION_TYPE::serializePrim<RemQual<type_name>::type>); \
		meta->setDeserialize(TEXT_SERIALIZATION_TYPE::deserializePrim<RemQual<type_name>::type>); \
		meta->setCopy(copyPrimitive<RemQual<type_name>::type>); \
	}

#define DEFINE_META_PRIMITIVE_ENUM(type_name, string_values) \
    MetaCreator<RemQual<type_name>::type> NAME_GENERATOR_META(type_name)(#type_name, sizeof(type_name), string_values); \
    template<> \
	void MetaCreator<RemQual<type_name>::type>::registerMetaData() \
	{ \
		MetaData* meta = META_TYPE(type_name); \
		meta->setSerialize(TEXT_SERIALIZATION_TYPE::serializePrim<int32>); \
		meta->setDeserialize(TEXT_SERIALIZATION_TYPE::deserializePrim<int32>); \
	}

#define DEFINE_META_VECTOR_NEW(type_name, container_name, tag_name) \
    MetaCreator<RemQual<type_name>::type> NAME_GENERATOR_META(tag_name)(#type_name, sizeof(type_name)); \
    template<> \
	void MetaCreator<RemQual<type_name>::type>::registerMetaData() \
	{ \
		MetaData* meta = META_TYPE(type_name); \
		meta->setSerialize(TEXT_SERIALIZATION_TYPE::serializeVectorNew<RemQual<type_name>::type>); \
		meta->setDeserialize(TEXT_SERIALIZATION_TYPE::deserializeVectorNew<RemQual<container_name>::type>); \
		meta->setCopy(copyVectorPtr<RemQual<container_name>::type>); \
	}


#define DEFINE_META_VECTOR(type_name, container_name, tag_name) \
    MetaCreator<RemQual<type_name>::type> NAME_GENERATOR_META(tag_name)(#type_name, sizeof(type_name)); \
    template<> \
	void MetaCreator<RemQual<type_name>::type>::registerMetaData() \
	{ \
		MetaData* meta = META_TYPE(type_name); \
		meta->setSerialize(TEXT_SERIALIZATION_TYPE::serializeVector<RemQual<type_name>::type>); \
		meta->setDeserialize(TEXT_SERIALIZATION_TYPE::deserializeVector<RemQual<container_name>::type>); \
		meta->setCopy(copyVector<RemQual<type_name>::type>); \
	}


#define DEFINE_META_MAP_NEW(type_name, first, second) \
    MetaCreator<RemQual<type_name>::type> NAME_GENERATOR_META(type_name)(#type_name, sizeof(type_name)); \
    template<> \
	void MetaCreator<RemQual<type_name>::type>::registerMetaData() \
	{ \
		MetaData* meta = META_TYPE(type_name); \
		meta->setSerialize(TEXT_SERIALIZATION_TYPE::serializeMap<RemQual<type_name>::type>); \
		meta->setDeserialize(TEXT_SERIALIZATION_TYPE::deserializeMapNew<first, RemQual<second>::type>); \
		meta->setCopy(copyMapPtr<first, RemQual<second>::type>); \
	}

#define DEFINE_META_TEMPLATE(type, temp) MetaCreator<type> \
	type##temp##__FILE__##__LINE__(#type, sizeof(type))

#define ADD_MEMBER(member) \
	memberPtr = addMember(#member, (size_t)(&(nullCast()->member)), META_OBJECT(nullCast()->member))

#define ADD_MEMBER_ENUM(member) \
	memberPtr = addMember(#member, (size_t)(&(nullCast()->member)), MetaCreator<size_t>::get(), Member::generateMemberFlags({ Member::MemberFlagEnum }))

#define ADD_MEMBER_WITH_FLAGS(member, ...) \
	memberPtr = addMember(#member, (size_t)(&(nullCast()->member)), META_OBJECT(nullCast()->member),  Member::generateMemberFlags({ __VA_ARGS__ }))

#define ADD_MEMBER_PTR_WITH_FLAGS(member, ...) \
	memberPtr = addMember(#member, (size_t)(&(nullCast()->member)), META_OBJECT(*(nullCast()->member).get()), Member::generateMemberFlags({ __VA_ARGS__ }))

#define ADD_MEMBER_STRING(member) \
	ADD_MEMBER_WITH_FLAGS(member, Member::MemberFlagString)

#define ADD_MEMBER_PTR(member) \
	ADD_MEMBER_PTR_WITH_FLAGS(member, Member::MemberFlagPointer)

#define ADD_MEMBER_RESOURCE(member) \
	ADD_MEMBER_PTR_WITH_FLAGS(member, Member::MemberFlagPointer, Member::MemberFlagResource)

#define ADD_COMBO_META(meta) \
    { \
        MetaData* meta_##meta = MetaCreator<meta>::get(); \
        memberPtr->addComboMeta(meta_##meta, #meta, #meta); \
    }

#define ADD_COMBO_META_LABEL(meta, label) \
    { \
        MetaData* meta_##meta = MetaCreator<meta>::get(); \
        memberPtr->addComboMeta(meta_##meta, label, #meta); \
    }

#define ADD_COMBO_META_INTEGER(flag, label) \
	memberPtr->addComboInteger(flag, label)

#define SET_MEMBER_MIN(value) \
	memberPtr->addValue(value, Member::MemberValueMin)

#define SET_MEMBER_MAX(value) \
	memberPtr->addValue(value, Member::MemberValueMax)

#define SET_MEMBER_DEFAULT(value) \
	memberPtr->addValue(value, Member::MemberValueDefault)

#define SET_MEMBER_DEFAULT_ENUM(value) \
	memberPtr->addValue((size_t) value, Member::MemberValueDefault)

#define SET_MEMBER_IGNORE_GUI() \
	memberPtr->setFlag(Member::MemberFlagIgnoreGUI)

#define SET_MEMBER_CONST() \
	memberPtr->setFlag(Member::MemberFlagConst)

#define SET_MEMBER_NO_SLIDER() \
	memberPtr->setFlag(Member::MemberFlagNoSlider)

#define SET_MEMBER_COLLAPSEABLE() \
	memberPtr->setFlag(Member::MemberFlagCollapseable)

#define SET_MEMBER_START_COLLAPSED() \
	memberPtr->setFlag(Member::MemberFlagStartCollapsed)

#define SET_MEMBER_IGNORE_SERIALIZATION() \
	memberPtr->setFlag(Member::MemberFlagIgnoreSerialization)

#define SET_MEMBER_CALLBACK_PRE(member_callback) \
	memberPtr->addCallback(member_callback, Member::MemberCallbackPre)

#define SET_MEMBER_CALLBACK_POST(member_callback) \
	memberPtr->addCallback(member_callback, Member::MemberCallbackPost)

#define ADD_MEMBER_OVERRIDE_FLAGS(member_name, ...) \
	meta->addOverrideFlags(#member_name, { __VA_ARGS__ })

#define SET_IGNORE_DERIVED_MEMBERS() meta->setIgnoreDerived()

#define ADD_META_FUNCTION(text, meta_function) addMetaFunction(text, meta_function)

namespace cs
{
	void logMetaCreation(const char* msg);
	void logMetaDeletion(const char* msg);
}
