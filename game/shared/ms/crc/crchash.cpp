#include "checksum_crc.h"
#include "msfileio.h"

static unsigned long ComputeCRC32ForFile(const char *path)
{
	if (!path || !path[0])
		return 0UL;

	unsigned long hashValue = 0UL;
	CMemFile gameFile;
	bool bCouldLoad = gameFile.ReadFromFile(path);
	if (bCouldLoad)
	{
		hashValue = CRC32::CRC32_ProcessSingleBuffer(gameFile.m_Buffer, gameFile.m_BufferSize);
		gameFile.Close();
	}

	return hashValue;
}

bool MatchFileCheckSum(const char* FilePath, unsigned long CheckSum)
{
	return (ComputeCRC32ForFile(FilePath) == CheckSum);
}

unsigned long GetFileCheckSum(const char* FilePath)
{
	return ComputeCRC32ForFile(FilePath);
}