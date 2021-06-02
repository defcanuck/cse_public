#pragma once

#include <string>
#include <unordered_map>
#include <functional>

#include "global/Singleton.h"

namespace cs
{
	class Config
	{
	public:

		Config(const std::string& fileName);

		bool exists(const std::string& key);
		bool getValue(const std::string& key, std::string& value);
		bool isLoaded() const { return this->loaded; }

	private:

		void load(const std::string fileName);

		typedef std::unordered_map<std::string, std::string> ConfigMap;
		ConfigMap kv;
		bool loaded;
	};

	class ConfigManager : public Singleton<ConfigManager>
	{
	public:
		ConfigManager() { }
		~ConfigManager();

		Config* getConfig(const std::string& fileName);

	private:

		std::unordered_map<std::string, Config*> configMap;
	};
}