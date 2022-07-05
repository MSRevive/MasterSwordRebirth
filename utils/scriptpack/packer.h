#ifndef PACKER_H
#define PACKER_H

#include "sharedutil.h"
#include "msfileio.h"

typedef unsigned char byte;

class Packer 
{
public:
  Packer(char *wDir, char *rDir, char *oDir) {
    memcpy(m_WorkDir, wDir, MAX_PATH);
    memcpy(m_RootDir, rDir, MAX_PATH);
    memcpy(m_OutDir, oDir, MAX_PATH);
  }
  //void readDirectory(char *pszName);
  void packScripts();
  void cookScripts();
  void saveErrors();
  
private:
  char m_WorkDir[MAX_PATH];
  char m_RootDir[MAX_PATH];
  char m_OutDir[MAX_PATH];
  msstringlist m_StoredFiles;
  
  void readDirectory(char *pszName);
  void storeFile(char *pszCurrentDir, WIN32_FIND_DATA &wfd);
};

#endif