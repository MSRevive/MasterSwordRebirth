#ifndef MS_STEAM_SERVER_HELPER_H
#define MS_STEAM_SERVER_HELPER_H

#include <steam_api.h>
#include <steam_gameserver.h>

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