#include "SteamServerHelper.h"
#include "msdllheaders.h"
#include <steam/steam_api.h>
#include <steam/steam_gameserver.h>

// this context is not available on dedicated servers
// WARNING! always check if interfaces are available before using
static CSteamAPIContext s_SteamAPIContext;
CSteamAPIContext* steamapicontext = &s_SteamAPIContext;

// this context is not available on a pure client connected to a remote server.
// WARNING! always check if interfaces are available before using
static CSteamGameServerAPIContext s_SteamGameServerAPIContext;
CSteamGameServerAPIContext* steamgameserverapicontext = &s_SteamGameServerAPIContext;

static CSteamServerHelper s_SteamHelper;
CSteamServerHelper* g_SteamServerHelper = &s_SteamHelper;

ISteamHTTP* g_SteamHTTPContext = nullptr;
static bool g_bTryLoadSteamAPI = false;

CSteamServerHelper::CSteamServerHelper()
{
	Shutdown();
	g_bTryLoadSteamAPI = true;
}

void CSteamServerHelper::Think(void)
{
	if (g_bTryLoadSteamAPI)
	{
		g_bTryLoadSteamAPI = false;

		s_SteamAPIContext.Init();
		s_SteamGameServerAPIContext.Init();

		g_SteamHTTPContext = (IS_DEDICATED_SERVER() ? steamgameserverapicontext->SteamHTTP() : steamapicontext->SteamHTTP());
	}
}

void CSteamServerHelper::Shutdown(void)
{
	s_SteamAPIContext.Clear();
	s_SteamGameServerAPIContext.Clear();
	g_SteamHTTPContext = nullptr;
	g_bTryLoadSteamAPI = true;
}

// Should be handled by the engine?
void CSteamServerHelper::RunCallbacks(void)
{
	if (steamgameserverapicontext->SteamHTTP())
		SteamGameServer_RunCallbacks();

	if (steamapicontext->SteamHTTP())
		SteamAPI_RunCallbacks();
}