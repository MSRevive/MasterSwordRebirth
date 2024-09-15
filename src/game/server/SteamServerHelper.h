#ifndef MS_STEAM_SERVER_HELPER_H
#define MS_STEAM_SERVER_HELPER_H

#include <steam/steam_api.h>
#include <steam/steam_gameserver.h>

// class CSteamGameServerAPIContext;
// class CSteamAPIContext;
// class ISteamHTTP;

class CSteamServerHelper 
{
public:
	CSteamServerHelper();
	~CSteamServerHelper() = default;

	void Init(void);
	void Shutdown(void);
	void Think(void);
	void RunCallbacks(void);
};

extern CSteamAPIContext* steamapicontext;
extern CSteamGameServerAPIContext* steamgameserverapicontext;
extern ISteamHTTP* g_SteamHTTPContext;
extern CSteamServerHelper* g_SteamServerHelper;

#endif // MS_STEAM_SERVER_HELPER_H