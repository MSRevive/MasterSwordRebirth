#define WIN32_LEAN_AND_MEAN

#ifdef _WIN32
#include "windows.h"
#else
#include <unistd.h> // usleep
#define Sleep sleep
#endif

#include "memory"
#include "sharedutil.h"
#include "logfile.h"

#define ENCRYPT_TITLE "Master Sword Memory Encryption"

safemem_varbase::safemem_varbase()
{
	m_Changed = false;
	m_DataBuf = NULL;
	m_Key = 0;
}

void safemem_varbase::Register()
{
	Alloc();   //Alloc mem
	Encrypt(); //Ecrypt for the first time.  Sets m_CheckSum and m_Key
}

void safemem_varbase::Alloc()
{
	startdbg;

	ulong DataSize = DataSize_Get();

	dbg("Alloc memory");
	byte *pData = msnew(byte[DataSize]); //Alloc new mem

	if (!pData)
	{
		exit(0);
		return;
	}

	dbg("Set mem size");
	DataLoc_Set((ulong)pData); //Set New mem location

	if (m_DataBuf)
		delete m_DataBuf;			   //Delete old buffer
	m_DataBuf = msnew(byte[DataSize]); //Allocate new buffer

	enddbg;
}

void safemem_varbase::Encrypt()
{
	if (!m_DataBuf)
		return;

	ulong Size = DataSize_Get(); //Get mem size
	if (!Size)
		return; //Size is zero - should never happen

	byte *pData = DataLoc_Get();	//Get mem location
	memcpy(pData, m_DataBuf, Size); //Copy from buffer

	ulong NewKey = GetTickCount(); //Validate old key
	if (m_Key > NewKey)
	{
		Sleep(3000);
		exit(0); //Harshly exit, on failure
	}

	m_Key = NewKey; //Set new key

	//Start encryption
	m_CheckSum = m_Key; //Initialize Checksum

	for (size_t i = 0; i < Size; i++)
	{
		byte addamt = (i * 4) % 128;		   //Offset some data
		pData[i] = (pData[i] + addamt) & 0xFF; //(The 0xFF is to explicitly convert the result to a byte)
		m_CheckSum -= pData[i];				   //Update CheckSum
	}

	for (size_t i = 0; i < Size; i++) //Swap some data around
	{
		int swap[2] = {i, (i + 7) % Size};
		byte Temp = pData[swap[0]];
		pData[swap[0]] = pData[swap[1]];
		pData[swap[1]] = Temp;
	}

	DataSize_Set(Size);		   //Store size again with new key
	DataLoc_Set((ulong)pData); //Store location again with new key
}

//Make sure to deallocate m_DataBuf after calling this
void safemem_varbase::Decrypt() const
{
	startdbg;

	dbg("Get Size");
	ulong Size = DataSize_Get(); //Get mem size
	if (!Size)
		return;

	byte *pData = DataLoc_Get(); //Get mem pointer

	//Decrypt new data
	ulong CheckSum = m_Key; //Initialize Checksum

	dbg(msstring("Copy (Size: ") + (int)Size + ")");
	memcpy(m_DataBuf, pData, Size); //Copy to buffer

	pData = m_DataBuf; //Do operations on the buffer

	dbg("UnSwap");
	for (int i = Size - 1; i >= 0; i--) //Unswap the data
	{
		int swap[2] = {i, (i + 7) % Size};
		byte Temp = pData[swap[0]];
		pData[swap[0]] = pData[swap[1]];
		pData[swap[1]] = Temp;
	}

	dbg("Un-Offset");
	for (int i = Size - 1; i >= 0; i--)
	{
		CheckSum -= pData[i];	   //Update CheckSum
		pData[i] -= (i * 4) % 128; //Un-Offset the data
	}

	dbg("Validate");
	if (CheckSum != m_CheckSum) //Validate Checksum
	{
		Sleep(3000);
		exit(0); //Harshly exit, on failure
	}

	enddbg;
}

void safemem_varbase::UnRegister()
{
	byte *pData = DataLoc_Get(); //Get mem pointer

//Don't know why, but I get an error deleting these on the client.  Why??
#ifdef VALVE_DLL
	if (pData)
		delete pData; //Delete the mem
	if (m_DataBuf)
		delete m_DataBuf; //Delete the buffer
#endif

	DataLoc_Set(0L);
}
void safemem_varbase::ChangeData(void *pData)
{
	Decrypt(); //Decrypt mem

	ulong Size = DataSize_Get(); //Get Data Size

	for (int i = 0; i < Size; i++) //Check if any data changed
		if (m_DataBuf[i] != ((byte *)pData)[i])
		{
			m_Changed = true;
			break;
		}

	memcpy(m_DataBuf, pData, Size); //Copy in Data

	Encrypt(); //Re-Encrypt mem
}
void safemem_varbase::GetData(void *pBuffer) const
{
	Decrypt(); //Decrypt mem

	ulong Size = DataSize_Get(); //Get Data Size

	memcpy(pBuffer, m_DataBuf, Size); //Copy out Data
}

void safemem_varbase::DataLoc_Set(ulong Loc) { m_EncrypredLoc = m_Key - Loc; }
void safemem_varbase::DataSize_Set(ulong Size) { m_EncrypredSize = m_Key - Size; }
byte *safemem_varbase::DataLoc_Get() const { return (byte *)(m_Key - m_EncrypredLoc); }
ulong safemem_varbase::DataSize_Get() const { return m_Key - m_EncrypredSize; }
