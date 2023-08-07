/***
*
*	Copyright (c) 1999, 2000 Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#include "msdllheaders.h"
#include "game.h"

#include "svglobals.h"

#include "filesystem_shared.h"
#include "scriptmgr.h"

#ifdef LINUX
#include <unistd.h>
#endif

#include "fndatahandler.h"
#include "httprequesthandler.h"

cvar_t displaysoundlist = { "displaysoundlist", "0" };
cvar_t mapcyclefile = { "mapcyclefile", "mapcycle.txt" };
cvar_t servercfgfile = { "servercfgfile", "server.cfg" };
cvar_t lservercfgfile = { "lservercfgfile", "listenserver.cfg" };
// multiplayer server rules
cvar_t fragsleft = { "mp_fragsleft", "0", FCVAR_SERVER | FCVAR_UNLOGGED }; // Don't spam console/log files/users with this changing
cvar_t timeleft = { "mp_timeleft", "0", FCVAR_SERVER | FCVAR_UNLOGGED };	 // "      "
cvar_t timelimit = { "mp_timelimit", "0", FCVAR_SERVER };
// multiplayer server rules
cvar_t teamplay = { "mp_teamplay", "0", FCVAR_SERVER };
//cvar_t	fraglimit	= {"mp_fraglimit","0", FCVAR_SERVER };
//cvar_t	friendlyfire= {"mp_friendlyfire","0", FCVAR_SERVER };
cvar_t falldamage = { "mp_falldamage", "0", FCVAR_SERVER };
//cvar_t	weaponstay	= {"mp_weaponstay","0", FCVAR_SERVER };
cvar_t forcerespawn = { "mp_forcerespawn", "1", FCVAR_SERVER };
//cvar_t	flashlight	= {"mp_flashlight","0", FCVAR_SERVER };
//cvar_t	aimcrosshair= {"mp_autocrosshair","1", FCVAR_SERVER };
cvar_t decalfrequency = { "decalfrequency", "30", FCVAR_SERVER };
//cvar_t	defaultteam = {"mp_defaultteam","0" };

//cvar_t  mp_chattime = {"mp_chattime","10", FCVAR_SERVER };

cvar_t* g_psv_gravity = NULL;
cvar_t* g_psv_aim = NULL;
cvar_t* g_footsteps = NULL;
cvar_t* g_maxspeed = NULL;
cvar_t* g_accelerate = NULL;
cvar_t* g_airaccelerate = NULL;
cvar_t* g_wateraccelerate = NULL;
cvar_t* g_stepsize = NULL;
cvar_t* g_friction = NULL;
cvar_t* g_stopspeed = NULL;
cvar_t* g_waterfriction = NULL;

char g_pTempStringLimit[WRITE_STRING_MAX];

void wait(unsigned long ms)
{
#ifdef _WIN32
	Sleep(ms); // millisec
#else
	usleep(ms * 1000); // microsec -> millisec
#endif
}

// Register your console variables here
// This gets called one time when the game is initialied
void GameDLLInit(void)
{
	DBG_INPUT;

	//Master Sword -- Initialize the Master Sword Global items.
	if (!MSGlobalInit())
	{
		ALERT(at_console, "Mastersword Initialization FAILED!\n");
		wait(2000);
		SERVER_COMMAND("exit\n");
		return;
	}
	//-------------------------------------

	// Register cvars here:
	g_psv_gravity = CVAR_GET_POINTER("sv_gravity");
	g_psv_aim = CVAR_GET_POINTER("sv_aim");
	g_footsteps = CVAR_GET_POINTER("mp_footsteps");
	g_maxspeed = CVAR_GET_POINTER("sv_maxspeed");
	g_accelerate = CVAR_GET_POINTER("sv_accelerate");
	g_airaccelerate = CVAR_GET_POINTER("sv_airaccelerate");
	g_wateraccelerate = CVAR_GET_POINTER("sv_wateraccelerate");
	g_stepsize = CVAR_GET_POINTER("sv_stepsize");
	g_airaccelerate = CVAR_GET_POINTER("sv_airaccelerate");
	g_friction = CVAR_GET_POINTER("sv_friction");
	g_stopspeed = CVAR_GET_POINTER("sv_stopspeed");
	g_waterfriction = CVAR_GET_POINTER("sv_waterfriction");

	g_pTempStringLimit[0] = 0;

	CVAR_REGISTER(&displaysoundlist);

	CVAR_REGISTER(&teamplay);
	CVAR_REGISTER(&timelimit);

	CVAR_REGISTER(&fragsleft);
	CVAR_REGISTER(&timeleft);

	//CVAR_REGISTER (&friendlyfire);
	CVAR_REGISTER(&falldamage);
	//CVAR_REGISTER (&weaponstay);
	CVAR_REGISTER(&forcerespawn);
	//CVAR_REGISTER (&flashlight);
	//CVAR_REGISTER (&aimcrosshair);
	CVAR_REGISTER(&decalfrequency);
	//CVAR_REGISTER (&defaultteam);

	//CVAR_REGISTER (&mp_chattime);

	if (!FileSystem_Init())
	{
		//Queue up a shutdown command so we don't wind up crashing later on - Solokiller
		//Don't call g_engfuncs.pfnServerExecute; the engine is still initializing, let it finish
		UTIL_LogPrintf("Failed to initialize filesystem\n");
		g_engfuncs.pfnServerCommand("exit\n");
		return;
	}
	
	FnDataHandler::Initialize();
}

void GameDLLShutdown()
{
	g_pTempStringLimit[0] = 0;
	ScriptMgr::GameShutdown();
	FileSystem_Shutdown();
	FnDataHandler::Destroy();
}