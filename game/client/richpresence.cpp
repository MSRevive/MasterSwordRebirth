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
#include "steamhelper.h"
#include "discord/discord_rpc.h"
#include <time.h>
#include <sysinfoapi.h>

static bool loadedRichPresence = false;
static ULONG lastTimeUpdated = 0;
static int64_t startTime;

static char bufferDetails[128];
static char bufferState[128];

#define DISCORD_RPC_UPDATE_TIME 35 // sec

void RichPresenceInitialize()
{
	loadedRichPresence = true;
	DiscordEventHandlers handlers;
	memset(&handlers, 0, sizeof(handlers));
	Discord_Initialize("1063225093507010570", &handlers, 1, "1961680");
	startTime = time(NULL);
}

void RichPresenceShutdown()
{
	// Discord_Shutdown is broken --- game will hang endlessly if called!
	loadedRichPresence = false;
}

void RichPresenceUpdate()
{
	if (loadedRichPresence == false)
		return;

	const auto timeNow = (GetTickCount64() / 1000);

	if (lastTimeUpdated >= timeNow)
		return;

	lastTimeUpdated = (timeNow + DISCORD_RPC_UPDATE_TIME);
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

		_snprintf(bufferDetails, sizeof(bufferDetails), "%s - %i HP", player.m_DisplayName.c_str(), currentHealth);
		_snprintf(bufferState, sizeof(bufferState), "%s", MSGlobals::MapName.c_str());

		if (steamapicontext && steamapicontext->SteamFriends())
		{
			steamapicontext->SteamFriends()->SetRichPresence("name", player.m_DisplayName.c_str());
			steamapicontext->SteamFriends()->SetRichPresence("zone", MSGlobals::MapName.c_str());
			steamapicontext->SteamFriends()->SetRichPresence("steam_display", "#Status_InGame");
		}
	}
	else
	{
		strncpy(bufferDetails, "In Main-Menu", sizeof(bufferDetails));
		strncpy(bufferState, "Awaiting Greatness", sizeof(bufferState));
		clientsInGame = maxClients = 1;

		if (steamapicontext && steamapicontext->SteamFriends())
			steamapicontext->SteamFriends()->SetRichPresence("steam_display", "#Status_MainMenu");
	}

	DiscordRichPresence discordPresence;
	memset(&discordPresence, 0, sizeof(discordPresence));
	discordPresence.state = bufferState;
	discordPresence.details = bufferDetails;
	discordPresence.startTimestamp = startTime;
	discordPresence.partySize = clientsInGame;
	discordPresence.partyMax = maxClients;
	discordPresence.largeImageKey = "msr";
	Discord_UpdatePresence(&discordPresence);
}