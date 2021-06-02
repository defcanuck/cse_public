#pragma once

#include "os/FileManager.h"

#include <string>

namespace cs
{
	class FileManager_iOS : public FileManager
	{
	public:
		FileManager_iOS();

        virtual char separator() { return '/'; }
        virtual bool getPathToFile(const std::string& fileName, std::string& fullPath);
        
	private:

	};
}
