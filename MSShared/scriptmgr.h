#ifndef MSSHARED_SCRIPTMGR_H
#define MSSHARED_SCRIPTMGR_H

#include "GroupFile.h"

class CScript;

//Script Manager - Loads and Unloads groupfile when neccesary
class ScriptMgr
{
public:
	static CGameGroupFile m_GroupFile;
	static int m_TotalScripts;

	static void RegisterScript(CScript *NewScript);
	static void UnRegisterScript(CScript *NewScript);

	static void GameShutdown();
};

#endif //MSSHARED_SCRIPTMGR_H
