#include "PCH.h"

#include "global/ResourceFactory.h"
#include "gfx/Texture.h"
#include "gfx/Shader.h"
#include "gfx/Mesh.h"
#include "gfx/RenderInterface.h"
#include "scene/SceneReference.h"
#include "scripting/LuaScript.h"
#include "global/PropertySet.h"
#include "fx/ParticleEffect.h"
#include "audio/SoundManager.h"

#include "animation/spine/SpineAnimation.h"

#define RES_TYPE(T) std::type_index(typeid(T))

namespace cs
{

	ResourceFactory::ResourceFactory()
		: inScope(false)
	{
		loaders[RES_TYPE(Texture)] = [](const std::string& fileName, const std::string& filePath)
		{
			TextureResourcePtr textureResource = RenderInterface::getInstance()->loadTexture(filePath);
			TexturePtr texture = CREATE_CLASS(Texture, fileName, textureResource);
			return std::static_pointer_cast<Resource>(texture);
		};

		loaders[RES_TYPE(SceneReference)] = [](const std::string& fileName, const std::string& filePath)
		{
			return std::static_pointer_cast<Resource>(
				SceneReferenceCache::getInstance()->loadReference(fileName, filePath));
		};

		loaders[RES_TYPE(Mesh)] = [](const std::string& fileName, const std::string& filePath)
		{
			MeshPtr mesh = CREATE_CLASS(Mesh, fileName, filePath);
			return std::static_pointer_cast<Resource>(mesh);
		};

		loaders[RES_TYPE(ParticleEffect)] = [](const std::string& fileName, const std::string& filePath)
		{
			ParticleEffectPtr particleEffect = CREATE_CLASS(ParticleEffect, fileName, filePath);
			return std::static_pointer_cast<Resource>(particleEffect);
		};

		loaders[RES_TYPE(LuaScript)] = [](const std::string& fileName, const std::string& filePath)
		{
			LuaScriptFilePtr lua_res = CREATE_CLASS(LuaScriptFile, fileName);
			return std::static_pointer_cast<Resource>(lua_res);
		};

		loaders[RES_TYPE(LuaScriptFile)] = [](const std::string& fileName, const std::string& filePath)
		{
			LuaScriptFilePtr lua_res = CREATE_CLASS(LuaScriptFile, fileName);
			return std::static_pointer_cast<Resource>(lua_res);
		};

		loaders[RES_TYPE(PropertySetResource)] = [](const std::string& fileName, const std::string& filePath)
		{
			PropertySetResourcePtr prop_res = CREATE_CLASS(PropertySetResource, fileName, filePath);
			return std::static_pointer_cast<Resource>(prop_res);
		};

		loaders[RES_TYPE(SpineSkeletonData)] = [](const std::string& fileName, const std::string& filePath)
		{
			SpineSkeletonDataPtr skl_data_ptr = CREATE_CLASS(SpineSkeletonData, fileName);
			return std::static_pointer_cast<Resource>(skl_data_ptr);
		};

		loaders[RES_TYPE(SpineTextureAtlas)] = [](const std::string& fileName, const std::string& filePath)
		{
			SpineTextureAtlasPtr anim_atlas_ptr = CREATE_CLASS(SpineTextureAtlas, fileName);
			return std::static_pointer_cast<Resource>(anim_atlas_ptr);
		};

		loaders[RES_TYPE(SoundEffect)] = [](const std::string& fileName, const std::string& filePath)
		{
			SoundEffectPtr effect_ptr = CREATE_CLASS(SoundEffect, fileName, filePath);
			return std::static_pointer_cast<Resource>(effect_ptr);
		};

		loaders[RES_TYPE(Music)] = [](const std::string& fileName, const std::string& filePath)
		{
			MusicPtr music_ptr = CREATE_CLASS(Music, fileName, filePath);
			return std::static_pointer_cast<Resource>(music_ptr);
		};

		//this->canCacheMap[RES_TYPE(SpineSkeletonData)] = false;
	}

#define CHECK_RESOURCE(T_class, T_ext, fileName) if (ext == SerializableHandle<T_ext>::getExtension()) \
{ \
	log::info("preloading", fileName, " as ", #T_class); \
	return ResourceFactory::getInstance()->loadResource<T_class>(fileName); \
}

	std::shared_ptr<Resource> ResourceFactory::loadFileByExtension(const std::string& fileName)
	{
		const std::string ext = FileManager::getExtension(fileName);

		static bool initialized = false;
		static std::map<std::string, std::function<ResourcePtr(const std::string&)>> LoadersByExtension;
		if (!initialized)
		{
			LoadersByExtension["png"] = [](const std::string& fileName) { return ResourceFactory::getInstance()->loadResource<Texture>(fileName); };
			LoadersByExtension["obj"] = [](const std::string& fileName) { return ResourceFactory::getInstance()->loadResource<Mesh>(fileName); };
			LoadersByExtension["prop"] = [](const std::string& fileName) { return ResourceFactory::getInstance()->loadResource<PropertySetResource>(fileName); };
			LoadersByExtension["entity"] = [](const std::string& fileName) { return ResourceFactory::getInstance()->loadResource<SceneReference>(fileName); };
			LoadersByExtension["json"] = [](const std::string& fileName) { return ResourceFactory::getInstance()->loadResource<SpineSkeletonData>(fileName); };
			LoadersByExtension["fx"] = [](const std::string& fileName) { return ResourceFactory::getInstance()->loadResource<ParticleEffect>(fileName); };
			LoadersByExtension["wav"] = [](const std::string& fileName) { return ResourceFactory::getInstance()->loadResource<SoundEffect>(fileName); };
			LoadersByExtension["ogg"] = [](const std::string& fileName) { return ResourceFactory::getInstance()->loadResource<Music>(fileName); };
			LoadersByExtension["atlas"] = [](const std::string& fileName) { return ResourceFactory::getInstance()->loadResource<SpineTextureAtlas>(fileName); };
			initialized = true;
		}

		std::map<std::string, std::function<ResourcePtr(const std::string&)>>::iterator it = LoadersByExtension.find(ext);
		if (it != LoadersByExtension.end())
		{
			return it->second(fileName);
		}
		
		return std::shared_ptr<Resource>();
	}

	void ResourceFactory::beginScopeInternal()
	{
		inScope = true;
	}

	void ResourceFactory::endScopeInternal(const std::string& outFileName)
	{
		inScope = false;

#if defined(CS_EDITOR)
		std::ofstream outFile;
		if (outFileName.length() > 0)
		{
			std::string path = FileManager::getExecutablePath() + outFileName;
			outFile.open(path, std::ios::out);
		}
		else
			log::warning("No output file to save!");

		for (auto& it : this->loads)
		{
			log::info(it);
			if (outFile.is_open())
			{
				outFile << it << std::endl;
			}
		}
		if (outFile.is_open())
			outFile.close();

		this->loads.clear();
#endif

	}

	void ResourceFactory::preloadFromFile(const std::string& fileName)
	{
		std::string path;
		
		if (FileManager::getInstance()->getPathToFile(fileName, path))
		{
			std::ifstream ifs(path);

			if (ifs.is_open())
			{
				std::string line;
				while (!ifs.eof())
				{
					ifs >> line;
					if (ResourceFactory::getInstance()->loadFileByExtension(line).get() == nullptr)
					{
						log::error("Failed to preload ", line);
					}
					else
					{
						log::info("Preload File: ", line);
					}
				}
				ifs.close();
			}
		}
		else
		{
			log::warning("Cannot find an applicable preload file with filename ", fileName);
		}
	}
}
