/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
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
//
//  cdll_int.c
//
// this implementation handles the linking of the engine to the DLL
//

#include "hud.h"
#include "cl_util.h"
#include "netadr.h"
#include "vgui_schememanager.h"
#include "logger.h"
#include <windows.h>

//#define LOG_ALLEXPORTS //more exports in entity.cpp

#ifdef LOG_ALLEXPORTS
#define logfileopt logfile
#else
#define logfileopt NullFile
#endif

extern "C"
{
#include "pm_shared.h"
}

#include <string.h>
#include "hud_servers.h"
#include "vgui_int.h"
#include "interface.h"
#include "voice_status.h"
#include "filesystem_shared.h"

#define DLLEXPORT EXPORT

cl_enginefunc_t gEngfuncs;
CHud gHUD;
TeamFortressViewport *gViewPort = NULL;
extern float g_fMenuLastClosed;

// IMAGE-SPACE GLOW - Thothie TWHL JUN2010_22 - see comments in CLRender.cpp
extern void InitScreenGlow(void);
extern void RenderScreenGlow(void);

void InitInput(void);
void EV_HookEvents(void);
void IN_Commands(void);

static cvar_s *g_pVarBorderless = NULL;
static int g_iBorderlessMode = 0;

enum BORDERLESS_WINDOW_TYPES
{
	BORDERLESS_SHOWTASKBAR = 1,
	BORDERLESS_FULLSCREEN,
	BORDERLESS_RESIZABLE,
};

/*
========================== 
    Initialize

Called when the DLL is first loaded.
==========================
*/
extern "C"
{
	int DLLEXPORT Initialize(cl_enginefunc_t *pEnginefuncs, int iVersion);
	int DLLEXPORT HUD_VidInit(void);
	void DLLEXPORT HUD_Init(void);
	int DLLEXPORT HUD_Redraw(float flTime, int intermission);
	int DLLEXPORT HUD_UpdateClientData(client_data_t *cdata, float flTime);
	void DLLEXPORT HUD_Reset(void);
	void DLLEXPORT HUD_PlayerMove(struct playermove_s *ppmove, int server);
	void DLLEXPORT HUD_PlayerMoveInit(struct playermove_s *ppmove);
	char DLLEXPORT HUD_PlayerMoveTexture(char *name);
	int DLLEXPORT HUD_ConnectionlessPacket(const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size);
	int DLLEXPORT HUD_GetHullBounds(int hullnumber, float *mins, float *maxs);
	void DLLEXPORT HUD_Frame(double time);
	void DLLEXPORT HUD_VoiceStatus(int entindex, qboolean bTalking);
	void DLLEXPORT HUD_DirectorMessage(int iSize, void *pbuf);
}

/*
==========================
Set Borderless Modes
==========================
*/
static void SetBorderlessWindow() // Bernt; fixing bloom by making fullscreen windowed a thing *shrugs*!
{
	int iCurrentMode = (g_pVarBorderless ? ((int)g_pVarBorderless->value) : 0);
	if ((g_iBorderlessMode == iCurrentMode) || (iCurrentMode <= 0))
		return;

	HWND handle = GetActiveWindow();
	if (!handle)
		return;

	RECT area;
	const int w = GetSystemMetrics(SM_CXSCREEN);
	const int h = GetSystemMetrics(SM_CYSCREEN);
	SystemParametersInfoA(SPI_GETWORKAREA, 0, &area, 0);

	switch (iCurrentMode)
	{

	case BORDERLESS_SHOWTASKBAR:
	{
		SetWindowLongPtr(handle, GWL_STYLE, WS_VISIBLE | WS_POPUP);
		SetWindowPos(handle, HWND_TOP, 0, 0, w, area.bottom, SWP_FRAMECHANGED);
		break;
	}

	case BORDERLESS_FULLSCREEN:
	{
		SetWindowLongPtr(handle, GWL_STYLE, WS_VISIBLE | WS_POPUP);
		SetWindowPos(handle, HWND_TOP, 0, 0, w, h, SWP_FRAMECHANGED);
		break;
	}

	case BORDERLESS_RESIZABLE:
	{
		DWORD new_style = (WS_POPUP | WS_THICKFRAME | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX);
		::SetWindowLongPtrW(handle, GWL_STYLE, static_cast<LONG>(new_style));
		::SetWindowPos(handle, nullptr, 0, 0, w, area.bottom, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
		::ShowWindow(handle, SW_SHOW);
		break;
	}
	}

	g_iBorderlessMode = iCurrentMode;
}

/*
================================
HUD_GetHullBounds

  Engine calls this to enumerate player collision hulls, for prediction.  Return 0 if the hullnumber doesn't exist.
================================
*/
int DLLEXPORT HUD_GetHullBounds(int hullnumber, float *mins, float *maxs)
{
	int iret = 0;
	DBG_INPUT;

	startdbg;
	dbg("Begin");

	switch (hullnumber)
	{
	case 0: // Normal player
		mins = Vector(-16, -16, -36);
		maxs = Vector(16, 16, 36);
		iret = 1;
		break;
	case 1: // Crouched player
		mins = Vector(-16, -16, -18);
		maxs = Vector(16, 16, 18);
		iret = 1;
		break;
	case 2: // Point based hull
		mins = Vector(0, 0, 0);
		maxs = Vector(0, 0, 0);
		iret = 1;
		break;
	}

	logfile << Logger::LOG_INFO << "[HUD_GetHullBounds: Complete]\n";

	enddbg;
	return iret;
}

/*
================================
HUD_ConnectionlessPacket

 Return 1 if the packet is valid.  Set response_buffer_size if you want to send a response packet.  Incoming, it holds the max
  size of the response_buffer, so you must zero it out if you choose not to respond.
================================
*/
int DLLEXPORT HUD_ConnectionlessPacket(const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size)
{
	DBG_INPUT;
	startdbg;
	logfileopt << "HUD_ConnectionlessPacket\r\n";
	// Parse stuff from args
	int max_buffer_size = *response_buffer_size;

	// Zero it out since we aren't going to respond.
	// If we wanted to response, we'd write data into response_buffer
	*response_buffer_size = 0;

	// Since we don't listen for anything here, just respond that it's a bogus message
	// If we didn't reject the message, we'd return 1 for success instead.
	logfileopt << "HUD_ConnectionlessPacket END\r\n";
	enddbg;
	return 0;
}

void DLLEXPORT HUD_PlayerMoveInit(struct playermove_s *ppmove)
{
	DBG_INPUT;
	startdbg;
	dbg("Begin");
	PM_Init(ppmove);
	enddbg;
}

char DLLEXPORT HUD_PlayerMoveTexture(char *name)
{
	DBG_INPUT;
	char ret;
	startdbg;
	ret = PM_FindTextureType(name);
	enddbg;
	return ret;
}

void DLLEXPORT HUD_PlayerMove(struct playermove_s *ppmove, int server)
{
	DBG_INPUT;
	startdbg;

	dbg("Begin");
	//Half-life sets the dead flag if player healh is < 1.  In MS its < 0
	PM_Move(ppmove, server);
	enddbg;
}

int DLLEXPORT Initialize(cl_enginefunc_t *pEnginefuncs, int iVersion)
{
	DBG_INPUT;
	startdbg;

	gEngfuncs = *pEnginefuncs;

	if (iVersion != CLDLL_INTERFACE_VERSION)
		return 0;

	memcpy(&gEngfuncs, pEnginefuncs, sizeof(cl_enginefunc_t));

	EV_HookEvents();
	g_pVarBorderless = CVAR_CREATE("ms_borderless", "0", FCVAR_ARCHIVE);
	
	if(!FileSystem_Init())
	{
		logfile << Logger::LOG_ERROR << "[DLLEXPORT Initialize: Failed to initialize filesystem]\n";
		return 0;
	}

	logfile << Logger::LOG_INFO << "[DLLEXPORT Initialize: Complete]\n";

	enddbg;
	return 1;
}

/*
==========================
	HUD_VidInit

Called when the game initializes
and whenever the vid_mode is changed
so the HUD can reinitialize itself.
==========================
*/

int DLLEXPORT HUD_VidInit(void)
{
	DBG_INPUT;
	startdbg;

	dbg("Call gHUD.VidInit");
	gHUD.VidInit();

	dbg("Call VGui_Startup");
	VGui_Startup();

	dbg("Call Glow");
	// IMAGE-SPACE GLOW - Thothie TWHL JUN2010_22 - see comments in tri.cpp
	InitScreenGlow();

	logfile << Logger::LOG_INFO << "[HUD_VidInit: Complete]\n";

	enddbg;

	return 1;
}

/*
==========================
	HUD_Init

Called whenever the client connects
to a server.  Reinitializes all 
the hud variables.
==========================
*/

void DLLEXPORT HUD_Init(void)
{
	g_fMenuLastClosed = 0.0f;

	DBG_INPUT;
	startdbg;

	logfile << Logger::LOG_INFO << "[HUD_Init: InitInput]\n";
	dbg("Call InitInput");
	InitInput();

	logfile << Logger::LOG_INFO << "[HUD_Init: gHUD.Init]\n";
	dbg("Call gHUD.Init");
	gHUD.Init();

	logfile << Logger::LOG_INFO << "[HUD_Init: Scheme_Init]\n";
	dbg("Call Scheme_Init");
	Scheme_Init();

	logfile << Logger::LOG_INFO << "[HUD_Init: Complete]\n";

	enddbg;
}

/*
==========================
	HUD_Redraw

called every screen frame to
redraw the HUD.
===========================
*/

int DLLEXPORT HUD_Redraw(float time, int intermission)
{
	DBG_INPUT;
	startdbg;

	dbg("Call gHUD.Redraw");

	logfileopt << "HUD_Redraw...";

	// IMAGE-SPACE GLOW - Thothie TWHL JUN2010_22 - see comments in tri.cpp
	RenderScreenGlow();

	gHUD.Redraw(time, intermission);

	//logfileopt << "END\r\n";
	enddbg;
	return 1;
}

/*
==========================
	HUD_UpdateClientData

called every time shared client
dll/engine data gets changed,
and gives the cdll a chance
to modify the data.

returns 1 if anything has been changed, 0 otherwise.
==========================
*/

int DLLEXPORT HUD_UpdateClientData(client_data_t *pcldata, float flTime)
{
	int ret = 0;
	DBG_INPUT;
	startdbg;

	dbg("Call IN_Commands");
	IN_Commands();

	dbg("Call gHUD.UpdateClientData");
	ret = gHUD.UpdateClientData(pcldata, flTime);

	enddbg;
	return ret;
}

/*
==========================
	HUD_Reset

Called at start and end of demos to restore to "non"HUD state.
==========================
*/

void DLLEXPORT HUD_Reset(void)
{
	DBG_INPUT;
	startdbg;
	dbg("Call VidInit");

	gHUD.VidInit();

	logfile << Logger::LOG_INFO << "[HUD_Reset: Complete]\n";

	enddbg;
}

/*
==========================
HUD_Frame

Called by engine every frame that client .dll is loaded
==========================
*/

void DLLEXPORT HUD_Frame(double time)
{
	DBG_INPUT;
	startdbg;

	dbg("Call ServersThink");
	ServersThink(time);
	dbg("Call ServersThink DONE");

	dbg("Call SetBorderlessWindow");
	SetBorderlessWindow();
	dbg("Call SetBorderlessWindow DONE");

	enddbg;
}

/*
==========================
HUD_VoiceStatus

Called when a player starts or stops talking.
==========================
*/

void DLLEXPORT HUD_VoiceStatus(int entindex, qboolean bTalking)
{
	DBG_INPUT;
	startdbg;

	dbg("Call GetClientVoiceMgr()->UpdateSpeakerStatus");
	GetClientVoiceMgr()->UpdateSpeakerStatus(entindex, bTalking);

	enddbg;
}

/*
==========================
HUD_DirectorEvent

Called when a director event message was received
==========================
*/

void DLLEXPORT HUD_DirectorMessage(int iSize, void *pbuf)
{
	DBG_INPUT;
	startdbg;

	dbg("Call gHUD.m_Spectator.DirectorMessage");
	gHUD.m_Spectator.DirectorMessage(iSize, pbuf);

	enddbg;
}