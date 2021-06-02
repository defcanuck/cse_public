#pragma once

#include "scripting/LuaPtr.h"
#include "scripting/LuaConverter.h"

#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>

#define TYPE_LUA_ENUM(enum_name) struct enum_name##_STRUCT { };
#define DEF_LUA_ENUM(enum_name) class_<enum_name##_STRUCT>(#enum_name)

#define DEF_LUA_CLASS_SHARED(class_name) class_<class_name, std::shared_ptr<class_name> >(#class_name)
#define DEF_LUA_CLASS_SHARED_DERIVED(class_name, parent_class) class_<class_name, parent_class, std::shared_ptr<class_name> >(#class_name)
#define DEF_LUA_CLASS(class_name) class_<class_name>(#class_name)


#define BIND_LUA_CLASS(T) \
	*createClass<T>()

#define PROTO_LUA_FUNCTION_DEF(T) \
	template<> luabind::detail::class_base* createClass<T>()

#define PROTO_LUA_CLASS(T) \
	PROTO_LUA_FUNCTION_DEF(T)

#define BEGIN_DEFINE_LUA_ENUM(T) \
	struct T##_STRUCT { }; \
	PROTO_LUA_FUNCTION_DEF(T) { \
		class_<T##_STRUCT>* enum_def = new class_<T##_STRUCT>(#T); \
		(*enum_def) 

#define END_DEFINE_LUA_ENUM() \
	; return reinterpret_cast<luabind::detail::class_base*>(enum_def); }

#define BEGIN_DEFINE_LUA_CLASS(T) \
	PROTO_LUA_FUNCTION_DEF(T) { \
		class_<T>* class_def = new class_<T>(#T); \
		(*class_def) 

#define BEGIN_DEFINE_LUA_CLASS_DERIVED(T0, T1) \
	PROTO_LUA_FUNCTION_DEF(T0) { \
		class_<T0, T1>* class_def = new class_<T0, T1>(#T0); \
		(*class_def) 
	
#define BEGIN_DEFINE_LUA_CLASS_RENAMED(T, LUA_NAME) \
	PROTO_LUA_FUNCTION_DEF(T) { \
		class_<T>* class_def = new class_<T>(#LUA_NAME); \
		(*class_def) 

#define BEGIN_DEFINE_LUA_CLASS_SHARED(T) \
	PROTO_LUA_FUNCTION_DEF(T) { \
		class_<T, std::shared_ptr<T>>* class_def = new class_<T, std::shared_ptr<T>>(#T); \
		(*class_def) 

#define BEGIN_DEFINE_LUA_CLASS_SHARED_RENAMED(T, LUA_NAME) \
	PROTO_LUA_FUNCTION_DEF(T) { \
		class_<T, std::shared_ptr<T>>* class_def = new class_<T, std::shared_ptr<T>>(#LUA_NAME); \
		(*class_def) 

#define BEGIN_DEFINE_LUA_CLASS_DERIVED_SHARED(T0, T1) \
	PROTO_LUA_FUNCTION_DEF(T0) { \
		class_<T0, T1, std::shared_ptr<T0>>* class_def = new class_<T0, T1, std::shared_ptr<T0>>(#T0); \
		(*class_def).scope \
		[ \
			def("castToBase", &T0::castToBase) \
		]

#define BEGIN_DEFINE_LUA_CLASS_DERIVED_LUA(T0, T1) \
	PROTO_LUA_FUNCTION_DEF(T0) { \
		class_<T0, T1, std::shared_ptr<T0>>* class_def = new class_<T0, T1, std::shared_ptr<T0>>(#T0); \
		(*class_def)

#define END_DEFINE_LUA_CLASS() \
	; return reinterpret_cast<luabind::detail::class_base*>(class_def); }

#define ADD_META_BITSET(name, type, max_value) \
	BEGIN_DEFINE_LUA_CLASS(name) \
		.def(constructor<>()) \
		.def(constructor<const size_t&>()) \
		.def(constructor<const BitMask<type, max_value>&>()) \
		.def("set", (void(BitMask<type, max_value>::*)(const type&)) &BitMask<type, max_value>::set) \
		.def("unset", (void(BitMask<type, max_value>::*)(const type&)) &BitMask<type, max_value>::unset) \
		.def("test", (void(BitMask<type, max_value>::*)(const type&)) &BitMask<type, max_value>::test) \
		.def("toggle", (void(BitMask<type, max_value>::*)(const type&)) &BitMask<type, max_value>::toggle) \
		.def("clear", &BitMask<type, max_value>::clear) \
	END_DEFINE_LUA_CLASS()

#define ADD_SHARED_LIST(compname) \
	BEGIN_DEFINE_LUA_CLASS(compname) \
		.def(constructor<>()) \
		.def("size", &compname::size) \
		.def("push_back", &compname::push_back) \
		.def("clear", &compname::clear) \
		.def("at", &compname::at) \
		.def_readwrite("elements", &compname::elements, return_stl_iterator) \
	END_DEFINE_LUA_CLASS()

#define DEFINE_INHERITED_VALUES(name, derived) \
	struct Lua##name##BaseCast \
	{ \
		static name##Ptr to##name(const name##Ptr& ptr) { return std::dynamic_pointer_cast<name>(ptr); } \
		static derived##Ptr to##derived(const name##Ptr& ptr) { return std::dynamic_pointer_cast<derived>(ptr); } \
	};

#define SET_INHERITED_VALUES(name, derived) \
	.scope \
	[ \
		def("to"#derived, &Lua##name##BaseCast::to##derived), \
		def("to"#name, &Lua##name##BaseCast::to##name) \
	]

#define DEFINE_DERIVED_VALUES_1(name, derived1) \
	struct Lua##name##DerivedCast \
	{ \
		static derived1##Ptr to##derived1(const name##Ptr& ptr) { return std::dynamic_pointer_cast<derived1>(ptr); } \
	};

#define SET_DERIVED_VALUES_1(name, derived1) \
	.scope \
	[ \
		def("to"#derived1, &Lua##name##DerivedCast::to##derived1) \
	]

#define DEFINE_DERIVED_VALUES_2(name, derived1, derived2) \
	struct Lua##name##DerivedCast \
	{ \
		static derived1##Ptr to##derived1(const name##Ptr& ptr) { return std::dynamic_pointer_cast<derived1>(ptr); } \
		static derived2##Ptr to##derived2(const name##Ptr& ptr) { return std::dynamic_pointer_cast<derived2>(ptr); } \
	};

#define SET_DERIVED_VALUES_2(name, derived1, derived2) \
	.scope \
	[ \
		def("to"#derived1, &Lua##name##DerivedCast::to##derived1), \
		def("to"#derived2, &Lua##name##DerivedCast::to##derived2) \
	]

#define DEFINE_DERIVED_VALUES_3(name, derived1, derived2, derived3) \
	struct Lua##name##DerivedCast \
	{ \
		static derived1##Ptr to##derived1(const name##Ptr& ptr) { return std::dynamic_pointer_cast<derived1>(ptr); } \
		static derived2##Ptr to##derived2(const name##Ptr& ptr) { return std::dynamic_pointer_cast<derived2>(ptr); } \
		static derived3##Ptr to##derived3(const name##Ptr& ptr) { return std::dynamic_pointer_cast<derived3>(ptr); } \
	};

#define SET_DERIVED_VALUES_3(name, derived1, derived2, derived3) \
	.scope \
	[ \
		def("to"#derived1, &Lua##name##DerivedCast::to##derived1), \
		def("to"#derived2, &Lua##name##DerivedCast::to##derived2), \
		def("to"#derived3, &Lua##name##DerivedCast::to##derived3) \
	]

#define DEFINE_DERIVED_VALUES_4(name, derived1, derived2, derived3, derived4) \
	struct Lua##name##DerivedCast \
		{ \
		static derived1##Ptr to##derived1(const name##Ptr& ptr) { return std::dynamic_pointer_cast<derived1>(ptr); } \
		static derived2##Ptr to##derived2(const name##Ptr& ptr) { return std::dynamic_pointer_cast<derived2>(ptr); } \
		static derived3##Ptr to##derived3(const name##Ptr& ptr) { return std::dynamic_pointer_cast<derived3>(ptr); } \
		static derived4##Ptr to##derived4(const name##Ptr& ptr) { return std::dynamic_pointer_cast<derived4>(ptr); } \
	};

#define SET_DERIVED_VALUES_4(name, derived1, derived2, derived3, derived4) \
	.scope \
	[ \
		def("to"#derived1, &Lua##name##DerivedCast::to##derived1), \
		def("to"#derived2, &Lua##name##DerivedCast::to##derived2), \
		def("to"#derived3, &Lua##name##DerivedCast::to##derived3), \
		def("to"#derived4, &Lua##name##DerivedCast::to##derived4) \
	]