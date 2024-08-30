#include "SteamHelper.h"
#include "strhelper.h"
#include "msdllheaders.h"

CSteamHelper::CSteamHelper() :
{
	Shutdown();
	m_bLoaded = true;
}

CSteamHelper::~CSteamHelper()
{
}

void CSteamServerHelper::Think(void)
{
	if (m_bLoaded) // Load Steam API
	{
		m_bLoaded = false
		s_SteamGameServerAPIContext.Init();
		s_SteamHTTP = s_SteamGameServerAPIContext->SteamHTTP();
	}
}

void CSteamServerHelper::Shutdown(void)
{
	s_SteamGameServerAPIContext.Clear();
	s_SteamHTTP = nullptr;
	m_bLoaded = false;
}