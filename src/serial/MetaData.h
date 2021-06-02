#pragma once


#include "global/Values.h"
#include "global/Callback.h"
#include "global/Utils.h"
#include "serial/Member.h"
#include "serial/JSON.h"
#include "serial/Serializable.h"

#include "os/LogManager.h"
#include "global/Singleton.h"

#include <string>
#include <map>
#include <vector>
#include <functional>
#include <unordered_map>
#include <typeindex>

// #define LOG_COPY_OP 1

#if defined(LOG_COPY_OP)
	#define CopyLog(...) log::info(__VA_ARGS__)
#else
	#define CopyLog(...) void(0)
#endif

namespace cs
{
	class RefVariant;
	class Resource;
	class UIWidget;

	typedef void(*serializeFunc)(std::ostream&, RefVariant);
	typedef void(*deserializeFunc)(JsonValue, RefVariant);
	typedef const MetaData*(*deserializeNewFunc)(JsonValue, RefVariant);
	typedef uchar*(*initNewFunc)();
	typedef void(*copyFunc)(void*, void*);
	typedef std::shared_ptr<Resource>(*initFromStrFunc)(const std::string&);

	class MetaFunction
	{
	public:

		MetaFunction(const std::string& t)
			: text(t) { }

		virtual CallbackPtr createCallback(void* vptr) = 0;

		std::string text;
	
	};

	template <class Caller>
	class MetaFunctionTyped : public MetaFunction
	{
	public:
		typedef void(Caller::*FunctionPtr)();

		MetaFunctionTyped(FunctionPtr p, const std::string& t)
			: MetaFunction(t)
			, ptr(p) { }
		
		virtual CallbackPtr createCallback(void* vptr)
		{
			return createCallbackArg0(ptr, (Caller*) vptr);
		}

	private:

		FunctionPtr ptr;
	};

	class MetaData
	{
		
	public:

		typedef std::vector<MetaFunction*> MetaFunctions;
		typedef std::map<std::string, const Member*> MemberMap;
		typedef std::vector<const Member*> MemberList;
		typedef std::map<std::string, size_t> MemberOverrideFlags;

		typedef std::function<void(std::ostream&, RefVariant)> SerializeFunc;
		typedef std::function<void(JsonValue, RefVariant)> DeserializeFunc;
		typedef std::function<void(void* dst, void* src)> CopyFunc;
		typedef std::function<const MetaData*(JsonValue, RefVariant)> DeserializeNewFunc;
		typedef std::function<uchar*()> InitNewFunc;
		typedef std::function<std::shared_ptr<Resource>(const std::string&)> InitFromStrFunc;

		struct Dummy { };

		MetaData()
			: initialized(false)
			, serialFunc(nullptr)
			, deserialFunc(nullptr)
			, deserialNewFunc(nullptr)
			, initFunc(nullptr)
			, initStrFunc(nullptr)
			, cpFunc(nullptr)
			, name("")
			, size(-1)
			, type_index(typeid(Dummy))
			, derived(nullptr)
			, ignoreDerived(false)
		{ 
		}

		MetaData(const std::string& n, int32 sz)
			: initialized(false)
			, serialFunc(nullptr)
			, deserialFunc(nullptr)
			, deserialNewFunc(nullptr)
			, initFunc(nullptr)
			, initStrFunc(nullptr)
			, cpFunc(nullptr)
			, name(n)
			, size(sz)
			, type_index(typeid(Dummy))
			, derived(nullptr)
			, ignoreDerived(nullptr)
		{ 
		}

		void init(const std::string& n, int32 sz, const StringList& sstr, std::type_index type_index)
		{
			this->name = n;
			this->size = sz;
			this->toString = sstr;
			this->type_index = type_index;
			this->initialized = true;
		}

		const bool getInitialized() const { return this->initialized; }

		const std::string& getName() const { return this->name; }
		int32 getSize() const { return this->size; }
		void addMember(Member *member);
		void erase(void* data) const;

		void copy(void* dest, void* src) const;
		void copyData(void* dest, const void* src) const;
		void* newCopy(const void* src) const;
		void* create(void) const;

		bool hasSerialization() const { return this->serialFunc != nullptr; }
		void setSerialize(SerializeFunc& fn);
		void setSerialize(serializeFunc func = nullptr);

		bool hasDeserialization() const { return this->deserialFunc != nullptr; }
		void setDeserialize(DeserializeFunc& fn);
		void setDeserialize(deserializeFunc func = nullptr);

		void setDeserializeNew(DeserializeNewFunc& fn);
		void setDeserializeNew(deserializeNewFunc func = nullptr);

		void setInitNew(InitNewFunc& fn);
		void setInitNew(initNewFunc func = nullptr);

		void setInitFromString(InitFromStrFunc& fn);
		void setInitFromString(initFromStrFunc func = nullptr);

		void setCopy(CopyFunc& fn);
		void setCopy(copyFunc func = nullptr);

		uchar* createNew() const;
		std::shared_ptr<Resource> createFromString(const std::string& name) const;

		void serialize(std::ostream& os, RefVariant variant) const;
		void deserialize(JsonValue& value, RefVariant variant) const;
		const MetaData* deserializeNew(JsonValue& value, RefVariant var) const;

		size_t getNumMembers() const { return this->members.size(); }
		const MemberMap& getMembers() const { return this->members; }
		const Member* getMember(const std::string& name) const;

		void getAllMembers(MemberList& map) const;
		void getCompleteMemberMap(MemberMap& map) const;

		const StringList& getToStringArray() const { return this->toString; }
		bool hasToStringValue() const { return this->toString.size() > 0; }

		const std::string& convertToString(int32 idx) const;
		int32 convertToEnum(const std::string& str) const;

		const std::type_index& getTypeIndex() const { return this->type_index; }

		void setDerived(MetaData* meta) { this->derived = meta; }
		void getAllTypes(std::vector<std::type_index>& types) const;

		void addMetaFunction(MetaFunction* meta_func);
		const MetaFunctions& getMetaFunctions() const { return this->metaFunctions; }

		void addOverrideFlags(const std::string& member_name, std::vector<Member::MemberFlags> flags, void* vptr = nullptr);
		void removeOverrideFlags(const std::string& member_name, std::vector<Member::MemberFlags> flags, void* vptr = nullptr);
		
		bool hasObjectOverride(void* vptr) const;
		void clearObjectOverrideFlags(void* vptr);
		void clearAllObjectOverrideFlags();

		bool isOverrideSet(const std::string& member_name, Member::MemberFlags flag, void* vptr = nullptr) const;
		bool isOverrideSet(const Member* member, Member::MemberFlags flag, void* vptr = nullptr) const;

		void setIgnoreDerived() { this->ignoreDerived = true; }

	private:

		bool initialized;

		SerializeFunc serialFunc;
		DeserializeFunc deserialFunc;
		DeserializeNewFunc deserialNewFunc;
		InitNewFunc initFunc;
		InitFromStrFunc initStrFunc;
		CopyFunc cpFunc;
	
		std::string name;
		int32 size;
		StringList toString;
	
		MemberMap members;
		MemberOverrideFlags overrideFlags;

		typedef std::map<size_t, MemberOverrideFlags> ObjectUniqueOverrideFlag;
		ObjectUniqueOverrideFlag objectOverrideFlags;

		std::type_index type_index;
		MetaData* derived;
		bool ignoreDerived;

		MetaFunctions metaFunctions;
	};

    class MetaManager : public Singleton<MetaManager>
    {
    public:
        typedef std::unordered_map<std::string, const MetaData*> MetaMap;
        typedef std::unordered_map<size_t, const MetaData*> MetaIndexMap;
        
        void registerMeta(const MetaData* instance);
        const MetaData* get(const std::string& name);
        const MetaData* get(size_t index);
        
        MetaMap& getMap();
        
    private:
        
        MetaMap map;
        MetaIndexMap indexMap;
        
    };
    
	template <class T>
	class MetaCreator
	{

	public:

		static const StringList kEmptyPropertyList;
		static bool initialized;
		//typedef ;

	public:
		MetaCreator(const std::string& name, int32 size, const StringList& toStr = kEmptyPropertyList) 
		{
            //log::info("Initializing meta for ", name);
			MetaCreator::init(name, size, toStr);
            //log::info("End meta initialize for ", name);
		}

		static void init(const std::string& name, int32 size, const StringList& toStr = kEmptyPropertyList)
		{
			MetaData* meta = MetaCreator<T>::get();
			meta->init(name, size, toStr, std::type_index(typeid(T)));
			MetaManager::getInstance()->registerMeta(meta);
			registerMetaData();
			MetaCreator<T>::initialized = true;
		}

		static void addDerived(MetaData* meta)
		{
			if (meta)
				MetaCreator<T>::get()->setDerived(meta);
		}

		static MetaData* get(bool fromInit = false)
		{
			//assert(MetaCreator<T>::initialized || (!MetaCreator<T>::initialized && fromInit));
			static MetaData instance;
			return &instance;
		}

		static T* nullCast()
		{
			return reinterpret_cast<T*>(nullptr);
		}

		static void registerMetaData();

	private:

		
	};

	template <typename T>
	const StringList MetaCreator<T>::kEmptyPropertyList;

	template <typename T>
	bool MetaCreator<T>::initialized = false;

	template <class T>
	void copyPrimitive(void* dst, void* src)
	{

#if defined(LOG_COPY_OP)
		const MetaData* metadata = MetaCreator<T>::get();
		CopyLog("Copy primitive ", metadata->getName(), " with size ", metadata->getSize());
#endif

		T* cast_dst = (T*) dst;
		T* cast_src = (T*) src;
		*cast_dst = *cast_src;
	}

	template <class T>
	void copyMembers(void* dst, void* src)
	{
		
		const MetaData* metadata = MetaCreator<T>::get();
		MetaData::MemberList members;
		metadata->getAllMembers(members);

		for (auto& it : members)
		{
			const Member* member = it;
			size_t offset = member->getOffset();
			const MetaData* member_metadata = member->getMetaData();
			
			if (member->getIsPointer())
			{
				std::shared_ptr<Serializable>* src_ptr = reinterpret_cast<std::shared_ptr<Serializable>*>(PTR_ADD(src, offset));
				std::shared_ptr<Serializable>* dst_ptr = reinterpret_cast<std::shared_ptr<Serializable>*>(PTR_ADD(dst, offset));
				
				// dst_ptr is guaranteed to be valid - src_ptr not so much
				if (!(*src_ptr).get())
					continue;
				const MetaData* src_meta = (*src_ptr)->getMetaData();

				(*dst_ptr) = std::shared_ptr<Serializable>(reinterpret_cast<Serializable*>(src_meta->createNew()));
				if (member->getIsResource())
				{
					// resources can be shared_ptr assigned
					// src_meta->copy((void*)dst_ptr, (void*)src_ptr);

					*dst_ptr = *src_ptr;
				}
				else
				{
					// pointers can be copied by the contents of the shared_ptr (since they are infact two distinct objects)
					src_meta->copy((void*)dst_ptr->get(), (void*)src_ptr->get());
					CopyLog("Copying ", member->getName(), " with new ", src_meta->getName(), " member of type[", member_metadata->getName(), "]");
				}
				
			}
			else if (member->getIsString())
			{
				std::string* str_dst = (std::string*) PTR_ADD(dst, offset);
				std::string* str_src = (std::string*) PTR_ADD(src, offset);
				*str_dst = *str_src;
			}
			else
			{
				member_metadata->copy(PTR_ADD(dst, offset), PTR_ADD(src, offset));
				CopyLog("Copying ", member->getName(), " member of type [", member_metadata->getName(), "]");
			}
		}
	}

	template <class K, class V>
	void copyMapPtr(void* dst, void* src)
	{
		std::map<K, std::shared_ptr<V>>* dst_map = reinterpret_cast<std::map<K, std::shared_ptr<V>>*>(dst);
		std::map<K, std::shared_ptr<V>>* src_map = reinterpret_cast<std::map<K, std::shared_ptr<V>>*>(src);

		for (auto& it : (*src_map))
		{
			const K& key = it.first;
			std::shared_ptr<V>& src_item = it.second;
			std::shared_ptr<Serializable> src_serial = std::static_pointer_cast<Serializable>(src_item);
			const MetaData* src_metadata = src_serial->getMetaData();

			std::shared_ptr<V> dst_item = std::shared_ptr<V>(reinterpret_cast<V*>(src_metadata->createNew()));
			src_metadata->copy(dst_item.get(), src_item.get());
			(*dst_map)[key] = dst_item;
			CopyLog("Copying ", src_metadata->getName(), ", size of map is ", (*dst_map).size());
		}
	}

	template <class T>
	void copyVector(void* dst, void* src)
	{
		T* dst_vec = reinterpret_cast<T*>(dst);
		T* src_vec = reinterpret_cast<T*>(src);
	
		size_t sz = (*src_vec).size();
		if (sz > 0)
		{
			(*dst_vec).resize(sz);
			memcpy((*dst_vec).data(), (*src_vec).data(), sz * sizeof((*src_vec)[0]));
		}
	}

	template <class T>
	void copyResource(void* dst, void* src)
	{
		std::shared_ptr<T>* dst_ptr = reinterpret_cast<std::shared_ptr<T>*>(dst);
		std::shared_ptr<T>* src_ptr = reinterpret_cast<std::shared_ptr<T>*>(src);

        /* is this necessary?
		std::shared_ptr<Resource>& resource = std::static_pointer_cast<Resource>(*src_ptr);
		const std::string& name = resource->getName();
        */
        
		// Assign the resource over :)
		(*dst_ptr) = (*src_ptr);
	}

	template <class T>
	void copyVectorPtr(void* dst, void* src)
	{
		std::vector<std::shared_ptr<T>>* dst_vec = reinterpret_cast<std::vector<std::shared_ptr<T>>*>(dst);
		std::vector<std::shared_ptr<T>>* src_vec = reinterpret_cast<std::vector<std::shared_ptr<T>>*>(src);

		for (size_t i = 0; i < (*src_vec).size(); i++)
		{
			std::shared_ptr<T>& src_item = (*src_vec)[i];
			std::shared_ptr<Serializable> src_serial = std::static_pointer_cast<Serializable>(src_item);
			const MetaData* src_metadata = src_serial->getMetaData();

			std::shared_ptr<T> dst_item = std::shared_ptr<T>(reinterpret_cast<T*>(src_metadata->createNew()));
			src_metadata->copy(dst_item.get(), src_item.get());
			(*dst_vec).push_back(dst_item);
			CopyLog("Copying ", src_metadata->getName(), ", size of vector is ", (*dst_vec).size());
		}
	}
}
