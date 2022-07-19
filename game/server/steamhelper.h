#ifndef MS_STEAM_HELPER_H
#define MS_STEAM_HELPER_H

class CBasePlayer;
class CSteamAPIContext;
class CSteamGameServerAPIContext;

extern CSteamAPIContext* steamapicontext;
extern CSteamGameServerAPIContext* steamgameserverapicontext;

namespace SteamHelper
{
	void Initialize();
	void Shutdown();
	void Think();

	void SetAchievement(CBasePlayer* pPlayer, const char* str);
	void SetStat(CBasePlayer* pPlayer, const char* str, int value);
}

#endif // MS_STEAM_HELPER_H