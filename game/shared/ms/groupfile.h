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
	byte* Data;
};

class CData
{
public:
	CData()
	{
		m_pData = NULL;
	}

	CData(const byte pData[], const size_t Size)
	{
		SetData(pData, Size);
	}

	virtual ~CData()
	{
		if (m_pData)
			delete m_pData;
	}

	void SetData(const byte pData[], const size_t Size)
	{
		m_pData = new byte[Size];
		m_DataSize = Size;
		memcpy(m_pData, pData, m_DataSize);
	}

	void GetData(byte* pData) const
	{
		memcpy(pData, m_pData, m_DataSize);
	}

	byte* GetData() const
	{
		return m_pData;
	}

	size_t GetDataSize() const
	{
		return m_DataSize;
	}

protected:
	byte* m_pData;
	size_t m_DataSize;
};

// Class for file operations
class CGroupFile
{
protected:
	char m_FileName[MAX_PATH];
	//unsigned long FindHeader( const char *pszName, groupheader_t &GroupHeader );
	bool DeleteEntry(const char* pszName);
	bool m_IsOpen;

public:
	~CGroupFile() { Close(); };
	void Open(char* pszFileName);
	bool IsOpen() { return m_IsOpen; };
	void Close();
	//bool WriteEntry(const char* pszName, byte* pData, unsigned long DataSize);
	bool WriteEntry(const char* pszName, byte* pData, size_t DataSize);

	//Call Read() with pBuffer == NULL to just get the size
	//bool ReadEntry(const char* pszName, byte* pBuffer, unsigned long& DataSize);
	bool ReadEntry(const char* pszName, byte* pBuffer, size_t& DataSize);
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
#include "filesystem_shared.h"

/**
*	@brief Class to read the group file in the game
*	This class handles efficient and secure loading of the game group file.
*	It loads only the header data and keeps a handle to the file to read and decrypt scripts on demand.
*/
class CGameGroupFile
{
public:
	CGameGroupFile();
	~CGameGroupFile();

	bool IsOpen() const { return cFile.IsOpen(); }

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
	bool ReadEntry(const char* pszName, byte* pBuffer, unsigned long& DataSize);

private:
	CFile cFile;
	mslist<groupheader_t> m_EntryList;
};

#endif
#endif