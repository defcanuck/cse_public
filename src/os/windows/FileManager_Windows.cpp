#include "PCH.h"

#include "os/windows/FileManager_Windows.h"
#include "os/LogManager.h"

#include <Windows.h>
#include <iostream>
#include <clocale>
#include <cstdlib>

#include "dirent.h"

#include <string>
#include <fstream>

namespace cs
{
	FileManager* FileManager::getInstance()
	{
		if (!instance)
			instance = new FileManager_Windows();
		return instance;
	}

	FileManager_Windows::FileManager_Windows() : FileManager()
	{
		this->setSavePath(this->getExecutablePath(), true);
	}
    
    bool FileManager_Windows::getPathToFile(const std::string& fileName, std::string& fullPath)
    {
        if (!this->root)
        {
            log::error("No file system to parse!");
            return false;
        }
        
        StringList directories;
        if (this->root->getPath(fileName, directories, FileDataType_File))
        {
            fullPath = this->basePath + implode_backward(directories, FileManager::getInstance()->separator()) + fileName;
            return true;
        }

		if (this->rootAdd)
		{
			if (this->rootAdd->getPath(fileName, directories, FileDataType_File))
			{
				fullPath = implode_backward(directories, FileManager::getInstance()->separator()) + fileName;
				size_t p = fullPath.find_first_of('\\');
				fullPath = (p != std::string::npos) ? fullPath.substr(p + 1, std::string::npos) : fullPath;
				return true;
			}
		}
        return false;
    }

	void FileManager::refreshFileSystem(FileSystemNodePtr* root, const std::string& path)
	{
		DIR *dir;
		struct dirent *ent;

		assert((*root)->type == FileDataType_Directory);
		FileSystemDirectoryPtr file_dir = std::static_pointer_cast<FileSystemDirectory>((*root)->data);
		assert(file_dir);

		/* Open directory stream */
		dir = opendir(path.c_str());
		if (dir != nullptr)
		{

			/* Print all files and directories within the directory */
			while ((ent = readdir(dir)) != nullptr)
			{
				switch (ent->d_type)
				{
					case DT_REG:
					{
						std::string name = ent->d_name;
						FileSystemNodePtr child_entry = CREATE_CLASS(FileSystemNode, FileDataType_File, name);
						file_dir->nodes.push_back(child_entry);

					} break;
					case DT_DIR:
					{
						if (ent->d_namlen >= 1 && ent->d_name[0] != '.')
						{
						
							const std::string dir_name = ent->d_name;
							FileSystemNodePtr child_dir = CREATE_CLASS(FileSystemNode, FileDataType_Directory, dir_name);
							file_dir->nodes.push_back(child_dir);

							std::string dir_path = path + FileManager::getInstance()->separator() + ent->d_name;
							FileManager::refreshFileSystem(&child_dir, dir_path);
						}
					} break;

					case DT_LNK:
					default:
						log::info("Skipping %s*\n", ent->d_name);
				}
			}

			closedir(dir);
		}
	}
}
