#ifdef _WIN32
#include "windows.h"
#else
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <linux/sys.h>

//      typedef void *HANDLE;
typedef unsigned long DWORD;
typedef unsigned int uint;
typedef short int sint;

#define INVALID_HANDLE_VALUE ((HANDLE)((long)-1))
#define GENERIC_READ (0x80000000L)
#define GENERIC_WRITE (0x40000000L)
#define FILE_FLAG_SEQUENTIAL_SCAN 0x08000000
#define CREATE_ALWAYS 2
#define OPEN_EXISTING 3
#define OPEN_ALWAYS 4
#define FILE_SHARE_READ 0x00000001
#define FILE_SHARE_WRITE 0x00000002
#define FILE_BEGIN 0

FHANDLE CreateFile(char *file, uint mode, uint fProps, void *nul, uint opAlways, uint fileAtts, void *devnul)
{
	const char *modeStr;
	// calculate the proper mode - read/write from start of file, read from start, or write from start
	if (mode & GENERIC_READ)
	{
		if (mode & GENERIC_WRITE)
			modeStr = "r+";
		else
			modeStr = "r";
	}
	else
		modeStr = "w";
	FHANDLE fh;
	fh = fopen(file, modeStr);
	return fh;
}

//#undef CreateFile
//#define CreateFile sys_CreateFile

uint GetFileSize(FHANDLE file, int null)
{
	// save position
	sint pos = ftell(file);
	// go to end
	fseek(file, 0, SEEK_END);
	// get size (position)
	sint size = ftell(file);
	// return to previous position
	fseek(file, pos, SEEK_SET);
	return size;
}

#endif

#include "encrypt.h"
#include "GroupFile.h"
#include "msfileio.h"
//Deuplicated from msdebug.h
#ifdef NOT_HLDLL
#define msnew new
#elif DEV_BUILD
void *operator new(size_t size, const char *pszSourceFile, int LineNum);
void operator delete(void *ptr, const char *pszSourceFile, int LineNum);
#define msnew new (__FILE__, __LINE__)
#else
#define msnew new
#endif

void Print(char *szFmt, ...);

//namespace GroupFile
//{
//	void ReplaceChar( char *pString, char org, char dest );
//}

//Groupfile... its just like a pakfile
void CGroupFile::Open(char *pszFileName)
{
	 strncpy(m_FileName,  pszFileName, sizeof(m_FileName) );
	m_EntryList.clear();

	CMemFile GroupFile;
	if (GroupFile.ReadFromFile(m_FileName))
	{
		CEncryptData1 Data;
		Data.SetData(GroupFile.m_Buffer, GroupFile.GetFileSize());
		if (!Data.Decrypt())
			return;
		CMemFile DecryptedFile;
		DecryptedFile.SetBuffer(Data.GetData(), Data.GetDataSize());

		int HeaderEntries;

		DecryptedFile.ReadInt(HeaderEntries);
		//Read headers
		for (int i = 0; i < HeaderEntries; i++)
		{
			cachedentry_t Entry;
			DecryptedFile.Read(&Entry, sizeof(groupheader_t)); //Read only the groupheader_t part.  The cachedentry_t part is not stored
			Entry.Data = NULL;

			m_EntryList.add(Entry);
		}

		//Read existing data
		for (int i = 0; i < m_EntryList.size(); i++)
		{
			cachedentry_t &Entry = m_EntryList[i];

			groupheader_t ReadEntry;
			DecryptedFile.SetReadPtr(Entry.DataOfs);
			Entry.Data = msnew byte[Entry.DataSize];
			DecryptedFile.Read(Entry.Data, Entry.DataSize);
		}

		m_IsOpen = true;
	}
}

void CGroupFile::Close()
{
	for (int i = 0; i < m_EntryList.size(); i++)
	{
		cachedentry_t &Entry = m_EntryList[i];
		if (Entry.Data)
			delete Entry.Data;
		Entry.Data = NULL;
	}
	m_EntryList.clear();
	m_IsOpen = false;
}

//You must call Flush() to actually write the entries
bool CGroupFile::WriteEntry(const char *pszName, byte *pData, unsigned long DataSize)
{
	msstring EntryName = pszName;
	ReplaceChar(EntryName, '\\', '/');

	DeleteEntry(EntryName);

	/*CEncryptData1 Data;
	Data.SetData( pData, DataSize );
	Data.Encrypt( );*/

	cachedentry_t Entry;
	Entry.FileName = EntryName;

	//Entry.DataSize = Data.GetDataSize();
	Entry.DataSize = DataSize;

	Entry.Data = msnew byte[Entry.DataSize];
	memcpy(Entry.Data, pData, Entry.DataSize);

	//Data.GetData( Entry.Data );
	m_EntryList.add(Entry);

	return true;
}
bool CGroupFile::ReadEntry(const char *pszName, byte *pBuffer, unsigned long &DataSize)
{
	msstring EntryName = pszName;
	ReplaceChar(EntryName, '\\', '/');

	for (int i = 0; i < m_EntryList.size(); i++)
	{
		cachedentry_t &Entry = m_EntryList[i];
		if (Entry.FileName != EntryName)
			continue;

		/*CEncryptData1 Data;
		Data.SetData( Entry.Data, Entry.DataSize );
		if( !Data.Decrypt( ) )
			return false;
		DataSize = Data.GetDataSize();
		if( pBuffer ) memcpy( pBuffer, Data.GetData( ), DataSize );*/

		if (pBuffer)
			memcpy(pBuffer, Entry.Data, Entry.DataSize);
		DataSize = Entry.DataSize;

		return true;
	}

	return false;
}
/*DWORD CGroupFile::FindHeader( const char *pszName, groupheader_t &GroupHeader )
{
	DWORD dwFailReturn = (DWORD)-1;
	HANDLE hFile = CreateFile( m_FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if( hFile == INVALID_HANDLE_VALUE ) return dwFailReturn; 
	DWORD dwSize = GetFileSize( hFile, NULL ), dwBytesRead;
	if( dwSize < sizeof(DWORD) ) 
		{ CloseHandle( hFile ); return dwFailReturn; }
	DWORD groupEntries;
	if( !ReadFile( hFile, &groupEntries, sizeof(DWORD), &dwBytesRead, NULL ) )
		{ CloseHandle( hFile ); return dwFailReturn; }

	groupheader_t groupHeader;

	char *pszMyString = new(char[strlen(pszName)+1]);
	for( DWORD i = 0; i < groupEntries; i++ )
	{
		if( !ReadFile( hFile, &groupHeader, sizeof(groupheader_t), &dwBytesRead, NULL ) )
			{ CloseHandle( hFile );  return dwFailReturn; }
		 strncpy(pszMyString,  pszName, sizeof(pszMyString) );
		ReplaceChar( pszMyString, '/', '\\' );
		if( !stricmp(groupHeader.FileName,pszMyString) )
		{
			memcpy( &GroupHeader, &groupHeader, sizeof(groupheader_t) );
			CloseHandle( hFile );
			delete pszMyString;
			return i;
		}
	}
	CloseHandle( hFile );
	delete pszMyString;
	return (DWORD)-1;
}*/
bool CGroupFile::DeleteEntry(const char *pszName)
{
	for (int i = 0; i < m_EntryList.size(); i++)
	{
		cachedentry_t &Entry = m_EntryList[i];
		if (Entry.FileName == pszName)
		{
			if (Entry.Data)
				delete Entry.Data;
			m_EntryList.erase(i);
			return true;
		}
	}

	return false;
}

void CGroupFile::Flush()
{
	int TotalEntries = m_EntryList.size();
	int TotalSize = sizeof(int);
	for (int i = 0; i < TotalEntries; i++)
		TotalSize += sizeof(groupheader_t) + m_EntryList[i].DataSize;

	CMemFile GroupFile(TotalSize);
	GroupFile.WriteInt(TotalEntries); //[INT]

	int Offset = sizeof(int) + TotalEntries * sizeof(groupheader_t);
	for (int i = 0; i < TotalEntries; i++)
	{
		cachedentry_t &Entry = m_EntryList[i];

		groupheader_t &Header = Entry;
		Header.DataOfs = Offset;
		GroupFile.Write(&Header, sizeof(groupheader_t)); //[X groupheader_t]
		Offset += Header.DataSize;
	}

	for (int i = 0; i < TotalEntries; i++)
	{
		cachedentry_t &Entry = m_EntryList[i];
		GroupFile.Write(Entry.Data, Entry.DataSize); //[X data]
	}

	//Encrypt whole file
	CEncryptData1 Data;
	Data.SetData(GroupFile.m_Buffer, GroupFile.GetFileSize());
	Data.Encrypt();
	GroupFile.SetBuffer(Data.GetData(), Data.GetDataSize());

	GroupFile.WriteToFile(m_FileName);
}