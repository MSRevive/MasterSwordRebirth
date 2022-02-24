#include "cbase.h"
#include "iostream"
#include "../../MSShared/encrypt.h"
#include "../../MSShared/sharedutil.h"
#include "../../MSShared/msfileio.h"

using namespace std;

void StoreFile(char *pszCurrentDir, WIN32_FIND_DATA &wfd);
extern char *pszRoot;
msstringlist StoreFiles;

void PackScriptDir(char *pszName)
{
	PackDirectory(pszName);

	char cWriteFile[MAX_PATH];
	_snprintf(cWriteFile, MAX_PATH, "%s\\sc.dll", pszRoot);

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
			strncpy(cRelativePath, &FullPath[strlen(pszRoot) + 1], MAX_PATH);

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