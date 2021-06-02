#include "PCH.h"

#include "serial/MetaData.h"
#include "serial/RefVariant.h"
#include "serial/Serialize.h"

#include "ui/widget/UIWidget.h"

#include <cassert>

namespace cs
{
	void MetaData::addMember(Member *member)
	{
		assert(member);
		assert(this->members.find(member->getName()) == this->members.end());
		member->setIndex(this->members.size());
		this->members[member->getName()] = member;
	}

	void MetaData::copyData(void* dest, const void* src) const
	{
		memcpy(dest, src, size);
	}

	void MetaData::copy(void* dest, void* src) const
	{
		if (this->cpFunc)
			this->cpFunc(dest, src);
	}

	void MetaData::erase(void* data) const
	{
		delete[] reinterpret_cast<char *>(data);
		data = nullptr;
	}

	void *MetaData::newCopy(const void* src) const
	{
		void *data = new char[size];
		memcpy(data, src, size);
		return data;
	}

	void *MetaData::create(void) const
	{
		return new char[size];
	}

	void MetaData::setSerialize(serializeFunc func)
	{
		SerializeFunc fn = std::bind(func, std::placeholders::_1, std::placeholders::_2);
		this->setSerialize(fn);
	}

	void MetaData::setSerialize(SerializeFunc& fn)
	{
		this->serialFunc = fn;
	}

	void MetaData::setDeserialize(deserializeFunc func)
	{
		DeserializeFunc fn = std::bind(func, std::placeholders::_1, std::placeholders::_2);
		this->setDeserialize(fn);
	}

	void MetaData::setDeserialize(DeserializeFunc& fn)
	{
		this->deserialFunc = fn;
	}

	void MetaData::setCopy(copyFunc func)
	{
		CopyFunc fn = std::bind(func, std::placeholders::_1, std::placeholders::_2);
		this->setCopy(fn);
	}

	void MetaData::setCopy(CopyFunc& fn)
	{
		this->cpFunc = fn;
	}

	void MetaData::setDeserializeNew(deserializeNewFunc func)
	{
		DeserializeNewFunc fn = std::bind(func, std::placeholders::_1, std::placeholders::_2);
		this->setDeserializeNew(fn);
	}

	void MetaData::setDeserializeNew(DeserializeNewFunc& fn)
	{
		this->deserialNewFunc = fn;
	}

	void MetaData::serialize(std::ostream& os, RefVariant var) const
	{
		if (this->serialFunc)
			this->serialFunc(os, var);
	}

	void MetaData::deserialize(JsonValue& value, RefVariant var) const
	{
		if (this->deserialFunc)
			this->deserialFunc(value, var);
	}

	void MetaData::setInitNew(InitNewFunc& fn)
	{
		this->initFunc = fn;
	}

	void MetaData::setInitNew(initNewFunc func)
	{
		InitNewFunc fn = std::bind(func);
		this->setInitNew(fn);
	}

	void MetaData::setInitFromString(InitFromStrFunc& fn)
	{
		this->initStrFunc = fn;
	}

	void MetaData::setInitFromString(initFromStrFunc func)
	{
		InitFromStrFunc fn = std::bind(func, std::placeholders::_1);
		this->setInitFromString(fn);
	}

	std::shared_ptr<Resource> MetaData::createFromString(const std::string& name) const
	{
		static std::shared_ptr<Resource> kEmptyPtr;
		if (this->initStrFunc)
			return this->initStrFunc(name);

		return kEmptyPtr;
	}

	uchar* MetaData::createNew() const
	{
		if (this->initFunc)
			return this->initFunc();

		return nullptr;
	}

	const MetaData* MetaData::deserializeNew(JsonValue& value, RefVariant var) const
	{
		if (this->deserialNewFunc)
			return this->deserialNewFunc(value, var);

		return nullptr;
	}

	int32 MetaData::convertToEnum(const std::string& str) const
	{
		for (size_t i = 0; i < toString.size(); i++)
		{
			if (this->toString[i] == str)
				return (int32)i;
		}
		return -1;
	}

	const std::string& MetaData::convertToString(int32 idx) const
	{
		const static std::string kUnknownConersion = "error";
		if (idx < 0 || size_t(idx) >= this->toString.size())
			return kUnknownConersion;

		return this->toString[idx];
	}

	const Member* MetaData::getMember(const std::string& name) const
	{
		MemberMap::const_iterator it;
		if ((it = this->members.find(name)) != this->members.end())
			return it->second;

		if (this->derived)
			return this->derived->getMember(name);

		return nullptr;
	}

	void MetaManager::registerMeta(const MetaData* instance)
	{
		assert(instance != nullptr);
		const std::string& key = instance->getName();
		const std::type_index index = instance->getTypeIndex();
		this->map[key] = instance;
		this->indexMap[index.hash_code()] = instance;
	}

	const MetaData* MetaManager::get(const std::string& name)
	{
		MetaMap::iterator it;
		if ((it = this->map.find(name)) != this->map.end())
			return it->second;

		return nullptr;
	}

	const MetaData* MetaManager::get(size_t index)
	{
		MetaIndexMap::iterator it;
		if ((it = this->indexMap.find(index)) != this->indexMap.end())
			return it->second;

		return nullptr;
	}

	MetaManager::MetaMap& MetaManager::getMap()
	{
		return this->map;
	}

	void MetaData::getCompleteMemberMap(MemberMap& map) const
	{
		map.insert(this->members.begin(), this->members.end());
		if (this->derived && !this->ignoreDerived)
			this->derived->getCompleteMemberMap(map);
	}

	void MetaData::getAllMembers(MemberList& vec) const
	{
		struct
		{
			bool operator()(const Member* a, const Member* b)
			{
				return a->getIndex() < b->getIndex();
			}
		} MemberSort;

		// get the derived members first
		MemberList derivedMembers;
		if (this->derived && !this->ignoreDerived)
		{
			this->derived->getAllMembers(derivedMembers);
		}

		// get + sort the present members
		for (auto it : this->members)
		{
			vec.push_back(it.second);
		}
		std::sort(vec.begin(), vec.end(), MemberSort);

		// add derived members to the beginning of the list
		if (derivedMembers.size() > 0)
		{
			vec.insert(vec.begin(), derivedMembers.begin(), derivedMembers.end());
		}
	}

	void MetaData::getAllTypes(std::vector<std::type_index>& types) const
	{
		types.push_back(this->type_index);
		if (this->derived)
			this->derived->getAllTypes(types);
	}

	void MetaData::addMetaFunction(MetaFunction* meta_func)
	{
		this->metaFunctions.push_back(meta_func);
	}

	void MetaData::addOverrideFlags(const std::string& member_name, std::vector<Member::MemberFlags> flags, void* vptr)
	{
		MemberOverrideFlags* override_flags = nullptr;
		if (vptr)
		{
			ObjectUniqueOverrideFlag::iterator it = this->objectOverrideFlags.find((size_t)vptr);
			if (it != this->objectOverrideFlags.end())
			{
				override_flags = &it->second;
			}
			else
			{
				size_t key = size_t(vptr);
				this->objectOverrideFlags[key] = MemberOverrideFlags();
				override_flags = &this->objectOverrideFlags[key];
			}
		}

		if (!override_flags)
		{
			override_flags = &this->overrideFlags;
		}

		MemberOverrideFlags::iterator it = (*override_flags).find(member_name);
		if (it == (*override_flags).end())
		{
			(*override_flags)[member_name] = Member::generateMemberFlags(flags);
			return;
		}

		it->second = it->second | Member::generateMemberFlags(flags);
	}

	void MetaData::removeOverrideFlags(const std::string& member_name, std::vector<Member::MemberFlags> flags, void* vptr)
	{
		MemberOverrideFlags* override_flags = nullptr;
		if (vptr)
		{
			ObjectUniqueOverrideFlag::iterator it = this->objectOverrideFlags.find((size_t)vptr);
			if (it != this->objectOverrideFlags.end())
			{
				override_flags = &it->second;
			}
		}

		if (!override_flags)
		{
			override_flags = &this->overrideFlags;
		}

		MemberOverrideFlags::iterator it = (*override_flags).find(member_name);
		if (it == (*override_flags).end())
		{
			return;
		}
		it->second &= ~Member::generateMemberFlags(flags);
	}

	void MetaData::clearObjectOverrideFlags(void* vptr)
	{
		if (vptr)
		{
			ObjectUniqueOverrideFlag::iterator it = this->objectOverrideFlags.find((size_t)vptr);
			if (it != this->objectOverrideFlags.end())
			{
				this->objectOverrideFlags.erase(it);
			}
		}
	}

	void MetaData::clearAllObjectOverrideFlags()
	{
		this->objectOverrideFlags.clear();
	}

	bool MetaData::hasObjectOverride(void* vptr) const
	{
		return this->objectOverrideFlags.find(size_t(vptr)) != this->objectOverrideFlags.end();
	}

	bool MetaData::isOverrideSet(const std::string& member_name, Member::MemberFlags flag, void* vptr) const
	{
		bool isOverride = true;
		const MemberOverrideFlags* override_flags = nullptr;
		if (vptr)
		{
			ObjectUniqueOverrideFlag::const_iterator it = this->objectOverrideFlags.find((size_t)vptr);
			if (it != this->objectOverrideFlags.end())
			{
				override_flags = &it->second;
			}
		}

		if (!override_flags)
		{
			override_flags = &this->overrideFlags;
		}

		MemberOverrideFlags::const_iterator it = (*override_flags).find(member_name);
		if (it == (*override_flags).end())
			return false;

		return (it->second & (size_t(0x1) << size_t(flag))) > 0;
	}

	bool MetaData::isOverrideSet(const Member* member, Member::MemberFlags flag, void* vptr) const
	{
		const std::string& member_name = member->getName();
		return this->isOverrideSet(member_name, flag, vptr);
	}
}