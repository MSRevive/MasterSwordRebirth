#include <string.h> // Needed for strlen().
#include <stdio.h>
#include "../stream_safe.h"

#include "xxhash/xxhash32.h"
#include "stackstring.h"

using namespace std;

bool MatchFileCheckSum(const char* FilePath, uint32_t CheckSum)
{
	ifstream file(FilePath, ios_base::in);
	if (!file.is_open())
		return false;

	file.seekg(0, ios::end);
	long FileSize = file.tellg();
	file.seekg(0, ios::beg);

	char* pFileData = new char[FileSize];
	file.read(pFileData, FileSize);
	file.close();

	uint32_t CRCText = XXHash32::hash(pFileData, FileSize, 0);

	delete[] pFileData;
	if (CheckSum == CRCText)
		return true;
	else
		return false;
}

uint32_t GetFileCheckSum(const char* FilePath)
{
	ifstream file(FilePath, ios_base::in);
	if (!file.is_open())
		return false;

	file.seekg(0, ios::end);
	long FileSize = file.tellg();
	file.seekg(0, ios::beg);

	char* pFileData = new char[FileSize];
	file.read(pFileData, FileSize);
	file.close();

	uint32_t CRCText = XXHash32::hash(pFileData, FileSize, 0);

	delete[] pFileData;
	return CRCText;
}

uint32_t GetFileCheckSumSize(const char* FilePath)
{
	ifstream file(FilePath, ios_base::in);
	if (!file.is_open())
		return false;

	file.seekg(0, ios::end);
	long FileSize = file.tellg();
	file.seekg(0, ios::beg);

	char* pFileData = new char[FileSize];
	file.read(pFileData, FileSize);
	file.close();

	uint32_t CRCText = XXHash32::hash(pFileData, FileSize, 0);

	delete[] pFileData;
	return CRCText + FileSize;
}