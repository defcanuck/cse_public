#pragma once

#include "global/Values.h"
#include "global/BitMask.h"

#include <string>
#include <memory>
#include <vector>
#include <functional>

namespace cs
{
	class MetaData;
	class Member;

	typedef std::function<void()> OnMemberUpdateFunc;

	class MemberValueAbstract;
	typedef std::shared_ptr<MemberValueAbstract> MemberValuePtr;

	class MemberValueAbstract
	{
	public:
		virtual ~MemberValueAbstract() { }

		virtual bool equals(void* data) = 0;
	};


	template <class T>
	class MemberValue : public MemberValueAbstract
	{
	public:
		MemberValue(const T& val) :
			value(val) { }

		virtual bool equals(void* data)
		{
			T* typed_data = reinterpret_cast<T*>(data);
			return *typed_data == this->value;
		}

		T value;
	};

	class Member
	{
	public:

		enum MemberCallbackType
		{
			MemberCallbackNone = -1,
			MemberCallbackPre,
			MemberCallbackPost,
			MemberCallbackMAX
		};

		enum MemberValueType
		{
			MemberValueMin,
			MemberValueMax,
			MemberValueDefault,
			MemberValueMAX
		};

		enum MemberFlags
		{
			MemberFlagNone = -1,
			MemberFlagPointer,
			MemberFlagIgnoreGUI,
			MemberFlagIgnoreSerialization,
			MemberFlagEnum,
			MemberFlagResource,
			MemberFlagConst,
			MemberFlagNoSlider,
			MemberFlagCollapseable,
			MemberFlagStartCollapsed,
			MemberFlagString,
			//...
			MemberFlagMAX
		};

		Member(const std::string& n, size_t off, MetaData* meta, size_t f, uint8 index = 0)
			: index(0)
			, name(n)
			, offset(off)
			, data(meta)
			, flags(f) { }

		virtual ~Member() { }

		const std::string& getName() const { return this->name; }
		const std::string& getAnnotation() const { return this->annotation; }
		size_t getOffset() const { return this->offset; }
		virtual const MetaData* getMetaData() const { return this->data; }


		bool hasValue(MemberValueType type) const { return this->values.count(type) > 0; }

		template <typename T>
		void addValue(const T& value, MemberValueType type)
		{
			std::shared_ptr<MemberValue<T>> ptr = std::make_shared<MemberValue<T>>(value);
			this->values[type] = std::static_pointer_cast<MemberValueAbstract>(ptr);
		}

		template <typename T>
		bool getValue(MemberValueType type, T& retVal) const
		{
			ValueMap::const_iterator it = this->values.find(type);
			if (it == this->values.end())
				return false;

			std::shared_ptr<MemberValue<T>> adjVal = std::static_pointer_cast<MemberValue<T>>(it->second);
			retVal = adjVal->value;
			return true;
		}

		bool equalsValue(MemberValueType type, void* data) const
		{
			ValueMap::const_iterator it = this->values.find(type);
			if (it == this->values.end())
				return false;
			return it->second->equals(data);
		}

		virtual void createCallbacks(std::map<Member::MemberCallbackType, std::vector<OnMemberUpdateFunc>>& funcs, void* vptr) const
		{
			assert(false);
		}

		bool getIsPointer() const { return this->flags.test(MemberFlagPointer); }
		bool getIgnoreGUI() const { return this->flags.test(MemberFlagIgnoreGUI); }
		bool getIgnoreSerialization() const { return this->flags.test(MemberFlagIgnoreSerialization); }
		bool getIsEnumeration() const { return this->flags.test(MemberFlagEnum); }
		bool getIsResource() const { return this->flags.test(MemberFlagResource); }
		bool getIsLocked() const { return this->flags.test(MemberFlagConst); }
		bool getUseSlider() const { return !this->flags.test(MemberFlagNoSlider); }
		bool getCollapseable() const { return this->flags.test(MemberFlagCollapseable); }
		bool getStartCollapsed() const { return this->flags.test(MemberFlagStartCollapsed); }
		bool getIsString() const { return this->flags.test(MemberFlagString); }

		void setIndex(size_t i) { this->index = (uint8)i; }
		uint8 getIndex() const { return this->index; }

		void setFlag(MemberFlags flag) { this->flags.set(flag); }
		static size_t generateMemberFlags(std::vector<MemberFlags> flags)
		{
			return BitMask<MemberFlags, MemberFlagMAX>::gen(flags);
		}

        void addComboMeta(MetaData* meta, const std::string& combo_name, const std::string & meta_name);

		void addComboInteger(size_t value, const std::string& combo_name)
		{
			assert(this->combo.find(combo_name) == this->combo.end());
			this->combo[combo_name].meta_data = reinterpret_cast<const MetaData*>(value);
			this->combo[combo_name].meta_name = combo_name;
		}

		struct MetaDataCombo
		{
			const MetaData* meta_data;
			std::string meta_name;
		};
		typedef std::map<std::string, MetaDataCombo> ComboxBoxValues;

		size_t getNumComboMeta() const;
		size_t getComboMetaLabels(StringList& label_list) const;
		const MetaData* getComboMeta(const std::string& combo_name);
		const ComboxBoxValues& getComboMetaValues() const { return this->combo; }

	protected:

		uint8 index;
		std::string name;
		std::string annotation;
		size_t offset;
		const MetaData* data;
		BitMask<MemberFlags, MemberFlagMAX> flags;

		typedef std::map<uint32, MemberValuePtr> ValueMap;
		ValueMap values;

		
		ComboxBoxValues combo;
	};

	typedef std::map<Member::MemberCallbackType, std::vector<OnMemberUpdateFunc>> MemberUpdateCallbackMap;

	template <class T>
	class MemberTyped : public Member
	{
	public:

		struct MemberTypedCallback
		{
			MemberTypedCallback(void(T::*ptr)(), MemberCallbackType type = MemberCallbackNone)
				: callbackType(type)
				, fptr(ptr) { }

			MemberCallbackType callbackType;
			void (T::*fptr) ();
		};

		MemberTyped(const std::string& n, size_t off, MetaData* meta, size_t f = 0, uint8 i = 0)
			: Member(n, off, meta, f, i) { }

		void addCallback(void(T::*func)(void), MemberCallbackType type = MemberCallbackNone)
		{
			callbacks.push_back(MemberTypedCallback(func, type));
		}

		virtual void createCallbacks(MemberUpdateCallbackMap& funcs, void* vptr) const
		{
			T* parentPtr = reinterpret_cast<T*>(vptr);
			for (auto& it : this->callbacks)
			{
				OnMemberUpdateFunc callback = std::bind(it.fptr, parentPtr);
				if (funcs.find(it.callbackType) == funcs.end())
				{
					funcs[it.callbackType] = std::vector<OnMemberUpdateFunc>();
				}
				funcs[it.callbackType].push_back(callback);
			}
		}

	private:

		std::vector<MemberTypedCallback> callbacks;
	};

	template <class T>
	class MemberEnum : public MemberTyped<T>
	{
	public:
		MemberEnum(const std::string& n, size_t off, MetaData* meta, size_t f = 0)
			: MemberTyped<T>(n, off, meta, f) 
		{ 
			assert(this->flags & Member::MemberFlagEnum);
		}
	};
}
