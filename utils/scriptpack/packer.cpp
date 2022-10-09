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

Packer::Packer(char *wDir, char *rDir, char *oDir)
{
	_snprintf(m_WorkDir, MAX_PATH, "%s", wDir);
	_snprintf(m_RootDir, MAX_PATH, "%s", rDir);
	_snprintf(m_OutDir, MAX_PATH, "%s", oDir);
	_snprintf(m_CookedDir, MAX_PATH, "%s\\cooked\\", rDir);

	if (g_Release)
	{
		try {
			CreateDirectory(m_CookedDir, NULL);
		}catch(...)
		{
			printf("Failed to create %s\n", m_CookedDir);
			exit(-1);
		}
	}
}

//we grab all the files in the scripts directory to get ready for packing.
void Packer::readDirectory(char *pszName, bool cooked)
{
	WIN32_FIND_DATA wfd;
	HANDLE findHandle;
	
	char cSearchString[MAX_PATH];
	_snprintf(cSearchString, MAX_PATH, "%s\\*.*", pszName);
	
	if ((findHandle = FindFirstFile(cSearchString, &wfd)) == INVALID_HANDLE_VALUE) 
		return;

	storeFile(pszName, wfd, cooked);

	while (FindNextFile(findHandle, &wfd))
		storeFile(pszName, wfd, cooked);

	FindClose(findHandle);
}

//store files info in array so we can process them at a later time.
void Packer::storeFile(char *pszCurrentDir, WIN32_FIND_DATA &wfd, bool cooked)
{
	char cFullPath[MAX_PATH];
	_snprintf(cFullPath, MAX_PATH, "%s\\%s", pszCurrentDir, wfd.cFileName);
	
	if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{		
		if (wfd.cFileName[0] != '.') 
			readDirectory(cFullPath, cooked); // Braces are neccesary
	}
	else if (strstr(wfd.cFileName, ".script") || !stricmp(wfd.cFileName, "items.txt"))
	{
		if (cooked)
			m_CookedFiles.add(cFullPath);
		else
			m_StoredFiles.add(cFullPath);
	}
}

//checks the scripts for errors and cleans them for release.
void Packer::processScripts() 
{
	CMemFile InFile;
	size_t listSize = m_StoredFiles.size();

	if(g_Release && listSize > 0)
	{	
		for(size_t i = 0; i < listSize; i++)
		{
			msstring &FullPath = m_StoredFiles[i];
			if(InFile.ReadFromFile(FullPath))
			{
				char cRelativePath[MAX_PATH];
				strncpy(cRelativePath, &FullPath[strlen(m_WorkDir) + 1], MAX_PATH);
				
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
				strncpy(cRelativePath, &FullPath[strlen(m_WorkDir) + 1], MAX_PATH);

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
	//we want to make sc.dll in via root dir.
	char cWriteFile[MAX_PATH];
	_snprintf(cWriteFile, MAX_PATH, "%s\\sc.dll", m_OutDir);
	
	struct stat info;
	if(stat(cWriteFile, &info) == 0)
		std::remove(cWriteFile);

	msstringlist tempList;
	if (g_Release)
		tempList = m_CookedFiles;
	else
		tempList = m_StoredFiles;
	
	CGroupFile GroupFile;
	try {
		GroupFile.Open(cWriteFile);
	}
	catch(...)
	{
		printf("Failed to create %s\n", cWriteFile);
		exit(-1);
	}
	
	size_t tListSize = tempList.size();
	if(tListSize > 0)
	{
		CMemFile InFile;
		for (size_t i = 0; i < tListSize; i++)
		{
			msstring &FullPath = tempList[i];
			if (InFile.ReadFromFile(FullPath))
			{
				char cRelativePath[MAX_PATH];
				strncpy(cRelativePath, &FullPath[strlen(m_WorkDir) + 1], MAX_PATH);
				
				if (g_Verbose == true)
					printf("Packing file: %s\n", cRelativePath);
	
				if (!GroupFile.WriteEntry(cRelativePath, InFile.m_Buffer, InFile.m_BufferSize))
					printf("Failed to write entry: %s\n", cRelativePath);
			}
		}
	}
	else
		std::cout << "Failed to pack scripts, exiting" << std::endl;
	
	//close and flush GroupFile
	GroupFile.Flush();
	GroupFile.Close();
}

void Packer::doParser(byte *buffer, size_t bufferSize, char *name, char *create, bool errOnly)
{
	//need buffersize + 1 to make room for the null terminator
	size_t bufSize = bufferSize+1;

	//we want to use snprintf instead of strncpy or memcpy because it applies a null terminator.
	char *ffile = new char[bufSize]();
	snprintf(ffile, bufSize, "%s", buffer);

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
		delete ffile;
		exit(-1);
	}

	//deallocate memory for object when done.
	delete ffile;
}