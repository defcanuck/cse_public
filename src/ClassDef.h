
#pragma once

#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <mutex>

#include "serial/MetaMacro.h"
#include "serial/Serialize.h"
#include "serial/Serializable.h"

#include "global/EnumString.h"
#include "global/Allocator.h"

#if !defined(CS_IOS)
    #define CS_EDITOR 1
#endif

namespace cs
{
	
	template <class T>
	class AutoRelease
	{
	public:
		AutoRelease() : value(nullptr) { }
		~AutoRelease() { delete value; }

		T* operator->() { return this->value; }
		T* value;
	};

	template <class T>
	struct SharedList
	{
		~SharedList<T>()
		{
			this->clear();
		}

		inline int32 size() const
		{
			return static_cast<int32>(this->elements.size());
		}

		inline void clear()
		{
			for (auto& it : this->elements)
			{
				it = nullptr;
			}
			this->elements.clear();
		}

		std::shared_ptr<T>& at(int32 index)
		{
			assert(size_t(index) < this->elements.size());
			return this->elements[index];
		}

		inline void push_back(std::shared_ptr<T>& ptr)
		{
			this->elements.push_back(ptr);
		}

		std::vector<std::shared_ptr<T>> elements;
	};
}

#define CLASS_TYPE_DEFINITIONS(class_name) \
	class class_name; \
	typedef std::shared_ptr<class_name> class_name##Ptr;


#define REFLECT_CLASS(class_name) \
	public: \
		DEFINE_META_CLASS(class_name) \
	private:

#define REFLECT_CLASS_DERIVED(class_name) \
	public: \
		DEFINE_META_CLASS_DERIVED(class_name) \
	private:

#define CLASS_DEFINITION(class_name) \
	CLASS_TYPE_DEFINITIONS(class_name) \
	class class_name \
		{

#define END_CLASS() };
	
#define CLASS_DEFINITION_DERIVED(class_name, derived_name) \
	CLASS_TYPE_DEFINITIONS(class_name) \
	class class_name : public derived_name \
	{ \
		typedef derived_name BASECLASS; \
	public: \
		static std::shared_ptr<derived_name> castToBase(std::shared_ptr<RemQual<class_name>::type>& ptr) { return std::static_pointer_cast<derived_name>(ptr); }


#define CLASS_DEFINITION_DERIVED2(class_name, derived_name1, derived_name2) \
	CLASS_TYPE_DEFINITIONS(class_name) \
	class class_name : public derived_name1, public derived_name2 \
		{ \
		typedef derived_name1 BASECLASS;

#define CLASS_DEFINITION_REFLECT(class_name) \
	CLASS_TYPE_DEFINITIONS(class_name) \
	class class_name : public Serializable \
	{ \
		typedef class_name THISCLASS; \
		REFLECT_CLASS(class_name)


#define CLASS_DEFINITION_DERIVED_REFLECT(class_name, derived_name) \
	CLASS_TYPE_DEFINITIONS(class_name) \
	class class_name : public derived_name \
	{ \
		typedef class_name THISCLASS; \
		typedef derived_name BASECLASS; \
		public: \
		static std::shared_ptr<derived_name> castToBase(std::shared_ptr<RemQual<class_name>::type>& ptr) { return std::static_pointer_cast<derived_name>(ptr); } \
		REFLECT_CLASS_DERIVED(class_name)


#define CREATE_CLASS(class_name, ...) \
std::make_shared<class_name>(__VA_ARGS__)
//std::allocate_shared<class_name, cs::Allocator<class_name>>(cs::Allocator<class_name>(), __VA_ARGS__)

#define CREATE_CLASS_CAST(cast_name, class_name, ...) \
std::static_pointer_cast<cast_name>(std::make_shared<class_name>(__VA_ARGS__))
//std::static_pointer_cast<cast_name>(std::allocate_shared<class_name, cs::Allocator<class_name>>(cs::Allocator<class_name>(), __VA_ARGS__))

#define SAFE_FREE(ptr) \
if (ptr) { delete ptr; ptr = nullptr; }

