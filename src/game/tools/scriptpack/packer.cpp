#include <iostream>
#include <fstream>
#include <thread>
#include <sys/stat.h>

#include "cbase.h"
#include "packer.h"
#include "../stream_safe.h"
#include "parser.h"
#include "dirent.h"

extern bool g_Verbose;
extern bool g_Release;
extern bool g_ErrFile;
extern bool g_FailOnErr;

Packer::Packer(const char* wDir, const char* rDir, const char* oDir)
{
	_snprintf(m_WorkDir, MAX_PATH, "%s", wDir);
	_snprintf(m_RootDir, MAX_PATH, "%s", rDir);
	_snprintf(m_OutDir, MAX_PATH, "%s", oDir);
	_snprintf(m_CookedDir, MAX_PATH, "%s\\cooked", rDir);

	std::cout << "Work directory set to: " << m_WorkDir << std::endl;
	std::cout << "Cooked directory set to: " << m_CookedDir << std::endl;

	if (g_Release)
	{
		std::filesystem::path fsPath = m_CookedDir;
		std::error_code ec;
		std::filesystem::create_directories(fsPath, ec);
		if (ec) {
			std::cout << "ERROR: creating directories for " << fsPath << ": " << ec.message() << std::endl;
			exit(-1);
		}
	}
}

void Packer::readDirectory(const char *pszName, bool cooked)
{
	std::filesystem::path fsPath = pszName;

	for(const auto &entry : std::filesystem::recursive_directory_iterator(fsPath))
	{
		if (g_Release && entry.path().parent_path().filename() == "developer")
			continue;

		if (g_Verbose && entry.is_directory())
		{
			std::cout << "Reading Directory: " << entry.path().string() << std::endl;
		}

		if( entry.is_regular_file() )
		{
			auto file = entry.path();
			if(file.extension() == ".script" || file.extension() == ".as" || file.filename() == "items.txt")
			{
				if( g_Verbose )
					std::cout << file.string() << std::endl;

				if(cooked)
					m_CookedFiles.push_back(file.string());
				else
					m_StoredFiles.push_back(file.string());
			}
		}
	}
}

//checks the scripts for errors and cleans them for release.
void Packer::catalogScripts() 
{
	CMemFile InFile;
	printf("size: %d\n", m_StoredFiles.size());

	if(m_StoredFiles.size() > 0)
	{
		for(std::string &file : m_StoredFiles)
		{
			if(InFile.ReadFromFile(file.c_str()))
			{
				std::cout << file << std::endl;
				std::string relativePath = file.substr(strlen(m_WorkDir) + 1, file.length());
				std::cout << relativePath << std::endl;

				if (g_Verbose)
					std::cout << "Cataloging script: " << relativePath << std::endl;
				
				if (g_Release)
				{
					std::string cookedFile = m_CookedDir;
					cookedFile += "/";
					cookedFile += relativePath;

					std::thread parserThread(&Packer::processScript, this, InFile.m_Buffer, InFile.m_BufferSize, relativePath, cookedFile, false);
					parserThread.join();
				}
				else
				{
					std::thread parserThread(&Packer::processScript, this, InFile.m_Buffer, InFile.m_BufferSize, relativePath, file, false);
					parserThread.join();
				}
			}
		}

		if (g_Release)
		{
			readDirectory(m_CookedDir, true);
		}
	}
}

// packs the scripts.
// we have to use c strings here because MSR does.
void Packer::packScripts()
{
	char cWriteFile[MAX_PATH];
	_snprintf(cWriteFile, MAX_PATH, "%s\\scripts.pak", m_OutDir);

	if(std::filesystem::exists(cWriteFile))
		std::remove(cWriteFile);

	FILE* fp = fopen(cWriteFile, "wb+");

	if (fp == NULL)
	{
		printf("Failed to create %s\n", cWriteFile);
		exit(-1);
	}

	std::vector<std::string> files;

	size_t baseDirLen = 0;
	if (g_Release)
	{
		files = m_CookedFiles;
		baseDirLen = strlen(m_CookedDir)-1;
		std::cout << "RELEASE " << baseDirLen << std::endl;
	}	
	else
	{
		files = m_StoredFiles;
		baseDirLen = strlen(m_WorkDir)-1;
		std::cout << "NO-RELEASE " << baseDirLen << std::endl;
	}
		

	pakHeader_t Header;
	Header.MagicNumber = 1262698832;
	Header.DirectoryOffset = sizeof(pakHeader_t);
	Header.DirectoryCount = files.size();

	pakDirectory_t dummy;
	strncpy(dummy.cFilename, "", 0);
	dummy.FileSize = 0;
	dummy.FileOffset = 0;

	// write the file header
	fwrite(&Header, sizeof(pakHeader_t), 1, fp);

	CMemFile InFile;
	size_t listSize = files.size();

	// write out dummy data to occupy the file metadata position
	for (int i = 0; i < listSize; i++)
		fwrite(&dummy, sizeof(pakDirectory_t), 1, fp);

	size_t currentFileBytesWritten = ftell(fp);

	// jump back to just after the header
	fseek(fp, sizeof(pakHeader_t), SEEK_SET);

	if (files.size() > 0 && baseDirLen > 0)
	{
		for(std::string &file : files)
		{
			if (InFile.ReadFromFile(file.c_str()))
			{
				std::cout << file << std::endl;
				std::string relativePath = file.substr(baseDirLen, file.length());
				std::cout << relativePath << std::endl;

				pakDirectory_t File;
				strncpy(File.cFilename, &(file.c_str()[baseDirLen]), sizeof(File.cFilename)); // was strlen(m_WorkDir)+1 but that's not the correct index anymore?
				File.FileOffset = 0;
				File.FileSize = InFile.m_BufferSize;

				if (g_Verbose == true)
					printf("Packing file: %s\n", File.cFilename);

				// remember where this entry is
				size_t currentPosition = ftell(fp);

				// write this entry
				size_t ObjectsWritten = fwrite(&File, sizeof(pakDirectory_t), 1, fp);

				if (ObjectsWritten != 1)
					printf("Failed to write entry: %s\n", File.cFilename);

				// jump to where the file data should be
				fseek(fp, currentFileBytesWritten, SEEK_SET);

				// remember the current position as where this file starts
				File.FileOffset = ftell(fp);

				// write the file data
				ObjectsWritten = fwrite(InFile.m_Buffer, InFile.m_BufferSize, 1, fp);

				if (ObjectsWritten != 1)
					printf("Failed to write file: %s\n", File.cFilename);

				// remember the current position as where to pick up from when writing the next file
				currentFileBytesWritten = ftell(fp);

				// jump back to the entry
				fseek(fp, currentPosition, SEEK_SET);

				// write the entry with the correct FileOffset
				ObjectsWritten = fwrite(&File, sizeof(pakDirectory_t), 1, fp);
		
				if (ObjectsWritten != 1)
					printf("Failed to write entry: %s\n", File.cFilename);
			}
		}
	}
	else
	{
		std::cout << "ERROR: No files in list!" << std::endl;
		exit(-1);
	}

	fflush(fp);
	fclose(fp);
}

void Packer::processScript(byte *buffer, size_t bufferSize, std::string relativeFile, std::string createFile, bool errOnly)
{
	//need buffersize + 1 to make room for the null terminator
	size_t bufSize = bufferSize+1;

	//we want to use snprintf instead of strncpy or memcpy because it applies a null terminator.
	char *ffile = new char[bufSize]();
	_snprintf(ffile, bufSize, "%s", buffer);

	if (relativeFile == "items.txt" && !errOnly)
	{
		Parser parser(ffile, relativeFile);
		parser.saveResult(createFile);
	}
	else
	{
		//we create parser object.
		Parser parser(ffile, relativeFile);
		parser.stripComments();

		// Preprocess module syntax before error checking
		parser.preprocessModules();

		//we check for errors here because comments were already replaced.
		parser.checkQuotes(); //check for quote errors
		//parser.checkBrackets(); //check for closing errors

		//only run this stuff if we're doing full parser.
		if (!errOnly)
		{
			parser.stripWhiteSpace();
			parser.stripDebug();
		}

		//do error print at the end
		parser.printErrors();
		if (g_ErrFile)
			parser.saveErrors();

		if (!errOnly)
			parser.saveResult(createFile);

		if (g_FailOnErr && parser.errorCheck())
		{
			delete[] ffile;
			exit(-1);
		}
	}

	//deallocate memory for object when done.
	delete[] ffile;
}