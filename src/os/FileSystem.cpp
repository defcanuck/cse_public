#include "PCH.h"
#include "os/FileSystem.h"
#include "os/FileManager.h"

namespace cs
{
	FileSystemEntry::FileSystemEntry(const std::string& n)
		: FileSystemBase(n)
		, ext(FileManager::getExtension(n))
	{ }

	FileSystemNode::FileSystemNode(FileDataType t, const std::string& name)
		: type(t)
	{
		switch (this->type)
		{
		case FileDataType_File:
			this->data = CREATE_CLASS(FileSystemEntry, name);
			break;
		case FileDataType_Directory:
			this->data = CREATE_CLASS(FileSystemDirectory, name);
			break;
		}
	}

	FileSystemNode::FileSystemNode(FileSystemNode& node)
		: type(node.type)
		, data(node.data)
	{
		log::info("Copy - FileSystemNode");
	}


	FileSystemNodePtr FileSystemDirectory::find(const std::string& n)
	{
		for (auto& it : this->nodes)
		{
			FileSystemBase* file_base = it->data.get();
			assert(file_base);
			if (file_base->name == n)
			{
				return it;
			}
		}
		return FileSystemNodePtr();
	}

	bool FileSystemNode::getPath(const std::string& name, StringList& path, FileDataType search_type)
	{
		if (this->type == search_type && name == this->data->name)
		{
			// paths match
			return true;
		}

		if (this->type == FileDataType_Directory)
		{
			FileSystemDirectoryPtr file_dir = std::static_pointer_cast<FileSystemDirectory>(this->data);
			for (auto& it : file_dir->nodes)
			{
				if (it->getPath(name, path, search_type))
				{
					path.push_back(file_dir->name);
					return true;
				}
			}
		}

		return false;
	}

	bool FileSystemNode::exists(const StringList& path)
	{
		// yada yada
		return false;
	}

	bool FileSystemNode::addPath(FilePathParams& path, size_t path_index)
	{
		assert(this->type == FileDataType_Directory);
		if (path_index == path.pathList.size() - 1)
		{
			const std::string& file_name = path.pathList[path_index];

			FileSystemDirectoryPtr file_dir = std::static_pointer_cast<FileSystemDirectory>(this->data);
			if (file_dir->find(file_name) != nullptr)
			{
				log::error("Duplicate entry for file ", file_name);
				return false;
			}

			FileSystemNodePtr new_node = CREATE_CLASS(FileSystemNode, FileDataType_File, file_name);
			FileSystemEntryPtr new_entry = std::static_pointer_cast<FileSystemEntry>(new_node->data);
			new_entry->ext = FileManager::stripExtension(new_entry->name);
			file_dir->nodes.push_back(new_node);

			return true;
		}
		else if (path_index < path.pathList.size() - 1)
		{
			const std::string& dir_name = path.pathList[path_index];
			FileSystemDirectoryPtr file_dir = std::static_pointer_cast<FileSystemDirectory>(this->data);
			if (!file_dir)
			{
				return false;
			}

			FileSystemNodePtr target_dir = nullptr;
			if ((target_dir = file_dir->find(dir_name)) == nullptr)
			{
				target_dir = CREATE_CLASS(FileSystemNode, FileDataType_Directory, dir_name);
				file_dir->nodes.push_back(target_dir);
			}
			return target_dir->addPath(path, path_index + 1);
		}
		return false;
	}

	bool FileSystemNode::getNodesByExtension(const std::string& ext, FileSystemNodePtr& base_node)
	{
		bool found_entry = false;
		assert(base_node->type == FileDataType_Directory);

		FileSystemDirectoryPtr base_dir = std::static_pointer_cast<FileSystemDirectory>(base_node->data);
		switch (this->type)
		{
			case FileDataType_File:
			{
				FileSystemEntryPtr file_entry = std::static_pointer_cast<FileSystemEntry>(this->data);
				assert(file_entry);
				if (file_entry->ext == ext)
				{
					base_dir->nodes.push_back(CREATE_CLASS(FileSystemNode, FileDataType_File, file_entry->name));
					found_entry = true;
				}
			} break;
			case FileDataType_Directory:
			{
				FileSystemNodePtr new_node = CREATE_CLASS(FileSystemNode, FileDataType_Directory, this->data->name);
				FileSystemDirectoryPtr new_directory = std::static_pointer_cast<FileSystemDirectory>(new_node->data);

				FileSystemDirectoryPtr file_dir = std::static_pointer_cast<FileSystemDirectory>(this->data);
				for (auto& it : file_dir->nodes)
				{
					found_entry = it->getNodesByExtension(ext, new_node) || found_entry;
				}

				if (found_entry)
				{
					base_dir->nodes.push_back(new_node);
				}
			} break;
		}

		return found_entry;
	}

	void FileSystemNode::getFilesByExtension(StringList& list, const std::string& ext)
	{
		switch (this->type)
		{
			case FileDataType_File:
			{
				FileSystemEntryPtr file_entry = std::static_pointer_cast<FileSystemEntry>(this->data);
				assert(file_entry);
				if (file_entry->ext == ext)
				{
					list.push_back(this->data->name);
				}
			} break;
			case FileDataType_Directory:
			{

				FileSystemDirectoryPtr file_dir = std::static_pointer_cast<FileSystemDirectory>(this->data);
				for (auto& it : file_dir->nodes)
				{
					it->getFilesByExtension(list, ext);
				}
			} break;
		}
	}

	void FileSystemNode::debugPrint()
	{
		std::stringstream str;
		this->print(str, 0);
		log::info("File System:");
		log::info(str.str());
	}

	void FileSystemNode::print(std::stringstream& out, uint32 depth)
	{
		// out << std::string(depth, ' ');
		switch (this->type)
		{
			case FileDataType_File:
			{
				FileSystemEntryPtr file_entry = std::static_pointer_cast<FileSystemEntry>(this->data);
				out << "File: " << file_entry->name << std::endl;
			} break;
			case FileDataType_Directory:
			{
				FileSystemDirectoryPtr file_dir = std::static_pointer_cast<FileSystemDirectory>(this->data);
				out << "Dir: " << file_dir->name << std::endl;
				for (auto& it : file_dir->nodes)
				{
					it->print(out, depth + 1);
				}
			} break;
		}
	}

}
