#include <iostream>
#include <fstream>

#include "cbase.h"
#include "packer.h"
#include "../stream_safe.h"
#include "parser.h"
#include "groupfile.h"

extern bool verbose;
extern bool release;

void createCopy(std::string data) {
	std::ofstream o("./test.script");
	
	Parser parser(data);
	parser.stripComments();
	parser.stripTabs();
	parser.stripDebug();
	parser.stripEmptyLines();
	std::string result = parser.getResult();
	o << result << std::endl;
	o.close();
}

void Packer::cookScripts() {
	if(release == true) 
	{
		CMemFile inFile;
		for(size_t i = 0; i < m_StoredFiles.size(); i++)
		{
			msstring &fullPath = m_StoredFiles[i];
			if(inFile.ReadFromFile(fullPath))
			{
				char cRelativePath[MAX_PATH];
				strncpy(cRelativePath, &fullPath[strlen(m_RootDir) + 1], MAX_PATH);
				
				//convert char array to std::string for parser.
				char *cstr((char*)inFile.m_Buffer);
				std::string cppstr(cstr);
				
				if (verbose == true)
					printf("Doing file: %s\n", cRelativePath);
			}
		}
	}
}

//we grab all the files in the scripts directory to get ready for packing.
void Packer::readDirectory(char *pszName)
{
	WIN32_FIND_DATA wfd;
	HANDLE findHandle;
	
	char cSearchString[MAX_PATH];
	_snprintf(cSearchString, MAX_PATH, "%s\\*.*", pszName);
	
	if ((findHandle = FindFirstFile(cSearchString, &wfd)) == INVALID_HANDLE_VALUE) 
		return;

	storeFile(pszName, wfd);

	while (FindNextFile(findHandle, &wfd))
		storeFile(pszName, wfd);

	FindClose(findHandle);
}

void Packer::packScripts()
{
	readDirectory(m_WorkDir);
	
	//we want to make sc.dll in via root dir.
	char cWriteFile[MAX_PATH];
	_snprintf(cWriteFile, MAX_PATH, "%s\\sc.dll", m_RootDir);

	CGroupFile GroupFile;
	try {
		GroupFile.Open("sc.dll");
	}
	catch(...)
	{
		printf("Failed to create %s\n", cWriteFile);
		exit(-1);
	}

	CMemFile InFile;

	for (size_t i = 0; i < m_StoredFiles.size(); i++)
	{
		msstring &FullPath = m_StoredFiles[i];
		if (InFile.ReadFromFile(FullPath))
		{
			char cRelativePath[MAX_PATH];
			strncpy(cRelativePath, &FullPath[strlen(m_RootDir) + 1], MAX_PATH);
			
			// char *cstr((char*)InFile.m_Buffer);
			// std::string cppstr(cstr);
			
			//createCopy(cppstr);
			
			if (verbose == true)
				printf("Doing file: %s\n", cRelativePath);

			if (!GroupFile.WriteEntry(cRelativePath, InFile.m_Buffer, InFile.m_BufferSize))
				printf("Failed to write entry: %s\n", cRelativePath);
		}
	}

	GroupFile.Flush();
	GroupFile.Close();
}

//store files info in array so we can process them at a later time.
void Packer::storeFile(char *pszCurrentDir, WIN32_FIND_DATA &wfd)
{
	char cFullPath[MAX_PATH];
	_snprintf(cFullPath, MAX_PATH, "%s\\%s", pszCurrentDir, wfd.cFileName);
	
	if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{		
		if (wfd.cFileName[0] != '.') readDirectory(cFullPath); // Braces are neccesary
	}
	else if (strstr(wfd.cFileName, ".script") || !stricmp(wfd.cFileName, "items.txt"))
		m_StoredFiles.add(cFullPath);
}