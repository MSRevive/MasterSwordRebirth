#ifndef PACKER_H
#define PACKER_H

#include "sharedutil.h"
#include "msfileio.h"
#include "groupfile.h"

typedef unsigned char byte;

class Packer 
{
public:
  Packer(char *wDir, char *rDir, char *oDir) {
    memcpy(m_WorkDir, wDir, MAX_PATH);
    memcpy(m_RootDir, rDir, MAX_PATH);
    memcpy(m_CookedDir, rDir, MAX_PATH);
    memcpy(m_OutDir, oDir, MAX_PATH);
    
    strncat(m_CookedDir, "\\cooked", MAX_PATH);
    
    try {
      CreateDirectory(m_CookedDir, NULL);
    }catch(...)
    {
      printf("Failed to create %s\n", m_CookedDir);
			exit(-1);
    }
  }
  
  void readDirectory(char *pszName, bool cooked);
  void packScripts();
  void cookScripts();
  
private:
  char m_WorkDir[MAX_PATH];
  char m_RootDir[MAX_PATH];
  char m_CookedDir[MAX_PATH];
  char m_OutDir[MAX_PATH];
  msstringlist m_StoredFiles;
  msstringlist m_CookedFiles;
  
  void storeFile(char *pszCurrentDir, WIN32_FIND_DATA &wfd, bool cooked);
  void doParser(byte *buffer, size_t bufferSize, char *name, char *create, bool errOnly);
};

#endif