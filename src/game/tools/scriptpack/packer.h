#pragma once

#include "sharedutil.h"
#include "msfileio.h"
#include "groupfile.h"

class Packer 
{
public:
	Packer(std::string wDir, std::string rDir, std::string oDir);
	void readDirectory(std::string pszName);
	void packScripts();
	void stripComments(std::vector<std::byte>& data);
	void stripWhiteSpace(std::vector<std::byte>& data);
	void stripEmptyLines(std::vector<std::byte>& data);
  
private:
	std::string m_WorkDir;
	std::string m_RootDir;
	std::string m_OutDir;
	std::vector<std::string> m_StoredFiles;
	std::vector<std::string> m_Errors;
	
	void processScript(std::vector<std::byte> &buffer, std::string relativeFile);

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

	std::string trim(const std::string& str) {
		// Standard whitespace characters
		const std::string whitespace = " \t\n\r\f\v";

		// Find the first character that is not whitespace
		size_t start = str.find_first_not_of(whitespace);

		// If only whitespace or empty, return an empty string
		if (std::string::npos == start) {
			return "";
		}

		// Find the last character that is not whitespace
		size_t end = str.find_last_not_of(whitespace);

		// Return the substring between start and end (inclusive)
		return str.substr(start, (end - start) + 1);
	}
};