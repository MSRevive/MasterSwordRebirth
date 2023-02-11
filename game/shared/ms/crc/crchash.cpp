#include "checksum_crc.h"
#include "msfileio.h"

static unsigned int ComputeCRC32ForFile(const char* path)
{
	if (!path || !path[0])
		return 0;

	CMemFile gameFile;
	unsigned int hashValue = (gameFile.ReadFromFile(path) ? CRC32::CRC32_ProcessSingleBuffer(gameFile.m_Buffer, gameFile.m_BufferSize) : 0);
	gameFile.Close();

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