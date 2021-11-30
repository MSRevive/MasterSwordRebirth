#ifndef MSSHARED_GROUPFILE_H
#define MSSHARED_GROUPFILE_H
//Groupfile... its just like a pakfile

#include "stackstring.h"
typedef unsigned long ulong;

struct groupheader_t
{
	msstring FileName;
	ulong DataOfs, DataSize, DataSizeEncrypted;
};

/**
*	@brief Header for Group files
*	This is a variable length header where Headers is TotalEntries in size
*/
struct groupfileheader_t
{
	int TotalEntries;
	groupheader_t Headers[1]; //[ TotalEntries ]
};

struct cachedentry_t : groupheader_t
{
	byte *Data;
};

// Class for file operations with encryption
class CGroupFile
{
protected:
	char m_FileName[MAX_PATH];
	//unsigned long FindHeader( const char *pszName, groupheader_t &GroupHeader );
	bool DeleteEntry(const char *pszName);
	bool m_IsOpen;
	bool m_Encrypt;

public:
	CGroupFile(bool encrypt = true);
	~CGroupFile() { Close(); };
	void Open(char *pszFileName);
	bool IsOpen() { return m_IsOpen; };
	void Close();
	bool WriteEntry(const char *pszName, byte *pData, unsigned long DataSize);

	//Call Read() with pBuffer == NULL to just get the size
	bool ReadEntry(const char *pszName, byte *pBuffer, unsigned long &DataSize);
	void Flush();

private:
	mslist<cachedentry_t> m_EntryList;
};

/*Format:
  [DWORD] Number of Headers
  [groupheader_t * X] X Amount of Headers
  [DATA] All data
*/

#ifndef NOT_HLDLL
#include "FileSystem.h"

/**
*	@brief Class to read the group file in the game
*	This class handles efficient and secure loading of the game group file.
*	It loads only the header data and keeps a handle to the file to read and decrypt scripts on demand.
*/
class CGameGroupFile
{
protected:
	bool m_Encrypt;
public:
	CGameGroupFile(bool encrypt = false);
	~CGameGroupFile();

	bool IsOpen() const { return FILESYSTEM_INVALID_HANDLE != m_hFile; }

	/**
	*	@brief Loads the group file from a given file
	*	If the group file was previously loaded, all data is first purged
	*/
	bool Open(const char* pszFilename);

	/**
	*	@brief Manually close the file and purge all header data
	*/
	void Close();

	//Call Read() with pBuffer == NULL to just get the size
	//Decrypts script data on demand, avoid calling more than once for any given script
	bool ReadEntry(const char *pszName, byte *pBuffer, unsigned long &DataSize);

private:
	FileHandle_t m_hFile;
	mslist<groupheader_t> m_EntryList;
};

#endif
#endif