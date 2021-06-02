#pragma once

#include "ClassDef.h"

namespace cs
{

	template <typename key, class value>
	class SceneCollection
	{
	public:

		SceneCollection() { }

		template <class C>
		bool add(std::shared_ptr<C> new_element, key index)
		{
			auto it = this->value_map.find(index);
			if (it != this->value_map.end())
			{
				log::print(LogError, "Duplicate Key Found:", index);
				return false;
			}

			std::shared_ptr<value> new_value = std::static_pointer_cast<value>(new_element);
			this->value_map[index] = new_value;
			this->value_vec.push_back(new_value);

			return true;
		}

		template <class C>
		bool removeByElement(std::shared_ptr<C>& elem_to_remove, const key&(value::*func_ptr)())
		{
			std::function<const key&()> func = std::bind(func_ptr, elem_to_remove.get());
			const key& lookup_key = func();
			return this->removeByKey(lookup_key);
		}

		bool removeByKey(const key& key_to_remove)
		{
			auto it = this->value_map.find(key_to_remove);
			if (it != this->value_map.end())
			{
				std::shared_ptr<value> entity = this->value_map[key_to_remove];
				this->value_map.erase(it);
				for (typename ValueVector::iterator iv = this->value_vec.begin(); iv != this->value_vec.end(); ++iv)
				{
					if ((*iv).get() == entity.get())
					{
						this->value_vec.erase(iv);
						return true;
					}
				}
				log_error("Key mismatch!", key_to_remove);
			}
			return false;
		}

		void traverse(void(*traverseFunc)(value*, void*), void* data)
		{
			for (auto& it : this->value_vec)
			{
				it->traverse(traverseFunc, data);
			}
		}

		std::shared_ptr<value>& getByKey(const key& index)
		{
			static std::shared_ptr<value> kEmptyValuePtr(nullptr);
			auto it = this->value_map.find(index);
			if (it != this->value_map.end())
				return it->second;
			
			return kEmptyValuePtr;
		}

		std::shared_ptr<value> getAt(size_t idx)
		{ 
			return std::static_pointer_cast<value>(this->value_vec[idx]);
		}

		void remap(const key&(value::*func_ptr)())
		{
			this->value_map.clear();
			for (auto& it : this->value_vec)
			{
				std::function<const key&()> func = std::bind(func_ptr, it.get());
				const key& gen_key = func();
				if (this->value_map.find(gen_key) != this->value_map.end())
				{
					assert(false && "Duplicate key found during a remap! BAD!");
					continue;
				}
				this->value_map[gen_key] = it;
			}
		}

		void clear()
		{
			this->value_map.clear();
			this->value_vec.clear();
		}

		std::string getKeyAtIndex(size_t idx) 
		{ 
			assert(idx < this->value_vec.size());
			std::stringstream str;
			typename ValueMap::iterator it = this->value_map.begin();
			std::advance(it, idx);
			str << it->first;
			return str.str();
		}

		size_t getNumElements() const 
		{ 
			return value_map.size(); 
		}

		typedef std::map<key, std::shared_ptr<value>> ValueMap;
		ValueMap value_map;

		typedef std::vector<std::shared_ptr<value>> ValueVector;
		ValueVector value_vec;
	};
}
