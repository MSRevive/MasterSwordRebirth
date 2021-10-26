#include <iostream>
#include <cmath>
#include "encrypt.h"
#include "msdebug.h"

using namespace std;
#define MS_SIZE_LONG 4

void CEncryptBase::SetData(const byte *pData, const size_t Size)
{
	m_pData = msnew(byte[Size]);
	m_DataSize = Size;
	memcpy(m_pData, pData, m_DataSize);
}

void CEncryptBase::GetData(byte *pData) const
{
	memcpy(pData, m_pData, m_DataSize);
}
byte *CEncryptBase::GetData(void) const
{
	return m_pData;
}

// Encryption algorithim 1
// This algorithm should work in both Windows and Linux
// rand() cannot be used, because even when seeded the same,
// rand() generates different numbers for Linux and Windows.

void CEncryptData1::Encrypt()
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
	byte *Encrypted = msnew byte[m_DataSize + MS_SIZE_LONG];
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
		int swap[2] = {i, (i + 7) % m_DataSize};
		byte Temp = Encrypted[swap[0]];
		Encrypted[swap[0]] = Encrypted[swap[1]];
		Encrypted[swap[1]] = Temp;
	}

	//Store checksum
	Encrypted[m_DataSize + 0] = (byte)(CheckSum / static_cast<int>(pow(256.0f, 3)));
	Encrypted[m_DataSize + 1] = (byte)(CheckSum / static_cast<int>(pow(256.0f, 2)));
	Encrypted[m_DataSize + 2] = (byte)(CheckSum / static_cast<int>(pow(256.0f, 1)));
	Encrypted[m_DataSize + 3] = (byte)((int)CheckSum % (int)pow(256.0f, 1));
	m_DataSize += MS_SIZE_LONG; //Set the new size of the data
}

//Decryption algorithm 1
bool CEncryptData1::Decrypt()
{
	if (m_DataSize < MS_SIZE_LONG)
		return false; //File must be at least big enough for the checksum
	unsigned long FileCheckSum = 0;
	unsigned long CheckSumPos = m_DataSize - MS_SIZE_LONG;

	//Retrieve checksum
	FileCheckSum += m_pData[CheckSumPos + 3];
	FileCheckSum += (unsigned long)(m_pData[CheckSumPos + 2] * static_cast<int>(pow(256.0f, 1)));
	FileCheckSum += (unsigned long)(m_pData[CheckSumPos + 1] * static_cast<int>(pow(256.0f, 2)));
	FileCheckSum += (unsigned long)(m_pData[CheckSumPos + 0] * static_cast<int>(pow(256.0f, 3)));
	m_DataSize -= MS_SIZE_LONG; //Set the size of the data buffer

	//Allocate a buffer for the original data
	byte *Decrypted = msnew byte[m_DataSize];
	memcpy(Decrypted, m_pData, m_DataSize); //Copy encrypted data to new buffer
	delete[] m_pData;						//Delete old data
	m_pData = Decrypted;					//Set pointer to new buffer

	//Decrypt new data
	for (int i = m_DataSize - 1; i >= 0; i--) //Unswap the data
	{
		int swap[2] = {i, (i + 7) % m_DataSize};
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

// Encryption algorithim 2
void CEncryptData2::Encrypt()
{
	srand(m_DataSize / 3);

	//Do Checksum stuff
	unsigned long CheckSum = rand() % 6666;
	for (size_t i = 0; i < m_DataSize; i++)
	{
		byte Action = rand() % 2;
		if (Action)
			CheckSum += m_pData[i];
		else
			CheckSum -= m_pData[i];
	}

	byte *pNewData = new (byte[m_DataSize + sizeof(CheckSum)]);
	memcpy(pNewData, m_pData, m_DataSize);
	memcpy(&pNewData[m_DataSize], &CheckSum, sizeof(CheckSum));
	delete[] m_pData;
	m_pData = pNewData;
	m_DataSize += sizeof(CheckSum);

	//2:
	unsigned long usableDataSize = m_DataSize - sizeof(CheckSum);
	srand(usableDataSize / 6);
	byte tmp;
	size_t num, loops = rand() % 128;
	if (loops < 40)
		loops = 40;
	for (int i = 0; i < loops; i++)
	{
		num = rand() % usableDataSize;
		tmp = m_pData[usableDataSize - num];
		m_pData[usableDataSize - num] = m_pData[num];
		m_pData[num] = tmp + (num + 128) % 256;
	}

	//3:
	srand(usableDataSize);
	for (int i = 0; i < usableDataSize; i++)
		m_pData[i] += rand();
}

//#include "logfile.h"

bool CEncryptData2::Decrypt()
{
	long usableDataSize = m_DataSize - MS_SIZE_LONG;

	if (usableDataSize < 4)
		return false;

	//logfile << "usableDataSize == " << usableDataSize << endl;

	//3:
	srand(usableDataSize);
	int i;
	for (i = 0; i < usableDataSize; i++)
		m_pData[i] -= rand();

	/*logfile << "long size: " << sizeof(unsigned long) << endl;

	hex( logfile );
	logfile << "Stage 3:" << endl;
	Foreach( i, usableDataSize )
		logfile <<  "[" << (unsigned char)m_pData[i] << "]";
	logfile << "\r\n" << "Stage 3 hex:" << endl;
	Foreach( i, usableDataSize )
		logfile <<  "[" << (unsigned int)m_pData[i] << "]";
	logfile << "" << endl;*/

	//2:
	srand(usableDataSize / 6);
	int loops = rand() % 128;
	if (loops < 40)
		loops = 40;
	int randNum[128], num, tmp;
	for (i = 0; i < loops; i++)
		randNum[i] = rand() % usableDataSize;
	for (i = loops - 1; i >= 0; i--)
	{
		num = randNum[i];
		tmp = m_pData[num];
		m_pData[num] = m_pData[usableDataSize - num];
		m_pData[usableDataSize - num] = tmp - (num + 128) % 256;
	}

	/*logfile << "Stage 2:" << endl;
	Foreach( i, usableDataSize )
		logfile <<  "[" << (unsigned char)m_pData[i] << "]";
	logfile << "\r\n" << "Stage 2 hex:" << endl;
	Foreach( i, usableDataSize )
		logfile <<  "[" << (unsigned int)m_pData[i] << "]";
	logfile << "" << endl;*/

	//Check the Checksum
	srand(usableDataSize / 3);

	unsigned long CheckSum = rand() % 6666;
	for (i = 0; i < (signed)(m_DataSize - MS_SIZE_LONG); i++)
	{
		byte Action = rand() % 2;
		if (Action)
			CheckSum += m_pData[i];
		else
			CheckSum -= m_pData[i];
	}

	unsigned long realCheckSum;
	memcpy(&realCheckSum, &m_pData[m_DataSize - MS_SIZE_LONG], MS_SIZE_LONG);

	//logfile << "CheckSum == " << realCheckSum << "\r\n";

	m_DataSize -= MS_SIZE_LONG;
	byte *pNewData = new (byte[m_DataSize]);
	memcpy(pNewData, m_pData, m_DataSize);
	delete[] m_pData;
	m_pData = pNewData;
	return (CheckSum == realCheckSum);
}
