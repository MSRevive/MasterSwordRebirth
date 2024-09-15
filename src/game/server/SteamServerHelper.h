#ifndef MS_STEAM_SERVER_HELPER_H
#define MS_STEAM_SERVER_HELPER_H

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

class CSteamGameServerAPIContext;
class CSteamAPIContext;
class ISteamHTTP;

extern CSteamAPIContext* steamapicontext;
extern CSteamGameServerAPIContext* steamgameserverapicontext;
extern ISteamHTTP* g_SteamHTTPContext;
extern CSteamServerHelper* g_SteamServerHelper;

#endif // MS_STEAM_SERVER_HELPER_H