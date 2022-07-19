#include "steamhelper.h"
#include "strhelper.h"
#include "steam/steam_api.h"
#include "steam/steam_gameserver.h"
#include "msdllheaders.h"
#include "player.h"

// this context is not available on dedicated servers
// WARNING! always check if interfaces are available before using
static CSteamAPIContext s_SteamAPIContext;
CSteamAPIContext* steamapicontext = &s_SteamAPIContext;

// this context is not available on a pure client connected to a remote server.
// WARNING! always check if interfaces are available before using
static CSteamGameServerAPIContext s_SteamGameServerAPIContext;
CSteamGameServerAPIContext* steamgameserverapicontext = &s_SteamGameServerAPIContext;

static bool g_bTryLoadSteamAPI = false;

void SteamHelper::Initialize()
{
	Shutdown();
	g_bTryLoadSteamAPI = true;
}

void SteamHelper::Shutdown()
{
	s_SteamAPIContext.Clear();
	s_SteamGameServerAPIContext.Clear();
	g_bTryLoadSteamAPI = false;
}

void SteamHelper::Think()
{
	if (g_bTryLoadSteamAPI) // Load Steam API
	{
		g_bTryLoadSteamAPI = false;
		s_SteamAPIContext.Init();
		s_SteamGameServerAPIContext.Init();
	}
}

void SteamHelper::SetAchievement(CBasePlayer* pPlayer, const char* str)
{
	if (pPlayer == NULL)
		return;

	if (IS_DEDICATED_SERVER())
	{
		CSteamID steamUser((uint64)pPlayer->steamID64); // TODO - Is this enough?
		steamgameserverapicontext->SteamGameServerStats()->SetUserAchievement(steamUser, str);
		return;
	}

	steamapicontext->SteamUserStats()->SetAchievement(str);
}

void SteamHelper::SetStat(CBasePlayer* pPlayer, const char* str, int value)
{
	if (pPlayer == NULL)
		return;

	if (IS_DEDICATED_SERVER())
	{
		CSteamID steamUser((uint64)pPlayer->steamID64); // TODO - Is this enough?
		steamgameserverapicontext->SteamGameServerStats()->SetUserStat(steamUser, str, value);
		return;
	}

	steamapicontext->SteamUserStats()->SetStat(str, value);
}