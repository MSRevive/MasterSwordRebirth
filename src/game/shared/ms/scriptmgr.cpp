#include "inc_weapondefs.h"
#include "scriptmgr.h"
#include "logger.h"

int ScriptMgr::m_TotalScripts = 0;

void ScriptMgr::RegisterScript(CScript *NewScript)
{
	if (!g_ScriptPack.IsOpen())
	{
		g_ScriptPack.Open("scripts.pak");
	}
	m_TotalScripts++;
}

void ScriptMgr::UnRegisterScript(CScript *NewScript)
{
	m_TotalScripts--;
	if( m_TotalScripts == 0 )	
		g_ScriptPack.Close();	
}

void ScriptMgr::GameShutdown()
{
	//TODO: needed because scripts aren't freed on shutdown. If scripts are unloaded on shutdown, this is unnecessary - Solokiller
	g_ScriptPack.Close();
}
