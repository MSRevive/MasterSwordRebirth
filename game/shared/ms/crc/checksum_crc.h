//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Generic CRC functions
//
// $NoKeywords: $
//=============================================================================//

#ifndef CHECKSUM_CRC_H
#define CHECKSUM_CRC_H

#ifdef _WIN32
#pragma once
#endif

typedef unsigned int VCRC32_t;

namespace CRC32
{
	void CRC32_Init(VCRC32_t *pulCRC);
	void CRC32_ProcessBuffer(VCRC32_t *pulCRC, const void *p, int len);
	void CRC32_Final(VCRC32_t *pulCRC);
	VCRC32_t CRC32_GetTableEntry(unsigned int slot);

	inline VCRC32_t CRC32_ProcessSingleBuffer(const void *p, int len)
	{
		VCRC32_t crc;
		CRC32_Init(&crc);
		CRC32_ProcessBuffer(&crc, p, len);
		CRC32_Final(&crc);
		return crc;
	}
}

#endif // CHECKSUM_CRC_H