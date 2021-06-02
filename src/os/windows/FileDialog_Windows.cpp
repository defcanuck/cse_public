#include "PCH.h"

#include "os/FileDialog.h"
#include "global/Values.h"

#include <windows.h>
#include <Commdlg.h>

#include <sstream>
#include <algorithm>

namespace cs
{

	static OPENFILENAME ofn;

	char szFile[MAX_PATH];
	static char strPath[1024];

	wchar_t *convertCharArrayToLPCWSTR(const char* charArray)
	{
		wchar_t* wString = new wchar_t[4096];
		MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
		return wString;
	}

	void prepareFilterString(char* dst, std::stringstream& oss)
	{
		strcpy(dst, oss.str().c_str());
		size_t len = strlen(dst);
		for (size_t i = 0; i < len; i++)
			if (dst[i] == '|') dst[i] = '\0';
	}

	std::string openFileLoadDialog(StringList& desc, StringList& ext)
	{

		std::stringstream ss;
		size_t iter = min(desc.size(), ext.size());
		for (size_t i = 0; i < iter; i++)
		{
			const std::string& d = desc[i];
			const std::string& e = ext[i];

			ss << d << " (*." << e << ")|*." << e << "|";
		}
		ss << "All Files(*.*)|*.*||";

		prepareFilterString(strPath, ss);

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = szFile;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = strPath;
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	
		GetOpenFileName(&ofn);

		return std::string(szFile);
	}

	std::string openFileSaveDialog(const std::string& desc, const std::string& ext)
	{
		static char strPath[1024];
		
		std::stringstream ss;
		ss << desc << " (*." << ext << ")|*." << ext << "|All Files (*.*)|*.*||";
		
		prepareFilterString(strPath, ss);

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFilter = strPath;
		// ofn.lpstrFilter = "Text Files (*.txt)|*.txt|All Files (*.*)|*.*|";
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
		ofn.lpstrDefExt = ext.c_str();

		GetSaveFileName(&ofn);

		return std::string(ofn.lpstrFile);
	}
}