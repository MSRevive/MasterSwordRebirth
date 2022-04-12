#ifndef MS_STEAM_HELPER_H
#define MS_STEAM_HELPER_H

namespace SteamHelper
{
	void Initialize();
	void Shutdown();
	void Think();
}

class CSteamAPIContext;
class CSteamGameServerAPIContext;

extern CSteamAPIContext* steamapicontext;
extern CSteamGameServerAPIContext* steamgameserverapicontext;

#endif // MS_STEAM_HELPER_H