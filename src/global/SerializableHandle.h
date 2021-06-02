#pragma once

#include "os/FileDialog.h"
#include "os/FileManager.h"
#include "sdl/SDL_Helpers.h"
#include "sdl/SDL_Input.h"
#include "AssetType.h"

#include <string>

#define SET_RESOURCE_EXTENSION(class_name, extension) \
template <> \
std::string SerializableHandle<class_name>::getExtension() \
{ \
	return extension; \
}

#define SET_RESOURCE_DESCRIPTION(class_name, description) \
template <> \
std::string SerializableHandle<class_name>::getExtension() \
{ \
		return description; \
}


namespace cs
{
	enum SerializeMethod
	{
		SerializeJSON,
		SerializeBinary,
		//...
		SerializeMAX
	};

	class SerializableHandleBase
	{
	public:
		SerializableHandleBase(SerializeMethod mth = SerializeJSON)
			: method(mth)
			, fileName("")
			, filePath("") { }

		std::string getFullPath() { return this->filePath + this->fileName; }
		const std::string& getFileName() const { return this->fileName; }

		void setFileName(const std::string& fn) { this->fileName = fn; }
		void setFilePath(const std::string& fp) { this->filePath = fp; }

		void setFullPath(const std::string& fp)
		{
			FileManager::splitPath(fp, this->fileName, this->filePath);
			this->filePath = this->filePath + FileManager::getInstance()->separator();
		}

		bool canSave() const { return this->fileName.length() > 0 && this->filePath.length() > 0; }

	protected:

		SerializeMethod method;
		std::string fileName;
		std::string filePath;
	};

	template <class T>
	class SerializableHandle : public SerializableHandleBase
	{
	public:

		SerializableHandle(SerializeMethod mth = SerializeJSON)
			: SerializableHandleBase(mth)
			, object(nullptr)
			, loadFlags(kLoadFlagMaskAll)
			
		{ } 

		SerializableHandle(std::shared_ptr<T> obj, SerializeMethod mth = SerializeJSON)
			: SerializableHandleBase(mth)
			, object(obj)
			, loadFlags(kLoadFlagMaskAll)
		{ }

		std::shared_ptr<T>& get() { return this->object;  }
		void set(std::shared_ptr<T>& obj) { this->object = obj; }

		const std::shared_ptr<T>& const_get() const { return this->object; }

		T* operator->() { return this->object.get(); }

		bool exists() const { return this->object.get() != nullptr; }

		bool save();
		bool saveAs(const std::string& path);
		bool openSaveAs(const std::string& fileExt, const std::string& fileDesc);

		bool load(const std::string& path);
		bool openLoad();

		bool refresh(bool doPostLoad = true);

		void setLoadFlags(const LoadFlagMask& flags) { this->loadFlags = flags; }

		static std::string getExtension() { return "json"; }
		static std::string getDescription() { return "Default JSON"; }
	
	private:

		void setFileNameAndPath(const std::string& path);
		bool saveInternal(const std::string& path);
		bool loadInternal(const std::string& path, bool doPostLoad = true);

		std::shared_ptr<T> object;	
		LoadFlagMask loadFlags;
	};

	template <class T>
	bool SerializableHandle<T>::save()
	{
		std::string path = this->getFullPath();
		return this->saveInternal(path);
	}

	template <class T>
	bool SerializableHandle<T>::saveAs(const std::string& path)
	{
		bool ret = this->saveInternal(path);
		if (ret)
		{
			this->setFileNameAndPath(path);
			FileManager::getInstance()->addPathToFile(fileName, filePath);
		}
		return true;
	}

	template <class T>
	bool SerializableHandle<T>::refresh(bool doPostLoad)
	{
		std::string path = this->getFullPath();
		bool ret = this->loadInternal(path, doPostLoad);
		if (!ret)
		{
			log::info("Cannot open file at path ", path);
		}
		return ret;
	}

	template <class T>
	bool SerializableHandle<T>::load(const std::string& path)
	{
		this->setFileNameAndPath(path);
		bool ret = this->loadInternal(path);
		if (!ret)
		{
			log::info("Failed to open ", path);
		}
		return ret;
	}

	template <class T>
	bool SerializableHandle<T>::openLoad()
	{
		// TODO
		return true;
	}

	template <class T>
	void SerializableHandle<T>::setFileNameAndPath(const std::string& path)
	{
		size_t pivot = path.find_last_of('\\');
		if (pivot != std::string::npos)
		{
			this->filePath = path.substr(0, pivot + 1);
			this->fileName = path.substr(pivot + 1, std::string::npos);
		}
		else
		{
			this->filePath = "";
			this->fileName = path;
		}
	}

	template <class T>
	inline bool SerializableHandle<T>::openSaveAs(const std::string& fileExt, const std::string& fileDesc)
	{
		std::string str = openFileSaveDialog(fileDesc, fileExt);
		sdl::consumeEvent();
		bool ret = this->saveAs(str);
		if (ret)
		{
			std::string fileName, filePath;
			FileManager::getInstance()->splitPath(str, fileName, filePath);
			FileManager::getInstance()->addPathToFile(fileName, filePath);
		}
		return ret;
	}

	template <class T>
	bool SerializableHandle<T>::saveInternal(const std::string& path)
	{
		switch (this->method)
		{
			case SerializeJSON:
			{
				std::ofstream ofs;
				ofs.open(path, std::ofstream::out);
				if (ofs.is_open())
				{
					const MetaData* meta = this->object->getMetaData();
					log::info("Serializing type ", meta->getName());

					RefVariant ref(meta, this->object.get());
					ref.serialize(ofs);
					ofs.close();
					this->setFileNameAndPath(path);
					return true;
				}

				return false;
			} break;
			case SerializeBinary:
			{
				assert(false); // todo
				return false;

			} break;
		}

		return false;
	}

	template <class T>
	bool SerializableHandle<T>::loadInternal(const std::string& path, bool doPostLoad)
	{
		switch (this->method)
		{
			case SerializeJSON:
			{

				AutoRelease<DataStream> stream;
				if (FileManager::getInstance()->openStreamPath(path, &stream.value))
				{
					std::string buffer;
					std::istream& ifs = stream->getStream();

					ifs.seekg(0, std::ios::end);
					buffer.reserve((uint32) ifs.tellg());
					ifs.seekg(0, std::ios::beg);

					buffer.assign((std::istreambuf_iterator<char>(ifs)),
						std::istreambuf_iterator<char>());
					
					std::vector<char> writable(buffer.begin(), buffer.end());
					writable.push_back('\0');

					char* source = &writable[0];
					char* endptr;

					JsonValue value;
					JsonAllocator allocator;
					int status = jsonParse(source, &endptr, &value, allocator);
					if (status != JSON_OK)
					{
						size_t idx = endptr - source;
						log::print(LogError, jsonStrError(status), " at ", idx);
						log::print(LogError, buffer.substr(0, idx), " ", buffer[idx]);
						return false;
					}

					// Nuke the opening array braces
					assert(value.getTag() == JSON_OBJECT);

					if (this->object)
						this->object->onPreLoad();

					RefVariant ref(*this->object);
					ref.deserialize(value);

					if (doPostLoad && this->object)
						this->object->onPostLoad(this->loadFlags);

					return true;
				}

				return false;
			}
		}

		return false;
		
	}
}
