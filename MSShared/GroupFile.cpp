#ifdef _WIN32
#include "windows.h"
#include "logfile.h"
#else
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <linux/sys.h>

// typedef void *HANDLE;
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

#include <fstream>

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

//Groupfile... its just like a pakfile
CGroupFile::CGroupFile(bool encrypt)
{
	m_Encrypt = encrypt;
}

void CGroupFile::Open(char *pszFileName)
{
	strncpy(m_FileName, pszFileName, sizeof(m_FileName));
	m_EntryList.clear();

	CMemFile GroupFile;
	if (GroupFile.ReadFromFile(m_FileName))
	{
		int EncryptedHeaderSize;
		GroupFile.ReadInt(EncryptedHeaderSize);
		
		CData HeaderData;
		if (m_Encrypt)
			CEncryptData1 HeaderData;
				
		HeaderData.SetData(GroupFile.m_Buffer + GroupFile.GetReadPtr(), EncryptedHeaderSize); //GroupFile.m_Buffer + GroupFile.GetFileSize(), EncryptedHeaderSize
		if (!HeaderData.Decrypt())
			return;
			
		CMemFile DecryptedHeaders;
		DecryptedHeaders.SetBuffer(HeaderData.GetData(), HeaderData.GetDataSize());

		int HeaderEntries;
		DecryptedHeaders.ReadInt(HeaderEntries);

		//Read headers
		for (int i = 0; i < HeaderEntries; i++)
		{
			cachedentry_t Entry;
			DecryptedHeaders.Read(&Entry, sizeof(groupheader_t)); //Read only the groupheader_t part.  The cachedentry_t part is not stored
			Entry.Data = NULL;

			m_EntryList.add(Entry);
		}

		//Read existing data
		for (int i = 0; i < m_EntryList.size(); i++)
		{
			CData Data;
			if (m_Encrypt)
				CEncryptData1 Data;
			
			cachedentry_t &Entry = m_EntryList[i];
				
			Data.SetData(GroupFile.m_Buffer + Entry.DataOfs, Entry.DataSizeEncrypted);
			if(!Data.Decrypt())
				continue;

			Entry.Data = msnew byte[Data.GetDataSize()];
			memcpy(Entry.Data, Data.GetData(), Data.GetDataSize());
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
	//Use an ofstream so we don't have to precalculate the file size - Solokiller
	std::ofstream file(m_FileName, std::ios_base::out | std::ios_base::binary);

	if(!file.is_open())
		return;

	int EncryptedHeaderSize = 0;

	//Open has to know how large the header chunk is in encrypted form. Write a dummy value now, update later
	//This replaces the use of the file size to decrypt the file, instead we know the size of the header and from that header get the sizes of the encrypted scripts
	file.write(reinterpret_cast<char*>(&EncryptedHeaderSize), sizeof(int));

	const size_t uiHeaderOffset = file.tellp();
	int TotalEntries = m_EntryList.size();
	const size_t uiTotalHeaderSize = sizeof(groupfileheader_t) - sizeof(groupheader_t) + (sizeof(groupheader_t) * TotalEntries);
	groupfileheader_t* pHeaders = reinterpret_cast<groupfileheader_t*>(::operator new(uiTotalHeaderSize));
	pHeaders->TotalEntries = TotalEntries;

	//Write dummy data, update after script data has been written
	{
		CData Data;
		if (m_Encrypt)
			CEncryptData1 Data;
			
		Data.SetData(reinterpret_cast<byte*>(pHeaders), uiTotalHeaderSize);
		Data.Encrypt();
		file.write(reinterpret_cast<char*>(Data.GetData()), Data.GetDataSize());
	}

	for(int i = 0; i < TotalEntries; i++)
	{
		cachedentry_t &Entry = m_EntryList[i];
		Entry.DataOfs = static_cast<int>(file.tellp());
		
		CData Data;
		if (m_Encrypt)
			CEncryptData1 Data;
			
		Data.SetData(Entry.Data, Entry.DataSize);
		Data.Encrypt();

		file.write(reinterpret_cast<char*>(Data.GetData()), Data.GetDataSize()); //[X data]

		//Update entry info
		Entry.DataSizeEncrypted = Data.GetDataSize();
	}

	for(int i = 0; i < TotalEntries; i++)
	{
		pHeaders->Headers[i] = m_EntryList[i];
	}

	//Encrypt headers
	CData Data;
	if (m_Encrypt)
		CEncryptData1 Data;
		
	Data.SetData(reinterpret_cast<byte*>(pHeaders), uiTotalHeaderSize);
	Data.Encrypt();

	file.seekp(uiHeaderOffset);
	file.write(reinterpret_cast<char*>(Data.GetData()), Data.GetDataSize());

	file.seekp(0, std::ios_base::beg);

	//Write the encrypted header size
	EncryptedHeaderSize = Data.GetDataSize();
	file.write(reinterpret_cast<char*>(&EncryptedHeaderSize), sizeof(int));

	delete pHeaders;
	file.close();
}

#ifndef NOT_HLDLL
#include "FileSystem_Shared.h"

CGameGroupFile::CGameGroupFile(bool encrypt): m_hFile(FILESYSTEM_INVALID_HANDLE)
{
	m_Encrypt = encrypt;
}

CGameGroupFile::~CGameGroupFile()
{
	Close();
}

bool CGameGroupFile::Open(const char *pszFilename)
{
	Close();

	//Load group files from config directories only (avoids loading downloaded content)
	if (!g_pFileSystem->FileExists(pszFilename))
	{
		return false;
	}
	
	m_hFile = g_pFileSystem->Open(pszFilename, "rb", "GAMECONFIG");

	if(FILESYSTEM_INVALID_HANDLE == m_hFile)
	{
		return false;
	}
	
	int EncryptedHeaderSize = 0;
	if(sizeof(int) != g_pFileSystem->Read(&EncryptedHeaderSize, sizeof(int), m_hFile))
	{
		Close();
		return false;
	}

	/*
	*	Helper to free memory automatically
	*	TODO: Should use C++11 std::unique_ptr instead
	*/
	struct CleanupMemory
	{
		byte* pMemory;

		CleanupMemory( byte* pMemory ): pMemory( pMemory )
		{
		}

		~CleanupMemory()
		{
			delete[] pMemory;
		}
	};
	
	CMemFile DecryptedHeaders;
	{
		CleanupMemory EncryptedHeaderData(msnew byte[EncryptedHeaderSize]);

		if(EncryptedHeaderSize != g_pFileSystem->Read(EncryptedHeaderData.pMemory, EncryptedHeaderSize, m_hFile) )
		{
			return false;
		}
		
		CData HeaderData;
		if (m_Encrypt)
			CEncryptData1 HeaderData;
			
		HeaderData.SetData(EncryptedHeaderData.pMemory, EncryptedHeaderSize);
		if(!HeaderData.Decrypt())
			return false;
			
		DecryptedHeaders.SetBuffer(HeaderData.GetData(), HeaderData.GetDataSize());
	}

	int HeaderEntries;
	DecryptedHeaders.ReadInt(HeaderEntries);

	//Read headers
	for(int i = 0; i < HeaderEntries; i++)
	{
		cachedentry_t Entry;
		DecryptedHeaders.Read(&Entry, sizeof(groupheader_t));

		m_EntryList.add(Entry);
	}

	//Seek to head so we're not left dangling someplace where it might cause problems
	g_pFileSystem->Seek(m_hFile, 0, FILESYSTEM_SEEK_HEAD);

	return true;
}

void CGameGroupFile::Close()
{
	if( FILESYSTEM_INVALID_HANDLE != m_hFile )
	{
		g_pFileSystem->Close( m_hFile );
		m_hFile = FILESYSTEM_INVALID_HANDLE;
	}
	
	m_EntryList.clear();
}

bool CGameGroupFile::ReadEntry( const char *pszName, byte *pBuffer, unsigned long &DataSize )
{
	msstring EntryName = pszName;
	ReplaceChar(EntryName, '\\', '/');

	for(int i = 0; i < m_EntryList.size(); i++)
	{
		groupheader_t &Entry = m_EntryList[i];
		if( Entry.FileName != EntryName )
			continue;
			
		DataSize = Entry.DataSize;
		//Decrypt on demand
		if(pBuffer)
		{
			g_pFileSystem->Seek(m_hFile, Entry.DataOfs, FILESYSTEM_SEEK_HEAD);
			byte* pEncryptedBuffer = msnew byte[Entry.DataSizeEncrypted];
			bool bSuccess = false;

			if(Entry.DataSizeEncrypted == g_pFileSystem->Read(pEncryptedBuffer, Entry.DataSizeEncrypted, m_hFile))
			{
				CData Data(pEncryptedBuffer, Entry.DataSizeEncrypted);
				if (m_Encrypt)
					CEncryptData1 Data(pEncryptedBuffer, Entry.DataSizeEncrypted);

				if(Data.Decrypt())
				{
					//TODO: could check if DataSize matches Data.GetDataSize() here - Solokiller
					memcpy(pBuffer, Data.GetData(), DataSize);

					bSuccess = true;
				}
			}

			delete[] pEncryptedBuffer;

			//Seek to head so we're not left dangling someplace where it might cause problems
			g_pFileSystem->Seek( m_hFile, 0, FILESYSTEM_SEEK_HEAD );

			return bSuccess;
		}

		return true;
	}

	return false;
}

#endif