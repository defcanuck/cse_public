#pragma once

#include <string>
#include <istream>
#include <fstream>
#include <sstream>

#include "ClassDef.h"
#include "global/Values.h"

namespace cs
{

	struct FilePathParams
	{
		StringList pathList;
		std::string fullPath;
	};

	enum FileDataType
	{
		FileDataType_None = -1,
		FileDataType_File,
		FileDataType_Directory,
		FileDataType_MAX
	};

	struct FileSystemBase
	{
		FileSystemBase(const std::string& n)
			: name(n)
		{ }

		const FileDataType getType() const { return FileDataType_None; }

		std::string name;
	};

	struct FileSystemEntry : public FileSystemBase
	{
		FileSystemEntry(const std::string& n);
		const FileDataType getType() const { return FileDataType_File; }

		std::string ext;
	};
	typedef std::shared_ptr<FileSystemEntry> FileSystemEntryPtr;

	struct FileSystemNode;
	typedef std::shared_ptr<FileSystemNode> FileSystemNodePtr;

	struct FileSystemDirectory : public FileSystemBase
	{
		FileSystemDirectory(const std::string& n) :
			FileSystemBase(n)
		{ }
		const FileDataType getType() const { return FileDataType_Directory; }
		size_t getNumNodes() const { return this->nodes.size(); }

		FileSystemNodePtr find(const std::string& n);

		std::vector<FileSystemNodePtr> nodes;
	};
	typedef std::shared_ptr<FileSystemDirectory> FileSystemDirectoryPtr;

	struct FileSystemNode
	{

		FileSystemNode(FileDataType t, const std::string& name);
		FileSystemNode(FileSystemNode& node);
		virtual ~FileSystemNode() { }

		template <class T>
		std::shared_ptr<T> getData()
		{
			std::shared_ptr<T> entry = std::static_pointer_cast<T>(this->data);
			return entry;
		}

		template <class T>
		std::shared_ptr<T> getEntry(const std::string& name, FileDataType search_type)
		{
			if (this->type == search_type && name == this->data->name)
			{
				assert(this->type == search_type);
				return std::static_pointer_cast<T>(this->data);
			}

			if (this->type == FileDataType_Directory)
			{
				FileSystemDirectoryPtr file_dir = std::static_pointer_cast<FileSystemDirectory>(this->data);
				if (file_dir->name == "ArtSource")
				{
					// Don't return things in Art Source path
					return std::shared_ptr<T>();
				}

				
				for (auto& it : file_dir->nodes)
				{
					std::shared_ptr<T> found_node = it->getEntry <T>(name, search_type);
					if (found_node.get())
					{
						return found_node;
					}
				}
			}
			return std::shared_ptr<T>();
		}

		bool getNodesByExtension(const std::string& ext, FileSystemNodePtr& base_node);
		void getFilesByExtension(StringList& list, const std::string& ext);
		bool getPath(const std::string& name, StringList& path, FileDataType search_type);
		bool exists(const StringList& path);
		bool addPath(FilePathParams& path, size_t path_index);

		void debugPrint();
		void print(std::stringstream& out, uint32 depth = 0);

		FileDataType type;
		std::shared_ptr<FileSystemBase> data;
	};
}