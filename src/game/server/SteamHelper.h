#ifndef MS_STEAM_HELPER_H
#define MS_STEAM_HELPER_H

#include "steam/steam_api.h"
#include "steam/steam_gameserver.h"

class CSteamServerHelper
{
public:
	CSteamServerHelper();
	~CSteamServerHelper();

	void Think();
	void HTTPRunCallbacks
	void Shutdown();

private:
	bool m_bLoaded = false;
	static CSteamGameServerAPIContext* s_SteamGameServerAPIContext;
	ISteamHTTP* s_SteamHTTP = nullptr;
};

#endif // MS_STEAM_HELPER_H