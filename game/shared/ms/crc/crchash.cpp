#include "checksum_crc.h"
#include "msfileio.h"

static unsigned int ComputeCRC32ForFile(const char *path)
{
	if (!path || !path[0])
		return 0;

	unsigned int hashValue = 0;
	CMemFile gameFile;
	if (gameFile.ReadFromFile(path))
	{
		hashValue = CRC32::CRC32_ProcessSingleBuffer(gameFile.m_Buffer, gameFile.m_BufferSize);
		gameFile.Close();
	}

	return hashValue;
}

bool MatchFileCheckSum(const char* FilePath, unsigned int CheckSum)
{
	return (ComputeCRC32ForFile(FilePath) == CheckSum);
}

unsigned int GetFileCheckSum(const char* FilePath)
{
	return ComputeCRC32ForFile(FilePath);
}