#pragma once

#include <string>
#include <unordered_map>
#include <functional>

#include "ClassDef.h"
#include "global/Values.h"
#include "os/FileSystem.h"

namespace cs
{
	struct DataStream
	{
		DataStream() { }
		virtual ~DataStream() { }

		virtual std::istream& getStream() = 0;
	};

	struct FileStream : public DataStream
	{
		FileStream() { }

		virtual ~FileStream()
		{
			this->stream.close();
		}

		bool open(const std::string& path);

		inline bool isOpen() const { return this->stream.is_open(); }
		void close() { this->stream.close(); }

		virtual std::istream& getStream() { return this->stream; }

		std::ifstream stream;
		std::string fileName;
		std::string filePath;
	};

	class FileManager
	{
	public:
		FileManager() :
			basePath(""),
			gamePath("")
		{

		}

		~FileManager();

		static FileManager* getInstance();

		StringList test(const std::string& ext)
		{
			StringList list;
			list.push_back(ext);
			return list;
			
			//return std::string("HELLO");
		}

		virtual char separator() = 0;

		const FileSystemEntryPtr getFile(const std::string& fileName);
		
		void setBasePath(const std::string& root, bool absolute = false);
		void setSavePath(const std::string& save, bool absolute = false);

		void refreshFromBasePath();
		const std::string& getBasePath() const { return this->basePath; }
		const std::string& getSavePath() const { return this->savePath; }

		void printFileSystem();

		StringList getFilesByExtension(const std::string& fileExt);
		FileSystemNodePtr getNodesByExtension(const std::string& fileExt);

		void getFilesByExtensionRef(StringList& list, const std::string& fileExt);

        virtual bool getPathToFile(const std::string& fileName, std::string& path) { return false; }
		bool addPathToFile(const std::string& filePath, std::string& fileName);
		
		static bool splitPath(const std::string& fullFilePath, std::string& fileName, std::string& filePath);
		static std::string getExtension(const std::string& fileName);
		static std::string stripExtension(const std::string& fileName);
		
		virtual bool openStreamFile(const std::string& fileName, DataStream** stream);
		virtual bool openStreamPath(const std::string& path, DataStream** stream);

		static void setExecutablePath(const std::string path)
		{
			FileManager::exePath = path;
		}
		
		static bool hasExecutablePath()
		{
			return FileManager::exePath.length() > 0;
		}

		static const std::string& getExecutablePath() 
		{ 
			return FileManager::exePath;
		}

		static bool isSeparator(char c);
		static void flipSeparator(std::string& path);

	private:

		static std::string exePath;

		static FileManager* instance;

		static void addEntry(FileSystemNodePtr root, const std::string& filePath);
		static void refreshFileSystem(FileSystemNodePtr* root, const std::string& path);

	protected:

		std::string basePath;
		std::string gamePath;
		std::string savePath;

		FileSystemNodePtr root;
		FileSystemNodePtr rootAdd;
	};

}
