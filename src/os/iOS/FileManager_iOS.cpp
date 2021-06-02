#include "PCH.h"

#include "os/iOS/FileManager_iOS.h"
#include "os/LogManager.h"

#include <iostream>
#include <clocale>
#include <cstdlib>

#include "dirent.h"

#include <string>
#include <fstream>

#include "Platform_iOS.h"

namespace cs
{
	FileManager* FileManager::getInstance()
	{
		if (!instance)
			instance = new FileManager_iOS();
		return instance;
	}

	FileManager_iOS::FileManager_iOS()
    : FileManager()
	{
        const char* savePath = GetSavePath();
        size_t sz = strlen(savePath);
        char* appendStr = new char[sz + 2];
        memcpy(appendStr, savePath, sz);
        appendStr[sz + 0] = separator();
        appendStr[sz + 1] = '\0';
        
		this->setSavePath(appendStr, true);
        delete [] appendStr;
	}

	void FileManager::refreshFileSystem(FileSystemNodePtr* root, const std::string& path)
	{
        log::info("Refreshing bundle file system");
        int count = 0;
        char** fileList = 0;
        
        assert((*root)->type == FileDataType_Directory);
        FileSystemDirectoryPtr file_dir = std::static_pointer_cast<FileSystemDirectory>((*root)->data);
        assert(file_dir);

        GetFileList(&count, &fileList);
        for (int i = 0; i < count; ++i)
        {
            FileSystemNodePtr child_entry = CREATE_CLASS(FileSystemNode, FileDataType_File, fileList[i]);
            file_dir->nodes.push_back(child_entry);
            free(fileList[i]);
        }
        
        free(fileList);
	}
    
    bool FileManager_iOS::getPathToFile(const std::string& fileName, std::string& fullPath)
    {
        /*
        std::string ext = FileManager::getExtension(fileName);
        std::string name = FileManager::stripExtension(fileName);
        
        const char* path = GetPathToFile(fileName.c_str(), ext.c_str());
        if (strlen(path) > 0)
        {
            fullPath = path;
            return true;
        }
         */
        fullPath = this->getBasePath() + this->separator() + fileName;
        return true;
    }
}
