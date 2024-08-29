#ifndef MS_STEAM_HELPER_H
#define MS_STEAM_HELPER_H

#include "steam/steam_api.h"

class CSteamHelper
{
public:
	CSteamHelper();
	~CSteamHelper();

	void Think();
	void SetAchievement(const char* str);
	void SetStat(const char* str, int value);

private:
	bool m_bHasLoadedSteamStats;
	STEAM_CALLBACK(CSteamHelper, OnUserStatsReceived, UserStatsReceived_t, m_CallbackUserStatsReceived);
	STEAM_CALLBACK(CSteamHelper, OnUserStatsStored, UserStatsStored_t, m_CallbackUserStatsStored);
	STEAM_CALLBACK(CSteamHelper, OnAchievementStored, UserAchievementStored_t, m_CallbackAchievementStored);
};

extern CSteamAPIContext* steamapicontext;
extern CSteamHelper* steamhelper;

#endif // MS_STEAM_HELPER_H