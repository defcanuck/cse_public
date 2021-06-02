#include "PCH.h"

#include "os/FileManager.h"
#include "os/LogManager.h"

#include "global/Utils.h"

#include <assert.h>

namespace cs
{

	const char* kFilePathTypeStr[] = 
	{
		"Base",
		"Game"
	};

	std::string FileManager::exePath = "";
	FileManager* FileManager::instance = nullptr;

	bool FileStream::open(const std::string& path)
	{
		this->stream = std::ifstream(path);
		if (this->stream.is_open())
		{
			FileManager::splitPath(path, this->fileName, this->filePath);
			return true;
		}
		return false;
	}

	FileManager::~FileManager()
	{
	}

	std::string FileManager::getExtension(const std::string& fileName)
	{
		size_t found = fileName.find_first_of(".");
		if (found != std::string::npos)
			return fileName.substr(found + 1, std::string::npos);
		return "";
	}

	std::string FileManager::stripExtension(const std::string& fileName)
	{
		size_t found = fileName.find_first_of(".");
		if (found != std::string::npos)
			return fileName.substr(0, found);
		return fileName;
	}

	bool FileManager::splitPath(const std::string& fullFilePath, std::string& fileName, std::string& filePath)
	{
		size_t found = fullFilePath.find_last_of(FileManager::getInstance()->separator());
		if (found != std::string::npos)
		{
			filePath = fullFilePath.substr(0, found);
			fileName = fullFilePath.substr(found + 1, std::string::npos);
			return true;
		}
		return false;
	}
	
	void FileManager::addEntry(FileSystemNodePtr root, const std::string& filePath)
	{
		assert(root);

		FilePathParams path_params;
		path_params.pathList = explode(filePath, FileManager::getInstance()->separator());
		path_params.fullPath = filePath;

		if (!root->addPath(path_params, 0))
		{
			log_error("Error: Duplicate Entry for path: ", filePath);
			return;
		}
	}

	const FileSystemEntryPtr FileManager::getFile(const std::string& fileName)
	{
		if (!this->root)
		{
			log::error("No file system to search!");
			return nullptr;
		}
		return this->root->getEntry<FileSystemEntry>(fileName, FileDataType_File);
	}

	void FileManager::setBasePath(const std::string& root, bool absolute)
	{
		if (absolute)
			this->basePath = root;
		else
			this->basePath = FileManager::getExecutablePath() + root;

		this->refreshFromBasePath();
		// this->print();
	}

	void FileManager::setSavePath(const std::string& root, bool absolute)
	{
		if (absolute)
			this->savePath = root;
		else
			this->savePath = FileManager::getExecutablePath() + root;
	}

	void FileManager::refreshFromBasePath()
	{
		this->root = CREATE_CLASS(FileSystemNode, FileDataType_Directory, "");
		FileManager::refreshFileSystem(&this->root, this->basePath);
		//this->printFileSystem();
	}

	void FileManager::printFileSystem()
	{
		if (!this->root)
		{
			return;
		}

		std::stringstream str;
		this->root->print(str, 0);
		log::info(str.str());
	}

	StringList FileManager::getFilesByExtension(const std::string& fileExt)
	{
		StringList list;
		this->getFilesByExtensionRef(list, fileExt);
		return list;
	}

	FileSystemNodePtr FileManager::getNodesByExtension(const std::string& fileExt)
	{
		FileSystemNodePtr base_node = CREATE_CLASS(FileSystemNode, FileDataType_Directory, "root");
		FileSystemDirectoryPtr file_dir = std::static_pointer_cast<FileSystemDirectory>(this->root->data);

		for (auto& it : file_dir->nodes)
		{
			it->getNodesByExtension(fileExt, base_node);
		}
		return base_node;
	}

	void FileManager::getFilesByExtensionRef(StringList& list, const std::string& fileExt)
	{
		if (!this->root)
		{
			log::error("No file system to parse!");
			return;
		}

		list.clear();
		this->root->getFilesByExtension(list, fileExt);
	}

	bool FileManager::addPathToFile(const std::string& fullPath, std::string& fileName)
	{
		std::string filePath;
		if (FileManager::splitPath(fullPath, fileName, filePath))
		{
			if (!this->getFile(fileName))
			{
				if (!this->rootAdd)
				{
					this->rootAdd = CREATE_CLASS(FileSystemNode, FileDataType_Directory, "");
				}
				this->addEntry(this->rootAdd, fullPath);
			}
			return true;
		}
		return false;
	}

	bool FileManager::openStreamFile(const std::string& fileName, DataStream** stream)
	{
		std::string filePath;
		if (this->getPathToFile(fileName, filePath))
		{
			return this->openStreamPath(filePath, stream);
		}
		return false;
	}

	bool FileManager::openStreamPath(const std::string& path, DataStream** stream)
	{

		FileStream* file_stream = new FileStream();
		if (file_stream->open(path))
		{
			(*stream) = file_stream;

			std::string fileName, filePath;
			FileManager::splitPath(path, fileName, filePath);
			this->addPathToFile(path, fileName);

			return true;
		}

		delete file_stream;
		return false;

	}


	bool FileManager::isSeparator(char c)
	{
		return c == '\\' || c == '/';
	}

	void FileManager::flipSeparator(std::string& path)
	{
		for (size_t i = 0; i < path.size(); i++)
		{
			if (FileManager::isSeparator(path[i]))
			{
				path[i] = FileManager::getInstance()->separator();
			}
		}
	}
}
