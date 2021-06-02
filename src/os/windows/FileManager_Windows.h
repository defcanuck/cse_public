#pragma once

#include "os/FileManager.h"

#include <string>

namespace cs
{
	class FileManager_Windows : public FileManager
	{
	public:
		FileManager_Windows();

        virtual bool getPathToFile(const std::string& fileName, std::string& fullPath);
		virtual char separator() { return '\\'; }

	private:

	};
}
