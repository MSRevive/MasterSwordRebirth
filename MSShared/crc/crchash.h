#ifndef MSSHARED_CRCHASH_SHARED_H
#define MSSHARED_CRCHASH_SHARED_H

#include <stdint.h> // for uint32_t and uint64_t

bool MatchFileCheckSum(const char *FilePath, uint32_t CheckSum);
uint32_t GetFileCheckSum(const char *FilePath);
uint32_t GetFileCheckSumSize(const char *FilePath);

#endif