#include "SteamClientHelper.h"
#include "strhelper.h"
#include "hud.h"
#include "cl_util.h"

static CSteamAPIContext s_SteamAPIContext;
CSteamAPIContext* steamapicontext = &s_SteamAPIContext;

static CSteamClientHelper s_SteamHelper;
CSteamClientHelper* steamhelper = &s_SteamHelper;

static bool g_bTryLoadSteamAPI = false;

CSteamClientHelper::CSteamClientHelper() :
	m_CallbackUserStatsReceived(this, &CSteamClientHelper::OnUserStatsReceived),
	m_CallbackUserStatsStored(this, &CSteamClientHelper::OnUserStatsStored),
	m_CallbackAchievementStored(this, &CSteamClientHelper::OnAchievementStored)
{
	g_bTryLoadSteamAPI = true;
	m_bHasLoadedSteamStats = false;
}

CSteamClientHelper::~CSteamClientHelper()
{
}

void CSteamClientHelper::Think()
{
	if (g_bTryLoadSteamAPI) // Load Steam API
	{
		g_bTryLoadSteamAPI = false;
		s_SteamAPIContext.Init();

		if (steamapicontext && steamapicontext->SteamUserStats())
			steamapicontext->SteamUserStats()->RequestCurrentStats();
	}
}

void CSteamClientHelper::SetAchievement(const char* str)
{
	if (!m_bHasLoadedSteamStats)
		return;

	steamapicontext->SteamUserStats()->SetAchievement(str);
	steamapicontext->SteamUserStats()->StoreStats();
}

void CSteamClientHelper::SetStat(const char* str, int value)
{
	if (!m_bHasLoadedSteamStats)
		return;

	steamapicontext->SteamUserStats()->SetStat(str, value);
	steamapicontext->SteamUserStats()->StoreStats();
}

void CSteamClientHelper::OnUserStatsReceived(UserStatsReceived_t* pUserStatsReceived)
{
	if (m_bHasLoadedSteamStats || !steamapicontext || !steamapicontext->SteamUserStats() || (pUserStatsReceived == NULL) || (pUserStatsReceived->m_eResult != k_EResultOK))
		return;

	m_bHasLoadedSteamStats = true; // Ensure we do not try to load this again!

	// TODO - Get Stats & Achiev progress e.g:
	// Trigger achiev / stat events via usermessage from server, can also be exposed to script system..

	int iKills = 0, iDeaths = 0;
	steamapicontext->SteamUserStats()->GetStat("KILLS", &iKills);
	steamapicontext->SteamUserStats()->GetStat("DEATHS", &iDeaths);
}

void CSteamClientHelper::OnUserStatsStored(UserStatsStored_t* pCallback)
{
	// unused
}

void CSteamClientHelper::OnAchievementStored(UserAchievementStored_t* pCallback)
{
	// unused
}