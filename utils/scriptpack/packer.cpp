#include <iostream>
#include <fstream>
#include <thread>
#include <sys/stat.h>

#include "cbase.h"
#include "packer.h"
#include "../stream_safe.h"
#include "parser.h"
#include "compat.h"
#include "dirent.h"

extern bool g_Verbose;
extern bool g_Release;
extern bool g_ErrFile;
extern bool g_FailOnErr;

Packer::Packer(char *wDir, char *rDir, char *oDir)
{
	_snprintf(m_WorkDir, MAX_PATH, "%s", wDir);
	_snprintf(m_RootDir, MAX_PATH, "%s", rDir);
	_snprintf(m_OutDir, MAX_PATH, "%s", oDir);
	_snprintf(m_CookedDir, MAX_PATH, "%s\\cooked\\", rDir);

	if (g_Release)
	{
		if (!Compat::makePath(std::string(m_CookedDir)))
		{
			printf("Failed to create %s\n", m_CookedDir);
			exit(-1);
		}
	}
}

void Packer::readDirectory(char *pszName, bool cooked)
{
	char cFullPath[MAX_PATH];
	DIR *dir = opendir(pszName);
	if(!dir)
		exit(-1);

	struct dirent *ent;
	while ((ent = readdir(dir)) != NULL)
	{
		strcpy(cFullPath, pszName);
		strcat(cFullPath, "/");
		strcat(cFullPath, ent->d_name);

		if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
		{
			switch (ent->d_type)
			{
			case DT_REG:
				//ignore non script files.
				if(strstr(ent->d_name, ".script") || !stricmp(ent->d_name, "items.txt"))
				{
					if(cooked)
						m_CookedFiles.add(cFullPath);
					else
						m_StoredFiles.add(cFullPath);
				}
				break;
			case DT_DIR:
				if (g_Verbose)
					printf("Reading Directory: %s\n", ent->d_name);
				
				if (g_Release && !stricmp(ent->d_name, "developer"))
					continue;

				readDirectory(cFullPath, cooked);
				break;
			default:
				break;
			}
		}
	}

	closedir(dir);
}

//checks the scripts for errors and cleans them for release.
void Packer::processScripts() 
{
	CMemFile InFile;
	size_t listSize = m_StoredFiles.size();
	printf("size: %d\n", listSize);

	if(g_Release && listSize > 0)
	{
		for(size_t i = 0; i < listSize; i++)
		{
			msstring &FullPath = m_StoredFiles[i];
			if(InFile.ReadFromFile(FullPath))
			{
				char cRelativePath[MAX_PATH];
				strncpy(cRelativePath, &(FullPath.c_str()[strlen(m_WorkDir) + 1]), MAX_PATH);
				
				char createFile[MAX_PATH];
				_snprintf(createFile, MAX_PATH, "%s%s", m_CookedDir, cRelativePath);
				
				if (g_Verbose)
					printf("Cleaning script: %s\n", cRelativePath);
				
				std::thread parserThread(&Packer::doParser, this, InFile.m_Buffer, InFile.m_BufferSize, cRelativePath, createFile, false);
				parserThread.join();
					
				if (g_Verbose)
					printf("End script cleaning: %s\n\n", cRelativePath);
			}
		}
		
		//read the cooked dir once scripts are cooked.
		readDirectory(m_CookedDir, true);
	}
	else if (!g_Release && listSize > 0)
	{
		for(size_t i = 0; i < listSize; i++)
		{
			msstring &FullPath = m_StoredFiles[i];
			if(InFile.ReadFromFile(FullPath))
			{
				char cRelativePath[MAX_PATH];
				strncpy(cRelativePath, &(FullPath.c_str()[strlen(m_WorkDir) + 1]), MAX_PATH);

				if (g_Verbose)
					printf("Error checking script: %s\n", cRelativePath);

				std::thread parserThread(&Packer::doParser, this, InFile.m_Buffer, InFile.m_BufferSize, cRelativePath, FullPath, true);
				parserThread.join();

				if (g_Verbose)
					printf("End script processing: %s\n\n", cRelativePath);
			}
		}
	}
}

//packs the scripts.
void Packer::packScripts()
{
	//we want to make scripts.pak in via root dir.
	char cWriteFile[MAX_PATH];
	_snprintf(cWriteFile, MAX_PATH, "%s\\scripts.pak", m_OutDir);
	
	struct stat info;
	if(stat(cWriteFile, &info) == 0)
		std::remove(cWriteFile);

	FILE* fp = fopen(cWriteFile, "wb+");

	if (fp == NULL)
	{
		printf("Failed to create %s\n", cWriteFile);
		exit(-1);
	}

	msstringlist files;

	if (g_Release)
	{
		files = m_CookedFiles;
	}
	else
	{
		files = m_StoredFiles;
	}

	pakHeader_t Header;
	Header.MagicNumber = 1262698832;
	Header.DirectoryOffset = sizeof(pakHeader_t);
	Header.DirectoryCount = files.size();

	pakDirectory_t dummy;
	strcpy(dummy.cFilename, "");
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

	if (listSize > 0)
	{
		for (size_t i = 0; i < listSize; i++)
		{
			msstring &FullPath = files[i];
			if (InFile.ReadFromFile(FullPath))
			{
				pakDirectory_t File;
				strncpy(File.cFilename, &(FullPath.c_str()[strlen(m_WorkDir) + 1]), sizeof(File.cFilename));
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
		std::cout << "ERROR: No files found!" << std::endl;
		exit(-1);
	}
	
	// close and flush
	fflush(fp);
	fclose(fp);
}

void Packer::doParser(byte *buffer, size_t bufferSize, char *name, char *create, bool errOnly)
{
	//need buffersize + 1 to make room for the null terminator
	size_t bufSize = bufferSize+1;

	//we want to use snprintf instead of strncpy or memcpy because it applies a null terminator.
	char *ffile = new char[bufSize]();
	snprintf(ffile, bufSize, "%s", buffer);

	if (!stricmp(name, "items.txt") && !errOnly)
	{
		Parser parser(ffile, name);
		parser.saveResult(create);
	}
	else
	{
		//we create parser object.
		Parser parser(ffile, name);
		parser.stripComments();

		//we check for errors here because comments were already replaced.
		parser.checkQuotes(); //check for quote errors
		parser.checkBrackets(); //check for closing errors

		//only run this stuff if we're doing full parser.
		if (!errOnly)
		{
			parser.stripWhitespace();
			parser.stripDebug();
		}

		//do error print at the end
		parser.printErrors();
		if (g_ErrFile)
			parser.saveErrors();

		if (!errOnly)
			parser.saveResult(create);

		if (g_FailOnErr && parser.errorCheck())
		{
			delete[] ffile;
			exit(-1);
		}
	}

	//deallocate memory for object when done.
	delete[] ffile;
}