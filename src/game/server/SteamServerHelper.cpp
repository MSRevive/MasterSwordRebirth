#include "SteamServerHelper.h"
#include "msdllheaders.h"

static CSteamAPIContext s_SteamAPIContext;
static CSteamGameServerAPIContext s_SteamGameServerAPIContext;

extern "C" void __cdecl SteamAPIDebugTextHook( int nSeverity, const char *pchDebugText )
{
	g_engfuncs.pfnServerPrint(pchDebugText);
}

CSteamServerHelper::CSteamServerHelper()
{
	m_SteamGameServerContext = &s_SteamGameServerAPIContext;
	m_SteamContext = &s_SteamAPIContext;
}

void CSteamServerHelper::Init(void)
{
	s_SteamAPIContext.Init();
	g_engfuncs.pfnServerPrint("steam init\n");
	s_SteamGameServerAPIContext.Init();
	m_SteamGameServerContext->SteamGameServerUtils()->SetWarningMessageHook(&SteamAPIDebugTextHook);
	// if (!IS_DEDICATED_SERVER())
	// {
	// }else{
		
	// }

	m_bLoaded = true;
}

void CSteamServerHelper::Shutdown(void)
{
	if (m_bLoaded)
	{
		s_SteamAPIContext.Clear();
		s_SteamGameServerAPIContext.Clear();

		m_SteamGameServerContext = nullptr;
		m_SteamContext = nullptr;
		m_bLoaded = false;
	}
}

void CSteamServerHelper::Think(void)
{
	ISteamHTTP* m_steamHTTP = m_SteamGameServerContext->SteamHTTP();
	if (m_steamHTTP == nullptr)
		g_engfuncs.pfnServerPrint("http is null \n");
}

// Should be handled by the engine?
void CSteamServerHelper::RunCallbacks(void)
{
	if (m_SteamGameServerContext->SteamHTTP())
		SteamGameServer_RunCallbacks();

	if (m_SteamContext->SteamHTTP())
		SteamAPI_RunCallbacks();
}

ISteamHTTP* CSteamServerHelper::GetHTTP(void)
{
	return (IS_DEDICATED_SERVER() ? m_SteamGameServerContext->SteamHTTP() : m_SteamContext->SteamHTTP());
}