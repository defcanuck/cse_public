#pragma once

#include "scene/Scene.h"
#include "ui/UIStack.h"

namespace cs
{

	class SceneManager : public Singleton<SceneManager>
	{
	public:

		template <class T>
		std::shared_ptr<T> createScene(const std::string& name, SceneParams& params)
		{
			SceneMap::iterator it;
			if ((it = this->scenes.find(name)) != this->scenes.end())
			{
				if (it->second->getName() == params.name)
				{
					log::print(LogWarning, "Duplicate scene found! (might be intentional - ", params.name, ")");
					return std::static_pointer_cast<T>(it->second);
				}

				log::print(LogError, "Duplicate Scene Found! ", name);
				return std::shared_ptr<T>();
			}

			ScenePtr scene = std::static_pointer_cast<Scene>(CREATE_CLASS(T, Scene::kLock, params));
			if (scene)
			{
				if (scene->initFunc)
				{
					scene->data->setContext();
					scene->initFunc(scene.get());
				}
			}
			
			this->scenes[name] = scene;
			return std::static_pointer_cast<T>(scene);
		}

		bool removeScene(const std::string& name)
		{
			SceneMap::iterator it;
			if ((it = this->scenes.find(name)) != this->scenes.end())
			{
				this->scenes.erase(it);
				return true;
			}

			return false;
		}

		ScenePtr getScene(const std::string& name)
		{
			SceneMap::iterator it;
			if ((it = this->scenes.find(name)) != this->scenes.end())
			{
				return it->second;
			}
			return ScenePtr();
		}

	protected:

		template <class C>
		friend class Singleton;

		SceneManager();

		typedef std::map<std::string, ScenePtr> SceneMap;
		SceneMap scenes;

	};
}