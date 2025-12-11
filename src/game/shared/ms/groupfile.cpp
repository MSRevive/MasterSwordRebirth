#include "Platform.h"
#ifdef _WIN32
#include "PlatformWin.h"
#else
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include "extdll.h"
#endif

#include "groupfile.h"
#include "msfileio.h"
#include <memory>

#ifdef _MSR_UTILS
IFileSystem* g_pFileSystem;
#endif // _MSR_UTILS

//Deuplicated from msdebug.h
#ifdef NOT_HLDLL
#define msnew new
#elif DEV_BUILD
void* operator new(size_t size, const char* pszSourceFile, int LineNum);
void operator delete(void* ptr, const char* pszSourceFile, int LineNum);
#define msnew new (__FILE__, __LINE__)
#else
#define msnew new
#endif

/*-----------
	CGameGroupFile
------------*/
#ifndef NOT_HLDLL
#include "filesystem_shared.h"

CGameGroupFile::CGameGroupFile() : cFile()
{
}

CGameGroupFile::~CGameGroupFile()
{
	cFile.Close();
}

bool CGameGroupFile::Open(const char* pszFilename)
{
	// Validate input parameter
	if (!pszFilename || strlen(pszFilename) == 0)
	{
		printf("CGameGroupFile::Open: ERROR - Invalid filename (null or empty)\n");
		return false;
	}

	printf("CGameGroupFile::Open: Attempting to open PAK file: %s\n", pszFilename);

	//Load group files from config directories only (avoids loading downloaded content)
	if (!cFile.FileExists(pszFilename))
	{
		printf("CGameGroupFile::Open: ERROR - File does not exist: %s\n", pszFilename);
		return false;
	}

	printf("CGameGroupFile::Open: File exists, attempting to open for reading...\n");
	if (!cFile.Open(pszFilename, "rb", "GAMECONFIG"))
	{
		printf("CGameGroupFile::Open: ERROR - Failed to open file for reading: %s\n", pszFilename);
		printf("CGameGroupFile::Open: This could indicate permission issues or file corruption\n");
		return false;
	}

	// Store filename for refresh functionality
	m_Filename = pszFilename;
	printf("CGameGroupFile::Open: File opened successfully, reading PAK header...\n");

	pakHeader_t Header;
	memset(&Header, 0, sizeof(Header)); // Initialize header to prevent garbage data

	size_t bytesRead = cFile.Read(&Header, sizeof(Header));
	if (bytesRead != sizeof(Header))
	{
		printf("CGameGroupFile::Open: ERROR - Failed to read PAK header (read %zu bytes, expected %zu)\n", 
			   bytesRead, sizeof(Header));
		cFile.Close();
		return false;
	}

	printf("CGameGroupFile::Open: Header read successfully, validating magic number...\n");
	printf("CGameGroupFile::Open: Magic number: %u (expected: 1262698832)\n", Header.MagicNumber);
	printf("CGameGroupFile::Open: Directory offset: %u, Directory count: %u\n", 
		   Header.DirectoryOffset, Header.DirectoryCount);

	if (Header.MagicNumber != 1262698832)
	{
		printf("CGameGroupFile::Open: ERROR - Invalid PAK magic number: %u\n", Header.MagicNumber);
		printf("CGameGroupFile::Open: File may be corrupted or not a valid PAK file\n");
		cFile.Close();
		return false;
	}

	// Validate directory information
	if (Header.DirectoryCount == 0)
	{
		printf("CGameGroupFile::Open: WARNING - PAK file has no directory entries\n");
	}
	else if (Header.DirectoryCount > 10000) // Sanity check
	{
		printf("CGameGroupFile::Open: ERROR - Directory count seems invalid: %u\n", Header.DirectoryCount);
		cFile.Close();
		return false;
	}

	printf("CGameGroupFile::Open: Seeking to directory at offset %u...\n", Header.DirectoryOffset);
	cFile.Seek(Header.DirectoryOffset, FILESYSTEM_SEEK_HEAD);

	//Read entries
	printf("CGameGroupFile::Open: Reading %u directory entries...\n", Header.DirectoryCount);
	for (int i = 0; i < Header.DirectoryCount; i++)
	{
		pakDirectory_t Entry;
		memset(&Entry, 0, sizeof(Entry)); // Initialize entry to prevent garbage data
		
		size_t entryBytesRead = cFile.Read(&Entry, sizeof(pakDirectory_t));
		if (entryBytesRead != sizeof(pakDirectory_t))
		{
			printf("CGameGroupFile::Open: ERROR - Failed to read directory entry %d\n", i);
			cFile.Close();
			m_EntryList.clear();
			return false;
		}

		// Ensure filename is null-terminated (critical for string safety)
		Entry.cFilename[sizeof(Entry.cFilename) - 1] = '\0';

		m_EntryList.push_back(Entry);
	}

	//Seek to head so we're not left dangling someplace where it might cause problems
	cFile.Seek(0, FILESYSTEM_SEEK_HEAD);

	printf("CGameGroupFile::Open: Successfully loaded PAK file with %u entries\n", Header.DirectoryCount);
	return true;
}

void CGameGroupFile::Close()
{
	cFile.Close();
	m_EntryList.clear();
	m_Filename.clear();
}

bool CGameGroupFile::Refresh(const char* pszFilename)
{
	// Use provided filename or current filename
	const char* filename = pszFilename ? pszFilename : m_Filename.c_str();
	
	printf("CGameGroupFile::Refresh: Starting PAK file refresh...\n");
	
	if (!filename || strlen(filename) == 0)
	{
		printf("CGameGroupFile::Refresh: ERROR - No filename available (provided: %s, stored: %s)\n",
			   pszFilename ? pszFilename : "null", 
			   m_Filename.empty() ? "empty" : m_Filename.c_str());
		return false;
	}
	
	printf("CGameGroupFile::Refresh: Using filename: %s\n", filename);
	printf("CGameGroupFile::Refresh: Current state - %s, %zu entries loaded\n",
		   IsOpen() ? "OPEN" : "CLOSED", m_EntryList.size());
	
	// Store old entry count for comparison
	size_t oldEntryCount = m_EntryList.size();
	
	// Close current file and clear data
	printf("CGameGroupFile::Refresh: Closing current PAK file...\n");
	Close();
	
	// Reopen with the same filename
	printf("CGameGroupFile::Refresh: Reopening PAK file...\n");
	bool result = Open(filename);
	
	if (result)
	{
		printf("CGameGroupFile::Refresh: SUCCESS - PAK file refreshed successfully\n");
		printf("CGameGroupFile::Refresh: Entry count changed from %zu to %zu\n", 
			   oldEntryCount, m_EntryList.size());
	}
	else
	{
		printf("CGameGroupFile::Refresh: FAILED - Could not reopen PAK file\n");
	}
	
	return result;
}

bool CGameGroupFile::ReadEntry(const char* pszName, byte* pBuffer, unsigned long& DataSize)
{
	msstring EntryName = pszName;
	ReplaceChar(EntryName, '\\', '/');

	for (int i = 0; i < m_EntryList.size(); i++)
	{
		pakDirectory_t Entry = m_EntryList[i];
		
		// Try case-sensitive match first (faster)
		if (strcmp(Entry.cFilename, EntryName) == 0)
		{
			// Exact match found, continue to read
		}
		// Try case-insensitive match as fallback (for Windows compatibility)
		else
		{
#ifdef _WIN32
			if (_stricmp(Entry.cFilename, EntryName) != 0)
				continue;
#else
			if (strcasecmp(Entry.cFilename, EntryName) != 0)
				continue;
#endif
		}

		DataSize = Entry.FileSize;
		if (pBuffer)
		{
			cFile.Seek(Entry.FileOffset, FILESYSTEM_SEEK_HEAD);
			byte* _pBuffer = msnew byte[Entry.FileSize];
			bool bSuccess = false;

			unsigned long BytesRead = cFile.Read(_pBuffer, Entry.FileSize);
			if (BytesRead == Entry.FileSize)
			{
				memcpy(pBuffer, _pBuffer, BytesRead);
				DataSize = BytesRead;
				bSuccess = true;
			}

			delete[] _pBuffer;

			//Seek to head so we're not left dangling someplace where it might cause problems
			cFile.Seek(0, FILESYSTEM_SEEK_HEAD);

			return bSuccess;
		}

		return true;
	}

	return false;
}

const pakDirectory_t* CGameGroupFile::GetEntry(size_t index) const
{
	if (index >= m_EntryList.size())
		return nullptr;
	
	return &m_EntryList[index];
}

int CGameGroupFile::EnumerateAngelScriptFiles(std::vector<std::string>& outFiles) const
{
	outFiles.clear();
	
	for (size_t i = 0; i < m_EntryList.size(); i++)
	{
		const pakDirectory_t& entry = m_EntryList[i];
		std::string filename = entry.cFilename;
		
		// Check if this is an AngelScript file (.as extension)
		if (filename.length() > 3 && 
			filename.substr(filename.length() - 3) == ".as")
		{
			outFiles.push_back(filename);
		}
	}
	
	return static_cast<int>(outFiles.size());
}

#endif