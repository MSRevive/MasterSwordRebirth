#ifndef MSSHARED_CRCHASH_SHARED_H
#define MSSHARED_CRCHASH_SHARED_H

bool MatchFileCheckSum(const char* FilePath, unsigned int CheckSum);
unsigned int GetFileCheckSum(const char* FilePath);

#endif