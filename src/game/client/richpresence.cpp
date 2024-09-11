//
// Simple Rich Presence Exposure!
//

#include "msdllheaders.h"
#include "inc_weapondefs.h"
#include "pm_defs.h"
#include "global.h"
#include "event_api.h"
#include "hud.h"
#include "hud_iface.h"
#include "vgui_scorepanel.h"
#include "richpresence.h"
#include "SteamClientHelper.h"
#include "discord/discord_rpc.h"
#include <time.h>

#define DISCORD_RPC_UPDATE_TIME 35.0 // sec

void CRichPresence::Init(void)
{
	if (!m_bLoaded)
	{
		m_bLoaded = true;
		DiscordEventHandlers handlers;
		memset(&handlers, 0, sizeof(handlers));
		Discord_Initialize("1063225093507010570", &handlers, 1, "1961680");
		m_numStartTime = time(NULL);
	}
}

void CRichPresence::Shutdown(void)
{
	//Discord_Shutdown is broken --- game will hang endlessly if called!
	//Seems to work fine now
	Discord_Shutdown();
	m_bLoaded = false;
}

void CRichPresence::Update(void)
{
	if (!m_bLoaded)
		return;

	const float timeNow = gEngfuncs.GetClientTime();
	if (m_fLastUpdate >= timeNow)
		return;
	
	m_fLastUpdate = (timeNow + DISCORD_RPC_UPDATE_TIME);
	int maxClients = gEngfuncs.GetMaxClients(), clientsInGame = 0, currentHealth = 0;

	if (maxClients > 0)
	{
		for (int i = 1; i < MAX_PLAYERS; i++)
		{
			if (g_PlayerInfoList[i].name == NULL)
				continue;

			if (g_PlayerInfoList[i].thisplayer)
				currentHealth = g_PlayerExtraInfo[i].HP;

			clientsInGame++;
		}

		_snprintf(m_cBufferDetails, sizeof(m_cBufferDetails), "%s - %i HP", player.m_DisplayName.c_str(), currentHealth);
		_snprintf(m_cBufferState, sizeof(m_cBufferState), "%s", MSGlobals::MapName.c_str());

		if (steamapicontext && steamapicontext->SteamFriends())
		{
			steamapicontext->SteamFriends()->SetRichPresence("name", player.m_DisplayName.c_str());
			steamapicontext->SteamFriends()->SetRichPresence("zone", MSGlobals::MapName.c_str());
			steamapicontext->SteamFriends()->SetRichPresence("steam_display", "#Status_InGame");
		}
	}
	else
	{
		strncpy(m_cBufferDetails, "In Main-Menu", sizeof(m_cBufferDetails));
		strncpy(m_cBufferState, "Awaiting Greatness", sizeof(m_cBufferState));
		clientsInGame = maxClients = 1;

		if (steamapicontext && steamapicontext->SteamFriends())
			steamapicontext->SteamFriends()->SetRichPresence("steam_display", "#Status_MainMenu");
	}

	DiscordRichPresence discordPresence;
	memset(&discordPresence, 0, sizeof(discordPresence));
	discordPresence.state = m_cBufferState;
	discordPresence.details = m_cBufferDetails;
	discordPresence.startTimestamp = m_numStartTime;
	discordPresence.partySize = clientsInGame;
	discordPresence.partyMax = maxClients;
	discordPresence.largeImageKey = "msr";
	Discord_UpdatePresence(&discordPresence);
}