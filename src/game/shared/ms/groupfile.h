#ifndef MSSHARED_GROUPFILE_H
#define MSSHARED_GROUPFILE_H
//Groupfile... its just like a pakfile

#include "stackstring.h"
typedef unsigned long ulong;

#pragma pack(push, 1)  // Ensure tight packing with no padding

struct pakHeader_t
{
	int MagicNumber;
	unsigned int DirectoryOffset;
	unsigned int DirectoryCount;
};

// Raw on-disk format - increased from 56 to 256 bytes (requires re-packing scripts.pak)
struct pakDirectory_t
{
	char cFilename[256];  // Increased from 56 to 256 bytes to support longer paths
	unsigned int FileOffset;
	unsigned int FileSize;
};

#pragma pack(pop)  // Restore default packing

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

	/**
	*	@brief Refresh the group file by reloading from disk
	*	@param pszFilename Optional filename, uses current file if null
	*	@return true if refresh succeeded, false otherwise
	*/
	bool Refresh(const char* pszFilename = nullptr);

	bool ReadEntry(const char* pszName, unsigned char* pBuffer, unsigned long& DataSize);

	/**
	*	@brief Get the number of entries in the PAK file
	*/
	size_t GetEntryCount() const { return m_EntryList.size(); }
	
	/**
	*	@brief Get the current filename
	*/
	const std::string& GetFilename() const { return m_Filename; }
	
	/**
	*	@brief Get a specific entry by index
	*/
	const pakDirectory_t* GetEntry(size_t index) const;
	
	/**
	*	@brief Enumerate all AngelScript files (.as) in the PAK
	*	@param outFiles Vector to store found .as file paths
	*	@return Number of .as files found
	*/
	int EnumerateAngelScriptFiles(std::vector<std::string>& outFiles) const;

private:
	CFile cFile;
	std::vector<pakDirectory_t> m_EntryList;
	std::string m_Filename; // Store filename for refresh functionality
};
extern CGameGroupFile g_ScriptPack;

#endif
#endif