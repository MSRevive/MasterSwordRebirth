#ifndef MSSHARED_CRCHASH_SHARED_H
#define MSSHARED_CRCHASH_SHARED_H

bool MatchFileCheckSum(const char* FilePath, unsigned long CheckSum);
unsigned long GetFileCheckSum(const char* FilePath);

#endif