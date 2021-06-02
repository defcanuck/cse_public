#pragma once

#include "global/Resource.h"
#include "os/LogManager.h"
#include "os/FileManager.h"
#include "global/Singleton.h"

#include <memory>
#include <map>
#include <unordered_map>
#include <typeindex>
#include <functional>
#include <set>

namespace cs
{

	typedef std::shared_ptr<Resource> ResourcePtr;

	class ResourceFactory : public Singleton<ResourceFactory>
	{
	public:
		
		ResourceFactory();

		template<class T>
		std::shared_ptr<Resource> loadResource(const std::string& fileName)
		{
			std::type_index index(typeid(T));
			
			// Empty resource cache - prime it
			if (this->cache.count(index) == 0)
				this->cache[index] = ResourceTypeCache();
			
			
			if (this->canCacheResource<T>())
			{
				ResourceTypeCache& typeCache = this->cache[index];
				ResourceTypeCache::iterator it;
				if ((it = typeCache.find(fileName)) != typeCache.end())
					return std::static_pointer_cast<Resource>(it->second);
			}
			
			if (this->loaders.count(index) == 0)
			{
				log::print(LogError, "Resource Loading Not Supported!");
				return nullptr;
			}

			std::string filePath;
			FileManager::getInstance()->getPathToFile(fileName, filePath);
			ResourcePtr resource = this->loaders[index](fileName, filePath);

			if (resource && this->canCacheResource<T>())
			{

				ResourceTypeCache& typeCache = this->cache[index];
				typeCache[fileName] = resource;

#if defined(CS_EDITOR)
				if (this->inScope)
				{
					if (this->loads.count(fileName) == 0)
						this->loads.insert(fileName);
				}
#endif

				return std::static_pointer_cast<Resource>(resource);
			}

			return nullptr;
		}

		template <class T>
		std::shared_ptr<T> loadResourceTyped(const std::string& fileName)
		{
			return std::static_pointer_cast<T>(this->loadResource<T>(fileName));
		}

		template <class T>
		bool addResource(std::shared_ptr<T>& resource)
		{
			std::type_index index(typeid(T));
			if (this-cache.count(index) == 0)
				cache[index] = ResourceTypeCache();
			ResourceTypeCache& typeCache = this->cache[index];

			std::shared_ptr<T> temp_resource;
			std::shared_ptr<Resource> ptr = std::static_pointer_cast<Resource>(resource);
			std::string resource_name = ptr->getName();

			if (this->getResource<T>(resource_name, temp_resource))
			{
				log::error("Duplicate resource exists for ", resource_name);
				return false;
			}

			// Add to the cache
			typeCache[resource_name] = ptr;
			return true;
		}

		template <class T>
		bool getResource(const std::string fileName, std::shared_ptr<T>& ptr)
		{
			std::type_index index(typeid(T)); 
			if (this->cache.count(index) == 0)
				this->cache[index] = ResourceTypeCache();

			ResourceTypeCache& typeCache = this->cache[index];
			ResourceTypeCache::iterator it;

			if ((it = typeCache.find(fileName)) != typeCache.end())
			{
				ptr = std::static_pointer_cast<T>(it->second);
				return true;
			}

			return false;
		}

		template <class T>
		void performOperation(void(*opFunc)(std::shared_ptr<T>&, uintptr_t*), uintptr_t* userData)
		{
			
			std::type_index index(typeid(T));
			assert(this->cache.count(index) > 0);
			ResourceTypeCache& typeCache = this->cache[index];
			for (auto& it : typeCache)
			{
				std::shared_ptr<T> ptr = std::static_pointer_cast<T>(it.second);
				opFunc(ptr, userData);
			}
		}

		template <class T>
		void getResourceNamesByType(std::vector<std::string>& resources, bool(*checkFunc)(std::shared_ptr<T>&) = nullptr)
		{
			std::type_index index(typeid(T));
			if (this->cache.count(index) == 0)
				return;
			ResourceTypeCache& typeCache = this->cache[index];
			for (auto& it : typeCache)
			{
				std::shared_ptr<T> resource_typed = std::static_pointer_cast<T>(it.second);
				if (!checkFunc || checkFunc(resource_typed))
					resources.push_back(it.first);
			}
		}

		void getResourceNames(std::map<std::string, StringList>& resources)
		{
			for (auto it : this->cache)
			{
				std::string fileType = "";
				StringList list;
				for (auto& entry : it.second)
				{
					if (fileType.length() == 0)
					{
						fileType = entry.second->getMetaData()->getName();
					}
					list.push_back(entry.second->getName());
				}

				resources[fileType] = list;
			}
		}

		template <class T>
		void clearResourceByType()
		{
			std::type_index index(typeid(T));
			ResourceCache::iterator it = this->cache.find(index);
			if (it != this->cache.end())
			{
				this->cache.erase(it);
			}
		}

		void clearAllResources(std::set<std::type_index>& ignoredTypes)
		{
			for (ResourceCache::iterator it = this->cache.begin(); it != this->cache.end(); it++)
			{
				if (ignoredTypes.find(it->first) != ignoredTypes.end())
				{
					it = this->cache.erase(it);
				}
			}
			
			this->cache.clear();
		}

		void clearResourceByName(const std::string& cache_name)
		{
			ResourceCache::iterator it = this->cache.begin();
			while (it != this->cache.end())
			{
				std::string fileType = "";
				StringList list;
				ResourceTypeCache& res = it->second;
				ResourceTypeCache::iterator res_iter = res.begin();
				if (res_iter != res.end() && res_iter->second->getMetaData()->getName() == cache_name)
				{
					this->cache.erase(it);
					return;
				}
				it++;
			}
			log::error("Cannot find ", cache_name, " in any ResourceFactory caches");
		}

		static std::shared_ptr<Resource> loadFileByExtension(const std::string& fileName);

		static void preloadFromFile(const std::string& fileName);
		static void beginScope() { ResourceFactory::getInstance()->beginScopeInternal(); }
		static void endScope(const std::string& outFileName)   { ResourceFactory::getInstance()->endScopeInternal(outFileName); }

	private:

		void beginScopeInternal();
		void endScopeInternal(const std::string& outFileName);

		template <class T>
		bool canCacheResource()
		{
			std::type_index index(typeid(T));
			CanCacheResourceMap::iterator it = this->canCacheMap.find(index);
			if (it != this->canCacheMap.end())
			{
				return it->second;
			}
			return true;
		}

		typedef std::unordered_map<std::string, ResourcePtr> ResourceTypeCache;
		typedef std::map<std::type_index, ResourceTypeCache> ResourceCache;
		typedef std::map<std::type_index, std::function<ResourcePtr(const std::string&, const std::string&)>> ResourceLoaders;

		typedef std::map<std::type_index, bool> CanCacheResourceMap;
		
		ResourceCache cache;
		ResourceLoaders loaders;
		CanCacheResourceMap canCacheMap;
		
		bool inScope;
		std::set<std::string> loads;
	};
}
