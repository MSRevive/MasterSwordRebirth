#ifndef PACKER_H
#define PACKER_H

#include <angelscript.h>
#include "sharedutil.h"
#include "msfileio.h"
#include "groupfile.h"

typedef unsigned char byte;

class Packer 
{
public:
	Packer(char *wDir, char *rDir, char *oDir);
	void readDirectory(char *pszName, bool cooked = false);
	void processScripts();
	void packScripts();
  
private:
	asIScriptEngine *m_Engine;
	char m_WorkDir[MAX_PATH];
	char m_RootDir[MAX_PATH];
	char m_OutDir[MAX_PATH];
	msstringlist m_StoredFiles;
	msstringlist m_CookedFiles;
};

#endif