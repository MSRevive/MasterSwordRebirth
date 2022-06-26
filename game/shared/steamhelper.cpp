#include "steamhelper.h"
#include "strhelper.h"
#include "steam/steam_api.h"
#include "steam/steam_gameserver.h"
#include "msdllheaders.h"

// this context is not available on dedicated servers
// WARNING! always check if interfaces are available before using
static CSteamAPIContext s_SteamAPIContext;
CSteamAPIContext* steamapicontext = &s_SteamAPIContext;

// this context is not available on a pure client connected to a remote server.
// WARNING! always check if interfaces are available before using
static CSteamGameServerAPIContext s_SteamGameServerAPIContext;
CSteamGameServerAPIContext* steamgameserverapicontext = &s_SteamGameServerAPIContext;

static float g_fDelayedInitialize = 0.0f;

void SteamHelper::Initialize()
{
#ifdef VALVE_DLL
	g_fDelayedInitialize = (gpGlobals->time + 3.5);
#else
	g_fDelayedInitialize = 0.1f; // No delay on the client, but cannot be 0.
#endif
}

void SteamHelper::Shutdown()
{
	s_SteamAPIContext.Clear();
	s_SteamGameServerAPIContext.Clear();
	g_fDelayedInitialize = 0.0f;
}

void SteamHelper::Think()
{
#ifdef VALVE_DLL
	if ((g_fDelayedInitialize > 0) && (gpGlobals->time > g_fDelayedInitialize))
#else
	if (g_fDelayedInitialize > 0)
#endif
	{
		g_fDelayedInitialize = 0.0f;

		s_SteamAPIContext.Init();
		s_SteamGameServerAPIContext.Init();

		if (steamgameserverapicontext->SteamGameServer())
			//ALERT(at_console, "\nInitialized Steam GameServer Handler!\n");
			g_engfuncs.pfnServerPrint("\nInitialized Steam GameServer Handler!\n");
		else if (steamapicontext->SteamUser())
			g_engfuncs.pfnServerPrint("\nInitialized Steam Client Handler!\n");
			//ALERT(at_console, "\nInitialized Steam Client Handler!\n");

		return;
	}
}

//void SteamHelper::SendServerDetails() // You can call this to override the steam server info sent to the master server!
//{
//#ifdef VALVE_DLL
//	if (steamgameserverapicontext->SteamGameServer() == NULL)
//		return;
// 
//	steamgameserverapicontext->SteamGameServer()->SetProduct("msrebirth");
//	steamgameserverapicontext->SteamGameServer()->SetModDir("msrebirth");
//	steamgameserverapicontext->SteamGameServer()->SetGameDescription("Master Sword: Rebirth");
//	steamgameserverapicontext->SteamGameServer()->SetServerName("My Test Server");
//	steamgameserverapicontext->SteamGameServer()->ForceHeartbeat();
//#endif
//}
