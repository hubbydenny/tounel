#pragma once
#include <cstdint>
#include <cstddef>

typedef uint32 CRC32_t;

void CRC32_Init(CRC32_t* pulCRC);
void CRC32_ProcessBuffer(CRC32_t* pulCRC, const void* p, int len);
void CRC32_Final(CRC32_t* pulCRC);
CRC32_t	CRC32_GetTableEntry(unsigned int slot);

inline CRC32_t CRC32_ProcessSingleBuffer(const void* p, int len)
{
	CRC32_t crc;

	CRC32_Init(&crc);
	CRC32_ProcessBuffer(&crc, p, len);
	CRC32_Final(&crc);

	return crc;
}

inline unsigned long CRC32_ConvertToUnsignedLong(CRC32_t* pulCRC)
{
	return (unsigned long)(*pulCRC);
}

inline CRC32_t CRC32_ConvertFromUnsignedLong(unsigned long ulCRC)
{
	return (CRC32_t)(ulCRC);
}