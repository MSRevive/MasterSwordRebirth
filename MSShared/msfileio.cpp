#include "msfileio.h"
#include "encrypt.h"

#ifndef NOT_HLDLL
#include "extdll.h"
#include "util.h"
#else
#include "../MSShared/sharedutil.h"
#define msnew new
#define STRING(a) ""
#define LOAD_FILE_FOR_ME(a, b) 0

#endif

using namespace std;

//
//	Implementation of CGameFile and CPlayer_DataBuffer
//  ���������������������������������������������
bool CGameFile::OpenWrite(const char *pszFileName)
{
	m_File.open(pszFileName, ios_base::out | ios_base::binary);
	if (!m_File.is_open())
		return false;

	Reset();
	return true;
}
bool CGameFile::OpenRead(const char *pszFileName)
{
	m_File.open(pszFileName, ios_base::in | ios_base::binary);
	if (!m_File.is_open())
		return false;

	Reset();
	return true;
}
void CGameFile::Close()
{
	m_File.flush();
	m_File.close();
}
void CGameFile::Write(void *pvData, size_t Size)
{
	m_File.write((const char *)pvData, Size);
}
bool CGameFile::Read(void *pvData, size_t Size)
{
	if (Size)
		((char *)pvData)[0] = 0;
	m_File.read((char *)pvData, Size);
	//size_t retSize = fread( pvData, Size, 1, m_pFile );
	//return (retSize > 0) ? true : false;
	return !m_File.bad();
}
void CGameFile::ReadString(char *Data)
{
	byte ReadIn;
	ReadIn = 0;
	Data[0] = 0;
	int i = 0;
	do
	{
		if (!Read(&ReadIn, 1))
			break;
		Data[i++] = ReadIn;
	} while (ReadIn);
}

size_t CGameFile::GetFileSize()
{
	m_File.seekg(0, ios::cur);
	int CurPos = m_File.tellg();
	m_File.seekg(0, ios::end);
	int Size = m_File.tellg();
	m_File.seekg(CurPos, ios::beg);
	return Size;
}

CMemFile::CMemFile(int Alloc)
{
	init();
	Open(Alloc);
}
void CMemFile::Open(int iAlloc)
{
	m_ReadOffset = m_WriteOffset = 0;
	Alloc(iAlloc);
}
void CMemFile::Alloc(int Alloc)
{
	Dealloc();
	m_Buffer = msnew byte[Alloc];
	m_BufferSize = Alloc;
}
void CMemFile::Dealloc()
{
	if (m_Buffer)
		delete m_Buffer;
	m_Buffer = NULL;
	m_BufferSize = 0;
}
void CMemFile::SetBuffer(byte *pNewBuffer, size_t Size)
{
	Dealloc();
	Alloc(Size);
	memcpy(m_Buffer, pNewBuffer, Size);
}
void CMemFile::Close()
{
	Dealloc();
	m_ReadOffset = m_WriteOffset = 0;
}
void CMemFile::Reset()
{
	m_ReadOffset = m_WriteOffset = 0;
}
void CMemFile::Write(void *pvData, size_t Size)
{
	memcpy(&m_Buffer[m_WriteOffset], pvData, Size);
	m_WriteOffset += Size;
	//m_BufferSize += Size;
}
bool CMemFile::Read(void *pvData, size_t Size)
{
	if (m_ReadOffset + Size > m_BufferSize)
		return false;
	memcpy(pvData, &m_Buffer[m_ReadOffset], Size);
	m_ReadOffset += Size;
	return true;
}
void CMemFile::WriteToFile(const char *pszFileName)
{
	CGameFile::OpenWrite(pszFileName);
	CGameFile::Write(m_Buffer, m_BufferSize);
	CGameFile::Close();
}
bool CMemFile::ReadFromGameFile(const char *pszFileName)
{
	//Load a half-life engine file - could be compressed in a package

	int Size = 0;
	byte *pBuffer = LOAD_FILE_FOR_ME((char *)pszFileName, &Size);

	if (!pBuffer)
		return false;

	Dealloc();
	m_BufferSize = Size;
	m_Buffer = msnew byte[m_BufferSize];
	memcpy(m_Buffer, pBuffer, m_BufferSize);
	return true;
}
bool CMemFile::ReadFromFile(const char *pszFileName)
{
	bool Success = CGameFile::OpenRead(pszFileName);
	if (!Success)
		return false;

	Dealloc();
	m_BufferSize = CGameFile::GetFileSize();
	m_Buffer = msnew byte[m_BufferSize];
	if (!CGameFile::Read(m_Buffer, m_BufferSize))
		return false;
	CGameFile::Close();
	return true;
}
size_t CMemFile::GetFileSize()
{
	return m_BufferSize;
}
void CPlayer_DataBuffer::WriteToFile(const char *pszFileName, const char *OpenFlags, bool WriteBackup)
{
	//Print( "WriteToFile Size: %i\n", m_BufferSize );
	CMemFile::WriteToFile(pszFileName);
	if (WriteBackup) //Write backup
		CMemFile::WriteToFile(BACKUP_NAME(pszFileName));
}
bool CPlayer_DataBuffer::ReadFromFile(const char *pszFileName, const char *OpenFlags, bool ReadBackup)
{
	//Print( "ReadFromFile Size: %i\n", CGameFile::GetFileSize( ) );
	bool Success = CMemFile::ReadFromFile(pszFileName);
	if ((!Success || !CMemFile::GetFileSize()) && ReadBackup) //File doesn't exist or is zero length, try reading backup file
		Success = CMemFile::ReadFromFile(BACKUP_NAME(pszFileName));
	return Success;
}
void CPlayer_DataBuffer::Encrypt(int Encrypt)
{
	CEncryptBase *pEncrpytion;
	if (Encrypt == 0)
		pEncrpytion = msnew(CEncryptData1);
	else if (Encrypt == 1)
		pEncrpytion = msnew(CEncryptData2);
	else
		return;
	pEncrpytion->SetData(m_Buffer, m_BufferSize);
	pEncrpytion->Encrypt();
	Alloc(pEncrpytion->GetDataSize());
	pEncrpytion->GetData(m_Buffer);
	//m_BufferSize = pEncrpytion->GetDataSize( );
	delete pEncrpytion;
}
bool CPlayer_DataBuffer::Decrypt(int Encrypt)
{
	CEncryptBase *pEncrpytion;
	if (Encrypt == 0)
		pEncrpytion = msnew(CEncryptData1);
	else if (Encrypt == 1)
		pEncrpytion = msnew(CEncryptData2);
	else
		return false;
	pEncrpytion->SetData(m_Buffer, m_BufferSize);
	if (!pEncrpytion->Decrypt())
	{
		delete pEncrpytion;
		return false;
	}
	Alloc(pEncrpytion->GetDataSize());
	pEncrpytion->GetData(m_Buffer);
	//m_BufferSize = pEncrpytion->GetDataSize( );
	delete pEncrpytion;
	return true;
}
