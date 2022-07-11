#include <iostream>
#include <fstream>
#include <thread>

#include "cbase.h"
#include "packer.h"
#include "../stream_safe.h"
#include "parser.h"

extern bool g_Verbose;
extern bool g_Release;
extern bool g_ErrFile;

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

void Packer::cookScripts() 
{
	if(g_Release == true && m_StoredFiles.size() > 0)
	{	
		CMemFile InFile;
		for(size_t i = 0; i < m_StoredFiles.size(); i++)
		{
			msstring &fullPath = m_StoredFiles[i];
			if(InFile.ReadFromFile(fullPath))
			{
				char cRelativePath[MAX_PATH];
				strncpy(cRelativePath, &fullPath[strlen(m_RootDir) + 1], MAX_PATH);
				
				char createFile[MAX_PATH];
				strncpy(createFile, m_CookedDir, MAX_PATH);
				strncat(createFile, cRelativePath, MAX_PATH);
				
				if (g_Verbose == true)
					printf("Cleaning script: %s\n", cRelativePath);
				
				//convert char array to std::string for parser.
				std::thread parserThread(&Packer::doParser, this, (char*)InFile.m_Buffer, cRelativePath, createFile, false);
				parserThread.join();
					
				if (g_Verbose == true)
					printf("End script cleaning: %s\n", cRelativePath);
			}

			delete InFile.m_Buffer; //CMemFile doesn't clear it's own buffer so we have to do it :(
		}
		
		//read and store cooked directory scripts
		readDirectory(m_CookedDir, true);
	}
}

void Packer::packScripts()
{
	//we want to make sc.dll in via root dir.
	char cWriteFile[MAX_PATH];
	_snprintf(cWriteFile, MAX_PATH, "%s\\sc.dll", m_OutDir);
	
	struct stat info;
	if(stat(cWriteFile, &info) == 0)
		std::remove(cWriteFile);
	
	CGroupFile GroupFile;
	try {
		GroupFile.Open(cWriteFile);
	}
	catch(...)
	{
		printf("Failed to create %s\n", cWriteFile);
		exit(-1);
	}
	
	if(g_Release == true && m_CookedFiles.size() > 0)
	{
		CMemFile InFile;
		for (size_t i = 0; i < m_CookedFiles.size(); i++)
		{
			msstring &FullPath = m_CookedFiles[i];
			if (InFile.ReadFromFile(FullPath))
			{
				char cRelativePath[MAX_PATH];
				strncpy(cRelativePath, &FullPath[strlen(m_RootDir) + 1], MAX_PATH);
				
				if (g_Verbose == true)
					printf("Doing file: %s\n", cRelativePath);
	
				if (!GroupFile.WriteEntry(cRelativePath, InFile.m_Buffer, InFile.m_BufferSize))
					printf("Failed to write entry: %s\n", cRelativePath);
			}
			delete InFile.m_Buffer; //CMemFile doesn't clear it's own buffer so we have to do it :(
		}
	}
	else if(m_StoredFiles.size() > 0)
	{
		std::cout << "CookedFiles is 0 or release turned off." << "\n" << std::endl;
	
		CMemFile InFile;
		for (size_t i = 0; i < m_StoredFiles.size(); i++)
		{
			msstring &FullPath = m_StoredFiles[i];
			if (InFile.ReadFromFile(FullPath))
			{
				char cRelativePath[MAX_PATH];
				strncpy(cRelativePath, &FullPath[strlen(m_RootDir) + 1], MAX_PATH);
				
				if (g_Verbose == true)
					printf("Doing file: %s\n", cRelativePath);
				
				//perform error check
				std::thread parserThread(&Packer::doParser, this, (char*)InFile.m_Buffer, cRelativePath, FullPath, true);
				parserThread.join();
	
				if (!GroupFile.WriteEntry(cRelativePath, InFile.m_Buffer, InFile.m_BufferSize))
					printf("Failed to write entry: %s\n", cRelativePath);
			}
			delete InFile.m_Buffer; //CMemFile doesn't clear it's own buffer so we have to do it :(
		}
	}
	else
		std::cout << "Failed to pack scripts, exiting" << std::endl;
	
	//close and flush GroupFile
	GroupFile.Flush();
	GroupFile.Close();
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

void Packer::doParser(char *file, char *name, char *create, bool errOnly)
{
	//convert char array to std::string for parser.
	char *cstr(file);
	std::string data(cstr);

	//we create parser object.
	Parser parser(data, name);
	parser.stripComments();

	//we check for errors here because comments were already replaced.
	//parser.checkQuotes(); //check for quote errors
	parser.checkBrackets(); //check for closing errors

	//only run this stuff if we're doing full parser.
	if (!errOnly)
	{
		parser.stripTabs();
		parser.stripDebug();
		parser.stripEmptyLines();
	}

	//do error print at the end
	parser.printErrors();
	if (g_ErrFile)
		parser.saveErrors();

	if (!errOnly)
		parser.saveResult(create);
}