#pragma once

#include "sharedutil.h"
#include "msfileio.h"
#include "groupfile.h"

typedef unsigned char byte;

class Packer 
{
public:
	Packer(const char *wDir, const char* rDir, const char* oDir);
	void readDirectory(const char *pszName, bool cooked = false);
	void catalogScripts();
	void packScripts();
  
private:
	char m_WorkDir[MAX_PATH];
	char m_RootDir[MAX_PATH];
	char m_CookedDir[MAX_PATH];
	char m_OutDir[MAX_PATH];
	std::vector<std::string> m_StoredFiles;
	std::vector<std::string> m_CookedFiles;
	
	void processScript(const char *buffer, size_t bufferSize, std::string relativeFile, std::string createFile, bool errOnly);

	std::vector<std::byte> getFileContents(const std::string &path)
	{
		std::ifstream file(path, std::ios::binary);
		if (!file.is_open()) {
			// Handle error: file could not be opened
			return {}; // Return an empty vector
		}
		size_t fsize = std::filesystem::file_size(path);
		std::vector<std::byte> buffer(fsize);
		file.read(reinterpret_cast<char*>(buffer.data()), fsize);

		return buffer;
	}
};