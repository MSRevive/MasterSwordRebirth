#include "steamhelper.h"
#include "strhelper.h"
#include "hud.h"
#include "cl_util.h"

static CSteamAPIContext s_SteamAPIContext;
CSteamAPIContext* steamapicontext = &s_SteamAPIContext;

static CSteamHelper s_SteamHelper;
CSteamHelper* steamhelper = &s_SteamHelper;

static bool g_bTryLoadSteamAPI = false;

CSteamHelper::CSteamHelper() :
	m_CallbackUserStatsReceived(this, &CSteamHelper::OnUserStatsReceived),
	m_CallbackUserStatsStored(this, &CSteamHelper::OnUserStatsStored),
	m_CallbackAchievementStored(this, &CSteamHelper::OnAchievementStored)
{
	g_bTryLoadSteamAPI = true;
	m_bHasLoadedSteamStats = false;
}

CSteamHelper::~CSteamHelper()
{
}

void CSteamHelper::Think()
{
	if (g_bTryLoadSteamAPI) // Load Steam API
	{
		g_bTryLoadSteamAPI = false;
		s_SteamAPIContext.Init();

		if (steamapicontext && steamapicontext->SteamUserStats())
			steamapicontext->SteamUserStats()->RequestCurrentStats();
	}
}

void CSteamHelper::SetAchievement(const char* str)
{
	if (!steamapicontext || !steamapicontext->SteamUserStats())
		return;

	steamapicontext->SteamUserStats()->SetAchievement(str);
}

void CSteamHelper::SetStat(const char* str, int value)
{
	if (!steamapicontext || !steamapicontext->SteamUserStats())
		return;

	steamapicontext->SteamUserStats()->SetStat(str, value);
}

void CSteamHelper::OnUserStatsReceived(UserStatsReceived_t* pUserStatsReceived)
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

void CSteamHelper::OnUserStatsStored(UserStatsStored_t* pCallback)
{
	// unused
}

void CSteamHelper::OnAchievementStored(UserAchievementStored_t* pCallback)
{
	// unused
}