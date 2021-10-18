#include "cbase.h"
#include "iostream"
#include "../../MSShared/encrypt.h"
#include "../../MSShared/sharedutil.h"
#include "../../MSShared/msfileio.h"

using namespace std;

void StoreFile(char *pszCurrentDir, WIN32_FIND_DATA &wfd);
//extern HANDLE g_resHandle;
extern char *pszRoot;
//#define FILE_SCDLL "H:/MS/dlls/sc.dll"
msstringlist StoreFiles;

void PackScriptDir(char *pszName)
{
	PackDirectory(pszName);

	char cWriteFile[MAX_PATH];
	sprintf(cWriteFile, "%s\\sc.dll", pszRoot);

	CGroupFile GroupFile;
	GroupFile.Open(cWriteFile);

	CMemFile InFile;

	for (int i = 0; i < StoreFiles.size(); i++)
	{
		msstring &FullPath = StoreFiles[i];
		if (InFile.ReadFromFile(FullPath))
		{/*
		 CEncryptData1 Data;
		 Data.SetData( InFile.m_Buffer, InFile.m_BufferSize );
		 Data.Encrypt( );*/

			char cRelativePath[MAX_PATH];
			strcpy(cRelativePath, &FullPath[strlen(pszRoot) + 1]);

			if (!GroupFile.WriteEntry(cRelativePath, InFile.m_Buffer, InFile.m_BufferSize))
				Print("Failed to write entry: %s\n", cRelativePath);
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
	sprintf(cSearchString, "%s\\*.*", pszName);
	if ((findHandle = FindFirstFile(cSearchString, &wfd)) == INVALID_HANDLE_VALUE) return;

	StoreFile(pszName, wfd);

	while (FindNextFile(findHandle, &wfd))
		StoreFile(pszName, wfd);

	FindClose(findHandle);
}
void StoreFile(char *pszCurrentDir, WIN32_FIND_DATA &wfd)
{
	char cFullPath[MAX_PATH];
	sprintf(cFullPath, "%s\\%s", pszCurrentDir, wfd.cFileName);

	if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		//Braces are neccesary
		if (wfd.cFileName[0] != '.') PackDirectory(cFullPath);
	}
	else if (strlen(wfd.cFileName) > strlen(".script") &&
		!stricmp(&wfd.cFileName[strlen(wfd.cFileName) - strlen(".script")], ".script") ||
		!stricmp(wfd.cFileName, "items.txt"))
	{
		StoreFiles.add(cFullPath);

		/*HANDLE hFile = CreateFile( cFullPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
		if( hFile != INVALID_HANDLE_VALUE )
		{
		DWORD dwSize = GetFileSize( hFile, NULL );
		byte *pFileData = new(byte[dwSize]);
		if( !pFileData )
		{
		Print( "Out of memory!!\n" );
		exit( 1 );
		}
		DWORD dwBytesRead = 0;
		while( dwBytesRead < dwSize )
		ReadFile( hFile, pFileData, dwSize, &dwBytesRead, NULL );

		CEncryptData1 Data;
		Data.SetData( pFileData, dwSize );
		Data.Encrypt( );
		char cRelativePath[MAX_PATH];
		strcpy( cRelativePath, &cFullPath[strlen(pszRoot)+1] );
		byte *pNew = new(byte[Data.GetDataSize()]);
		memcpy( pNew, Data.GetData(), Data.GetDataSize() );
		char cWriteFile[MAX_PATH];
		sprintf( cWriteFile, "%s\\sc.dll", pszRoot );


		//Print( "%s - %s\n", cWriteFile, cRelativePath );
		CGroupFile GroupFile;
		GroupFile.Open( cWriteFile );
		if( !GroupFile.WriteEntry( cRelativePath, pNew, Data.GetDataSize() ) )
		Print( "Failed to write entry: %s\n", cRelativePath );

		delete pFileData;
		CloseHandle( hFile );
		}*/
	}
}