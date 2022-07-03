#include "cbase.h"
#include "../stream_safe.h"
#include "sharedutil.h"
#include "msfileio.h"
#include "parser.h"

#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>

void StoreFile(char *pszCurrentDir, WIN32_FIND_DATA &wfd);
extern bool verbose;
msstringlist StoreFiles;

void createCopy(std::string data) {
	std::ofstream o("./test.script");
	
	//parseData(data.begin(), data.end(), outIt);
	Parser parser(data);
	parser.stripComments();
	parser.stripTabs();
	parser.stripDebug();
	parser.stripEmptyLines();
	std::string result = parser.getResult();
	o << result << std::endl;
	o.close();
}

void PackScriptDir(char *pszName)
{
	PackDirectory(pszName);

	char cWriteFile[MAX_PATH];
	_snprintf(cWriteFile, MAX_PATH, "%s\\sc.dll", pszName);

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

	for (size_t i = 0; i < StoreFiles.size(); i++)
	{
		msstring &FullPath = StoreFiles[i];
		if (InFile.ReadFromFile(FullPath))
		{
			char cRelativePath[MAX_PATH];
			strncpy(cRelativePath, &FullPath[strlen(pszName) + 1], MAX_PATH);
			
			char *cstr((char*)InFile.m_Buffer);
			std::string cppstr(cstr);
			
			createCopy(cppstr);
			
			if (verbose == true)
				printf("Doing file: %s\n", cRelativePath);

			if (!GroupFile.WriteEntry(cRelativePath, InFile.m_Buffer, InFile.m_BufferSize))
				printf("Failed to write entry: %s\n", cRelativePath);
		}
	}

	GroupFile.Flush();
	GroupFile.Close();
}

void PackDirectory(char *pszName)
{
	WIN32_FIND_DATA wfd;
	HANDLE findHandle;
	char cSearchString[MAX_PATH];
	
	_snprintf(cSearchString, MAX_PATH, "%s\\*.*", pszName);
	if ((findHandle = FindFirstFile(cSearchString, &wfd)) == INVALID_HANDLE_VALUE) 
		return;

	StoreFile(pszName, wfd);

	while (FindNextFile(findHandle, &wfd))
		StoreFile(pszName, wfd);

	FindClose(findHandle);
}

void StoreFile(char *pszCurrentDir, WIN32_FIND_DATA &wfd)
{
	char cFullPath[MAX_PATH];
	_snprintf(cFullPath, MAX_PATH, "%s\\%s", pszCurrentDir, wfd.cFileName);

	if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{		
		if (wfd.cFileName[0] != '.') PackDirectory(cFullPath); // Braces are neccesary
	}
	else if (strstr(wfd.cFileName, ".script") || !stricmp(wfd.cFileName, "items.txt"))
		StoreFiles.add(cFullPath);
}