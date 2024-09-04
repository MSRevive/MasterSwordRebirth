#ifndef MS_STEAM_SERVER_HELPER_H
#define MS_STEAM_SERVER_HELPER_H

// This is required to use CSteamAPIContext and such, 
// which we have to do this because the API isn't exposed by the engine.
// We could probably expose it ourselves sense we're using ReHLDS, but too lazy.
#define VERSION_SAFE_STEAM_API_INTERFACES

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

	ISteamHTTP* GetHTTP(void);

private:
	bool m_bLoaded = false;
	CSteamGameServerAPIContext* m_SteamGameServerContext;
	CSteamAPIContext* m_SteamContext;
};

extern CSteamServerHelper g_SteamServerHelper;

#endif // MS_STEAM_SERVER_HELPER_H