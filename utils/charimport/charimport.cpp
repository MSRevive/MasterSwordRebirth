// 
// Write old FN archived char files to the new system!
// Decrypt old char files then enapsulate them with base64 and send them to the FN back end in some json formatted string.
//

#include "windows.h"
#include "stdio.h"
#include "stream_safe.h"
#include <conio.h>
#include "groupfile.h"
#include "msfileio.h"
#include "httprequesthandler.h"
#include "rapidjson/document_safe.h"
#include "base64/base64.h"

using namespace rapidjson;

#define FN_TARGET_URL "http://fn.msrebirth.net:27520/api/v1/character/"
#define MS_SIZE_LONG 4
#define MS_CHAR_SIZE 5

static size_t g_iNumFilesProcessed = 0;
static size_t g_iRootSize = 0;

class CEncryptData
{
public:
	CEncryptData() { m_pData = NULL; }
	CEncryptData(const byte* pData, const size_t Size) { SetData(pData, Size); }

	~CEncryptData()
	{
		if (m_pData)
			delete m_pData;
		m_pData = NULL;
	}

	void SetData(const byte* pData, const size_t Size)
	{
		m_pData = msnew(byte[Size]);
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

	void Encrypt()
	{
		//Calculate a checksum on the original data
		unsigned long CheckSum = 678;
		for (size_t i = 0; i < m_DataSize; i++)
		{
			bool Action = (i % 2) ? true : false;
			if (Action)
				CheckSum += (m_pData[i] * 2);
			else
				CheckSum -= m_pData[i];
		}

		//Allocate space for the new data + checksum
		byte* Encrypted = msnew byte[m_DataSize + MS_SIZE_LONG];
		memcpy(Encrypted, m_pData, m_DataSize); //Copy original data to new data
		delete[] m_pData;						//Delete old data
		m_pData = Encrypted;					//Set pointer to new data

		//Encrypt new data
		for (size_t i = 0; i < m_DataSize; i++)
		{
			byte addamt = (i * 4) % 128;				   //Offset some data
			Encrypted[i] = (Encrypted[i] + addamt) & 0xFF; //(The 0xFF is to explicitly convert the result to a byte)
		}

		for (size_t i = 0; i < m_DataSize; i++) //Swap some data around
		{
			int swap[2] = { i, (i + 7) % m_DataSize };
			byte Temp = Encrypted[swap[0]];
			Encrypted[swap[0]] = Encrypted[swap[1]];
			Encrypted[swap[1]] = Temp;
		}

		//Store checksum
		Encrypted[m_DataSize + 0] = (byte)(CheckSum / pow(256, 3));
		Encrypted[m_DataSize + 1] = (byte)(CheckSum / pow(256, 2));
		Encrypted[m_DataSize + 2] = (byte)(CheckSum / pow(256, 1));
		Encrypted[m_DataSize + 3] = (byte)((int)CheckSum % (int)pow(256, 1));
		m_DataSize += MS_SIZE_LONG; //Set the new size of the data
	}

	bool Decrypt()
	{
		if (m_DataSize < MS_SIZE_LONG)
			return false; //File must be at least big enough for the checksum

		unsigned long FileCheckSum = 0;
		unsigned long CheckSumPos = m_DataSize - MS_SIZE_LONG;

		//Retrieve checksum
		FileCheckSum += m_pData[CheckSumPos + 3];
		FileCheckSum += (unsigned long)(m_pData[CheckSumPos + 2] * pow(256, 1));
		FileCheckSum += (unsigned long)(m_pData[CheckSumPos + 1] * pow(256, 2));
		FileCheckSum += (unsigned long)(m_pData[CheckSumPos + 0] * pow(256, 3));
		m_DataSize -= MS_SIZE_LONG; //Set the size of the data buffer

		//Allocate a buffer for the original data
		byte* Decrypted = msnew byte[m_DataSize];
		memcpy(Decrypted, m_pData, m_DataSize); //Copy encrypted data to new buffer
		delete[] m_pData;						//Delete old data
		m_pData = Decrypted;					//Set pointer to new buffer

		//Decrypt new data
		for (int i = m_DataSize - 1; i >= 0; i--) //Unswap the data
		{
			int swap[2] = { i, (i + 7) % m_DataSize };
			byte Temp = Decrypted[swap[0]];
			Decrypted[swap[0]] = Decrypted[swap[1]];
			Decrypted[swap[1]] = Temp;
		}

		for (int i = m_DataSize - 1; i >= 0; i--)
			Decrypted[i] -= (i * 4) % 128; //Offset the data

		//Check the Checksum
		unsigned long CheckSum = 678;
		for (size_t i = 0; i < m_DataSize; i++)
		{
			bool Action = (i % 2) ? true : false;
			if (Action)
				CheckSum += (Decrypted[i] * 2);
			else
				CheckSum -= Decrypted[i];
		}

		return (FileCheckSum == CheckSum);
	}

protected:
	byte* m_pData;
	size_t m_DataSize;
};

// See https://developer.valvesoftware.com/wiki/SteamID
unsigned long long GetSteamID64(const char* id)
{
	if (!id || !id[0]) return 0ULL;

	char pchSteamID[32];
	strncpy(pchSteamID, id, sizeof(pchSteamID));

	const char* pStart = pchSteamID;
	size_t size = strlen(pchSteamID);

	for (size_t i = 0; i < size; i++)
	{
		if (pchSteamID[i] == '-') // Skip the first part.
		{
			pStart += (i + 1);
			strncpy(pchSteamID, pStart, sizeof(pchSteamID)); // Removes STEAM_X:, WE ONLY CARE ABOUT Y:Z part!
			break;
		}
	}

	char pchArg1[16], pchArg2[16];
	pStart = pchSteamID;
	size = strlen(pchSteamID);

	for (size_t i = 0; i < size; i++)
	{
		if (pchSteamID[i] == '-') // split
		{
			strncpy(pchArg1, pStart, sizeof(pchArg1));
			pchArg1[i] = '\0';

			pStart += (i + 1);
			strncpy(pchArg2, pStart, sizeof(pchArg2));
			break;
		}
	}

	return (atoi(pchArg1) + 2ULL * atoi(pchArg2) + 76561197960265728ULL);
}

static bool WriteCharToFN(const char* steamID, int slot, const char* data, int size)
{
	StringBuffer s;
	Writer<StringBuffer> writer(s);

	writer.StartObject();

	writer.Key("steamid");
	writer.String(steamID);

	writer.Key("slot");
	writer.Int(slot);

	writer.Key("size");
	writer.Int(size);

	writer.Key("data");
	writer.String(base64_encode((const unsigned char*)data, size).c_str());

	writer.EndObject();

	return HTTPRequestHandler::PostRequest(FN_TARGET_URL, s.GetString());
}

static void ImportOldCharacter(const char* file)
{
	if (!(file && file[0])) return; // Empty file str?

	// Parse SteamID + slot from file name
	char pchTempStr[MAX_PATH], pchSteamID[MAX_PATH], pchSlot[32];
	pchSteamID[0] = 0; pchSlot[0] = 0;
	strncpy(pchTempStr, file + g_iRootSize, MAX_PATH);

	for (int i = (strlen(pchTempStr) - 1); i >= 0; --i)
	{
		if (pchTempStr[i] == '_')
		{
			strncpy(pchSteamID, (file + g_iRootSize), MAX_PATH);
			strncpy(pchSlot, (file + g_iRootSize + i + 1), sizeof(pchSlot));
			pchSteamID[i] = 0;
			pchSlot[strlen(pchSlot) - MS_CHAR_SIZE] = 0;
			break;
		}
	}

	const unsigned long long steamID = GetSteamID64(pchSteamID);
	const int slot = (atoi(pchSlot) - 1);
	_snprintf(pchSteamID, MAX_PATH, "%llu", steamID);

	if ((slot < 0) || (slot >= 3))
	{
		printf("File '%s' has an invalid slot: %i (%s)\n", pchTempStr, slot, pchSlot);
		return;
	}

	// Read character file
	CPlayer_DataBuffer gFile;
	if (!gFile.ReadFromFile(file, "rb", false))
	{
		printf("Unable to read file: %s\n", pchTempStr);
		return;
	}

	// Decrypt character file
	CEncryptData* pEncrpytion = new CEncryptData;
	pEncrpytion->SetData(gFile.m_Buffer, gFile.m_BufferSize);

	if (!pEncrpytion->Decrypt())
	{
		delete pEncrpytion;
		printf("Unable to decrypt file: %s\n", pchTempStr);
		return;
	}

	// Assign decrypted data to gFile
	gFile.Alloc(pEncrpytion->GetDataSize());
	pEncrpytion->GetData(gFile.m_Buffer);
	delete pEncrpytion;

	// Write decrypted data for plr X to FN!
	if (!WriteCharToFN(pchSteamID, slot, (char*)gFile.m_Buffer, gFile.m_BufferSize))
	{
		printf("Unable to write contents of decryped file '%s' (size: %i) to FN!\n", pchTempStr, gFile.m_BufferSize);
		return;
	}

	gFile.Close();
	g_iNumFilesProcessed++;
	printf("Wrote file '%s' to FN!\n", pchTempStr);
}

int main(int argc, char** argv)
{
	char root[MAX_PATH], searchPath[MAX_PATH], filePath[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, root);
	g_iRootSize = (strlen(root) + 1);
	_snprintf(searchPath, MAX_PATH, "%s\\*.char", root);
	printf("Scanning folder: %s\n", searchPath);

	WIN32_FIND_DATA wfd;
	HANDLE findHandle = FindFirstFile(searchPath, &wfd);
	if (findHandle == INVALID_HANDLE_VALUE)
	{
		printf("Cannot find any .char files!\n");
		return 0;
	}

	do
	{
		if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			_snprintf(filePath, MAX_PATH, "%s\\%s", root, wfd.cFileName);
			ImportOldCharacter(filePath);
		}
		fflush(stdout);
	} while (FindNextFile(findHandle, &wfd));

	FindClose(findHandle);
	printf("\nCompleted importing %i characters\n", g_iNumFilesProcessed);
	printf("Press any key to exit...\n");

	while (1)
	{
		if (kbhit())
			break;
	}

	return 0;
}