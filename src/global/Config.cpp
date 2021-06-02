#include "PCH.h"

#include "global/Config.h"
#include "os/LogManager.h"
#include "os/FileManager.h"

#include <fstream>

namespace cs
{

	Config::Config(const std::string& fileName)
		: loaded(false)
	{
		this->load(fileName);
	}

	bool Config::getValue(const std::string& key, std::string& value)
	{
		ConfigMap::iterator it = this->kv.find(key);
		if (it == this->kv.end())
			return false;
		value = it->second;
		return true;
	}

	bool Config::exists(const std::string& key)
	{
		return this->kv.find(key) != this->kv.end();
	}
	
	void Config::load(const std::string fileName)
	{
		if (!FileManager::hasExecutablePath())
		{
			log_error("No executable base path defined!");
			return;
		}

		std::string path = FileManager::getExecutablePath() + fileName;
		std::ifstream ifs(path);
		if (ifs.is_open())
		{
			std::string line;
			while (!ifs.eof())
			{
				ifs >> line;
				log::info(line);

				size_t pivot = line.find_last_of('=');
				if (pivot != std::string::npos)
				{
					std::string key = line.substr(0, pivot);
					std::string value = line.substr(pivot + 1, std::string::npos);

					if (this->exists(key))
					{
						log::error("Duplicate key found for ", key);
						continue;
					}
					this->kv[key] = value;
					log::info("Adding ", key, " = ", value);
				}
			}
			loaded = true;
		
			ifs.close();
		}
		else
		{
            log::error("Cannot open ", fileName, " at path ", FileManager::getExecutablePath());
			loaded = false;
		}
	}

	ConfigManager::~ConfigManager()
	{
		for (auto& it : this->configMap)
			delete it.second;
		this->configMap.clear();
	}

	Config* ConfigManager::getConfig(const std::string& fileName)
	{
		std::unordered_map<std::string, Config*>::iterator it = this->configMap.find(fileName);
		if (it != this->configMap.end())
		{
			return it->second;
		}

		Config* new_config = new Config(fileName);
		this->configMap[fileName] = new_config;
		return new_config;
	}
}
