
#include "os/FileManager.h"
#include "os/LogManager.h"

#include <dirent.h>

namespace cs
{
	void FileManager::refreshInternal(const std::string& root, FileManifestEntries& entries)
	{
		DIR *dir;
		struct dirent *ent;

		/* Open directory stream */
		dir = opendir(root.c_str());
		if (dir != nullptr)
		{

			/* Print all files and directories within the directory */
			while ((ent = readdir(dir)) != nullptr)
			{
				switch (ent->d_type)
				{
				case DT_REG:
				{
					FileEntry entry;
					std::string name = ent->d_name;
					FileManager::populateFileEntry(entry, name, root);
					FileManager::addEntry(entries, entry);

				} break;
				case DT_DIR:
				{
					if (ent->d_namlen >= 1 && ent->d_name[0] != '.')
					{
						std::string rec = root + FileManager::getInstance()->separator() + ent->d_name;
						FileManager::refreshInternal(rec, entries);
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