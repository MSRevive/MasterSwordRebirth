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
// hud.cpp
//
// implementation of CHud class
//

//Master Sword
#include "msdllheaders.h"
#include "player/player.h"
//------------

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "vgui_teamfortressviewport.h"

#include "demo.h"
#include "demo_api.h"

//Master Sword
#include "logger.h"
#include "ms/health.h"
#include "ms/fatigue.h"
#include "stats/stats.h"
#include "ms/hudmagic.h"
#include "ms/action.h"
#include "ms/menu.h"
#include "ms/hudscript.h"
#include "ms/hudmisc.h"
#include "ms/hudmusic.h"
#include "ms/hudid.h"
#include "ms/clglobal.h"
#include "ms/vgui_localizedpanel.h" // MiB MAR2015_01 [LOCAL_PANEL] - Include for new panel
#include "voice_status.h"
#include "fmod/soundengine.h"
//-----------------
extern client_sprite_t *GetSpriteList(client_sprite_t *pList, const char *psz, int iRes, int iCount);
extern cvar_t *sensitivity;
cvar_t *cl_lw = NULL;

void ShutdownInput(void);

int __MsgFunc_ResetHUD(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_ResetHUD(pszName, iSize, pbuf);
}

int __MsgFunc_StatusIcons(const char *pszName, int iSize, void *pbuf); //Drigien MAY2008

int __MsgFunc_InitHUD(const char *pszName, int iSize, void *pbuf)
{
	logfile << Logger::LOG_INFO << "[__MsgFunc_InitHUD]\n";
	gHUD.MsgFunc_InitHUD(pszName, iSize, pbuf);

	return 1;
}

int __MsgFunc_SetFOV(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_SetFOV(pszName, iSize, pbuf);
}

int __MsgFunc_GameMode(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_GameMode(pszName, iSize, pbuf);
}

// TFFree Command Menu
/*void __CmdFunc_OpenCommandMenu(void)
{
	if ( gViewPort )
	{
		gViewPort->ShowCommandMenu( gViewPort->m_StandardMenu );
	}
}

// TFC "special" command
void __CmdFunc_InputPlayerSpecial(void)
{
	if ( gViewPort )
	{
		gViewPort->InputPlayerSpecial();
	}
}

void __CmdFunc_CloseCommandMenu(void)
{
	if ( gViewPort )
	{
		gViewPort->InputSignalHideCommandMenu();
	}
}

void __CmdFunc_ForceCloseCommandMenu( void )
{
	if ( gViewPort )
	{
		gViewPort->HideCommandMenu();
	}
}*/

//Master Sword
//void __CmdFunc_PlayerUseItem(void);
//void __CmdFunc_PlayerDropItem(void);
void __CmdFunc_DebugView(void);
void __CmdFunc_BindTeleport(void);
void __CmdFunc_DynamicNPC(void);
void __CmdFunc_DebugPrint(void);
void __CmdFunc_PlayerDesc(void);
void __CmdFunc_PlayerInfo(void);
void __CmdFunc_PlayerChooseChar(void);
void __CmdFunc_Inv(void);
void __CmdFunc_Slot1(void);
void __CmdFunc_Slot2(void);
void __CmdFunc_Slot3(void);
void __CmdFunc_Slot4(void);
void __CmdFunc_Slot5(void);
void __CmdFunc_Slot6(void);
void __CmdFunc_Slot7(void);
void __CmdFunc_Slot8(void);
void __CmdFunc_Slot9(void);
void __CmdFunc_Slot10(void);
void __CmdFunc_StartSayText();
void __CmdFunc_HUDScroll();
void __CmdFunc_QuickSlot();
void __CmdFunc_QuickSlotStart();
void __CmdFunc_QuickSlotEnd();
void __CmdFunc_ToggleMenu();
void __CmdFunc_SpecialUp();
void __CmdFunc_SpecialDown();

//void __CmdFunc_SpecialPressed(void);
//void __CmdFunc_SpecialReleased(void);
//__MsgFunc_SetProp APR2008a - moved to CLPlayer.cpp - caused compile error, moved back
int __MsgFunc_SetProp(const char *pszName, int iSize, void *pbuf); //MiB Nov/Dec 2007
int __MsgFunc_StoreItem(const char *pszName, int iSize, void *pbuf);
int __MsgFunc_SetStat(const char *pszName, int iSize, void *pbuf);
int __MsgFunc_Vote(const char *pszName, int iSize, void *pbuf);
int __MsgFunc_HUDInfoMsg(const char *pszName, int iSize, void *pbuf);
int __MsgFunc_CharInfo(const char *pszName, int iSize, void *pbuf);
int __MsgFunc_Item(const char *pszName, int iSize, void *pbuf);
int __MsgFunc_HideWeapon(const char *pszName, int iSize, void *pbuf);
int __MsgFunc_Hands(const char *pszName, int iSize, void *pbuf);
int __MsgFunc_Exp(const char *pszName, int iSize, void *pbuf); //Search shuri
int __MsgFunc_CLDllFunc(const char *pszName, int iSize, void *pbuf);
int __MsgFunc_Anim(const char *pszName, int iSize, void *pbuf);		  //MiB DEC2007a //wierdness splayviewanim
int __MsgFunc_CLXPlay(const char *pszName, int iSize, void *pbuf);	  //MAR2012_28 - moving most sounds client side
int __MsgFunc_LocalPanel(const char *pszName, int iSize, void *pbuf); // MiB MAR2015_01 [LOCAL_PANEL] - Message for server to adjust panel
int __MsgFunc_UpdateItem( const char *pszName, int iSize, void *pbuf ); // OCT2016_18 [NETMSG_UPDATEITEM] Thothie Update Item

//------------

// TFFree Command Menu Message Handlers

int __MsgFunc_TeamNames(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_TeamNames(pszName, iSize, pbuf);
	return 0;
}

int __MsgFunc_VGUIMenu(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_VGUIMenu(pszName, iSize, pbuf);
	return 0;
}

int __MsgFunc_MOTD(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_MOTD(pszName, iSize, pbuf);
	return 0;
}

int __MsgFunc_ServerName(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_ServerName(pszName, iSize, pbuf);
	return 0;
}

int __MsgFunc_ScoreInfo(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_ScoreInfo(pszName, iSize, pbuf);
	return 0;
}

int __MsgFunc_TeamScore(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_TeamScore(pszName, iSize, pbuf);
	return 0;
}

int __MsgFunc_TeamInfo(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_TeamInfo(pszName, iSize, pbuf);
	return 0;
}

int __MsgFunc_Spectator(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_Spectator(pszName, iSize, pbuf);
	return 0;
}

int __MsgFunc_AllowSpec(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_AllowSpec(pszName, iSize, pbuf);
	return 0;
}

int __MsgFunc_ViewModel(const char *pszName, int iSize, void *pbuf)
{
	gHUD.MsgFunc_ViewModel(pszName, iSize, pbuf);
	return 1;
}

#define SetMSHUD(ptr, type) \
	ptr = new (type);       \
	strncpy(ptr->Name,  #type, 32 )
	 
// This is called every time the DLL is loaded
void CHud::Init(void)
{
	//MasterSword Initializations:
	//ClientCmd( "r_shadows 1" );
	ClientCmd("cl_himodels 0");

	SetMSHUD(m_Menu, CHudMenu);
	SetMSHUD(m_Fatigue, CHudFatigue);
	SetMSHUD(m_Magic, CHudMagic);
	SetMSHUD(m_Music, CHudMusic);
	SetMSHUD(m_Action, CHudAction);
	SetMSHUD(m_Health, CHudHealth);
	SetMSHUD(m_HUDScript, CHudScript);
	SetMSHUD(m_Misc, CHudMisc);
	SetMSHUD(m_HUDId, CHudID);

	//MS commands and messages
	//HOOK_COMMAND( "use", PlayerUseItem );
	//HOOK_COMMAND( "drop", PlayerDropItem );
	HOOK_COMMAND("+special", SpecialDown);
	HOOK_COMMAND("-special", SpecialUp);
	HOOK_COMMAND("menu", ToggleMenu);
	HOOK_COMMAND("+quickslot", QuickSlotStart);
	HOOK_COMMAND("-quickslot", QuickSlotEnd);
	HOOK_COMMAND("quickslot", QuickSlot);
	HOOK_COMMAND("hud_scroll", HUDScroll);
	HOOK_COMMAND("say_text", StartSayText);
	HOOK_COMMAND("viewtest", DebugView);
	HOOK_COMMAND("listnpcs", DynamicNPC);
	HOOK_COMMAND("debugprint", DebugPrint);
	HOOK_COMMAND("telebind", BindTeleport);
	HOOK_COMMAND("desc", PlayerDesc);
	HOOK_COMMAND("choosechar", PlayerChooseChar);
	HOOK_COMMAND("playerinfo", PlayerInfo);
	HOOK_COMMAND("inventory", Inv);
	HOOK_MESSAGE(SetProp);	   //MiB Nov/Dev2007 - For name and title changes (perhaps more, too)
	HOOK_MESSAGE(StatusIcons); //Drigien MAY2008
	HOOK_MESSAGE(StoreItem);
	HOOK_MESSAGE(SetStat);
	HOOK_MESSAGE(Vote);
	HOOK_MESSAGE(HUDInfoMsg);
	HOOK_MESSAGE(CharInfo);
	HOOK_MESSAGE(Item);
	HOOK_MESSAGE(HideWeapon);
	HOOK_MESSAGE(Hands);
	HOOK_MESSAGE(CLDllFunc);
	HOOK_MESSAGE(Exp);		  //Search shuri
	HOOK_MESSAGE(Anim);		  //MiB DEC2007 //weirdness splayviewanim
	HOOK_MESSAGE(CLXPlay);	  //MiB DEC2007 //weirdness splayviewanim
	HOOK_MESSAGE(LocalPanel); // MiB MAR2015_01 [LOCAL_PANEL] - Hook for message
	HOOK_MESSAGE(ViewModel); //Allow us to change the view model properties.
	// -------------------------

	//HOOK_MESSAGE( Logo );
	HOOK_MESSAGE(ResetHUD);
	HOOK_MESSAGE(GameMode);
	HOOK_MESSAGE(InitHUD);
	HOOK_MESSAGE(SetFOV);
	//HOOK_MESSAGE( Concuss );

	HOOK_MESSAGE(TeamNames);
	HOOK_MESSAGE(MOTD);
	HOOK_MESSAGE(ServerName);
	HOOK_MESSAGE(ScoreInfo);
	HOOK_MESSAGE(TeamScore);
	HOOK_MESSAGE(TeamInfo);

	HOOK_MESSAGE(Spectator);
	HOOK_MESSAGE(AllowSpec);

	// VGUI Menus
	HOOK_MESSAGE(VGUIMenu);

	//m_Sprites.clear();
	std::vector<HudSprite>().swap(m_Sprites);
	//m_HudList.clear();
	std::vector<CHudBase*>().swap(m_HudList);

	m_iLogo = 0;
	m_iFOV = 0;

	CVAR_CREATE("zoom_sensitivity_ratio", "1.2", 0);
	default_fov = CVAR_CREATE("default_fov", "90", 0);
	m_pCvarStealMouse = CVAR_CREATE("hud_capturemouse", "1", FCVAR_ARCHIVE);

	cl_lw = gEngfuncs.pfnGetCvarPointer("cl_lw");

	// In case we get messages before the first update -- time will be valid
	m_flTime = 1.0;

	//m_Ammo.Init();
	m_Train.Init();
	m_Message.Init();
	m_StatusBar.Init();
	m_DeathNotice.Init();
	m_AmmoSecondary.Init();
	m_TextMessage.Init();
	m_StatusIcons.Init();
	m_SayText.Init();															 //Must init before m_Spectator
	m_Spectator.Init();															 //SDK 2.3
	GetClientVoiceMgr()->Init(&g_VoiceStatusHelper, (vgui::Panel **)&gViewPort); //SDK 2.3

	//Master Sword
	m_Health->Init();
	m_Fatigue->Init();
	m_Magic->Init();
	m_Music->Init();
	m_Action->Init();
	m_Menu->Init();
	m_HUDScript->Init();
	m_Misc->Init();
	m_HUDId->Init();

	//Initialize the player and all items
	MSCLGlobals::Initialize();
	//------------

	MsgFunc_ResetHUD(0, 0, NULL);
}

void CHud::Shutdown()
{
	m_Music->Shutdown();
	
	delete m_Menu;
	delete m_Fatigue;
	delete m_Magic;
	delete m_Music;
	delete m_Action;
	delete m_Health;
	delete m_HUDScript;
	delete m_Misc;
	delete m_HUDId;
}

// GetSpriteIndex()
// searches through the sprite list loaded from hud.txt for a name matching SpriteName
// returns an index into the gHUD.m_rghSprites[] array
// returns 0 if sprite not found
int CHud::GetSpriteIndex(const char *SpriteName)
{
	// look through the loaded sprite name list for SpriteName
	for (int i = 0; i < m_Sprites.size() - 1; ++i)
	{
		if (m_Sprites[i].Name == SpriteName)
			return static_cast<int>(i);
	}

	return -1; // invalid sprite
}

// Redraw
// step through the local data,  placing the appropriate graphics & text as appropriate
// returns 1 if they've changed, 0 otherwise
int CHud::Redraw(float flTime, int intermission)
{
	//Print( "Time: %f\n", flTime );
	m_fOldTime = m_flTime; // save time of previous redraw
	m_flTime = flTime;
	m_flTimeDelta = (double)m_flTime - m_fOldTime;
	static float m_flShotTime = 0;

	// Clock was reset, reset delta
	if (m_flTimeDelta < 0)
	{
		m_flTimeDelta = 0;
	}

	// Bring up the scoreboard during intermission
	if (gViewPort)
	{
		if (m_iIntermission && !intermission)
		{
			// Have to do this here so the scoreboard goes away
			m_iIntermission = intermission;
			gViewPort->HideCommandMenu();
			gViewPort->HideScoreBoard();
		}
		else if (!m_iIntermission && intermission)
		{
			gViewPort->HideCommandMenu();
			gViewPort->ShowScoreBoard();

			// Take a screenshot if the client's got the cvar set
			//if ( CVAR_GET_FLOAT( "hud_takesshots" ) != 0 )
			//	m_flShotTime = flTime + 1.0;	// Take a screenshot in a second
		}
	}

	if (m_flShotTime && m_flShotTime < flTime)
	{
		gEngfuncs.pfnClientCmd("snapshot\n");
		m_flShotTime = 0;
	}

	m_iIntermission = intermission;

	// if no redrawing is necessary
	// return 0;

	for (auto hudElement : m_HudList)
	{
		if (!intermission)
		{
			if ((hudElement->m_iFlags & HUD_ACTIVE) != 0 && (m_iHideHUDDisplay & HIDEHUD_ALL) == 0)
				hudElement->Draw(flTime);
		}
		else
		{ // it's an intermission,  so only draw hud elements that are set to draw during intermissions
			if ((hudElement->m_iFlags & HUD_INTERMISSION) != 0)
				hudElement->Draw(flTime);
		}
	}

	// are we in demo mode? do we need to draw the logo in the top corner?
	// if (m_iLogo)
	// {
	// 	int x, y, i;

	// 	if (m_hsprLogo == 0)
	// 		m_hsprLogo = LoadSprite("sprites/%d_logo.spr");

	// 	SPR_Set(m_hsprLogo, 250, 250, 250);

	// 	x = SPR_Width(m_hsprLogo, 0);
	// 	x = ScreenWidth - x;
	// 	y = SPR_Height(m_hsprLogo, 0) / 2;

	// 	// Draw the logo at 20 fps
	// 	int iFrame = (int)(flTime * 20) % MAX_LOGO_FRAMES;
	// 	i = grgLogoFrame[iFrame] - 1;

	// 	SPR_DrawAdditive(i, x, y, NULL);
	// }

	/*
	if ( g_iVisibleMouse )
	{
		void IN_GetMousePos( int *mx, int *my );
		int mx, my;

		IN_GetMousePos( &mx, &my );
		
		if (m_hsprCursor == 0)
		{
			char sz[256];
			 strncpy(sz,  "sprites/cursor.spr", sizeof(sz) );
			m_hsprCursor = SPR_Load( sz );
		}

		SPR_Set(m_hsprCursor, 250, 250, 250 );
		
		// Draw the logo at 20 fps
		SPR_DrawAdditive( 0, mx, my, NULL );
	}
	*/

	//m_Music->Redraw(flTime, intermission);

	return 1;
}

extern int g_iVisibleMouse;
float HUD_GetFOV(void);
extern cvar_t *sensitivity;
void CHud::Think(void)
{
	MSCLGlobals::Think();
	//------------
	int newfov = 0;
	
	for (auto hudElement : m_HudList)
	{
		if ((hudElement->m_iFlags & HUD_ACTIVE) != 0)
		{
			hudElement->Think();
		}
	}	

	newfov = HUD_GetFOV();
	if (newfov == 0)
	{
		m_iFOV = default_fov->value;
	}
	else
	{
		m_iFOV = newfov;
	}

	// the clients fov is actually set in the client data update section of the hud

	// Set a new sensitivity
	if (m_iFOV == default_fov->value)
	{
		// reset to saved sensitivity
		m_flMouseSensitivity = 0;
	}
	else
	{
		// set a new sensitivity that is proportional to the change from the FOV default
		m_flMouseSensitivity = sensitivity->value * ((float)newfov / (float)default_fov->value) * CVAR_GET_FLOAT("zoom_sensitivity_ratio");
	}

	// think about default fov
	if (m_iFOV == 0)
	{ // only let players adjust up in fov,  and only if they are not overriden by something else
		m_iFOV = V_max(default_fov->value, 90);
	}

	m_Music->Think();
}

void CHud::VidInit(void)
{
	m_scrinfo.iSize = sizeof(m_scrinfo);
	GetScreenInfo(&m_scrinfo);

	// ----------
	// Load Sprites
	// ---------
	//	m_hsprFont = LoadSprite("sprites/%d_font.spr");

	m_hsprLogo = 0;
	m_hsprCursor = 0;

	if (ScreenWidth < 640)
		m_iRes = 320;
	else
		m_iRes = 640;

	// Only load this once
	if (m_Sprites.empty())
	{
		// we need to load the hud.txt, and all sprites within
		int spriteCountAllRes;
		client_sprite_t* spriteList = SPR_GetList("sprites/hud.txt", &spriteCountAllRes);

		if (spriteList)
		{
			m_Sprites.reserve(spriteCountAllRes);

			client_sprite_t* p = spriteList;

			for (int j = 0; j < spriteCountAllRes; ++j)
			{
				if (p->iRes == m_iRes)
				{
					HudSprite hudSprite;
					strncpy(hudSprite.Name, p->szName, MAX_SPRITE_NAME_LENGTH);
					strncpy(hudSprite.SpriteName, p->szSprite, 64);
					// We put each sprite in the list into the vector.
					m_Sprites.push_back(hudSprite);
				}

				++p;
			}

			m_Sprites.shrink_to_fit();
		}
	}

	// make sure we actually load each sprite from the list now.
	for (auto& hudSprite : m_Sprites)
	{
		char file[256];
		snprintf(file, 256, "sprites/%s.spr", hudSprite.SpriteName);
		hudSprite.Handle = SPR_Load(file);
		//hudSprite.Handle = SPR_Load(fmt::format("sprites/{}.spr", hudSprite.SpriteName.c_str()).c_str());
	}

	// These no longer exists so just disabled them for now.
	// assumption: number_1, number_2, etc, are all listed and loaded sequentially
	// m_HUD_number_0 = GetSpriteIndex("number_0");

	// const auto& numberRect = m_Sprites[m_HUD_number_0].Rectangle;
	// m_iFontHeight = numberRect.bottom - numberRect.top;
	
	//Master Sword
	m_HUD_numberSML_0 = GetSpriteIndex("smlnum_0");
	m_HUD_char_slashSML = GetSpriteIndex("smlslash");
	m_HUD_char_slash = GetSpriteIndex("charslash");
	m_HUD_char_colon = GetSpriteIndex("charcolon");
	m_HUD_char_a = GetSpriteIndex("char_a");
	m_HUD_char_A = GetSpriteIndex("char_A");

	//Re-initialize the HUD elements
	for (auto hudElement : m_HudList)
	{
		hudElement->VidInit();
	}

	//Reload the Master Sword global sprite/TGA list
	//(only sprites are reloaded, not TGAs)
	MSBitmap::ReloadSprites();
}

//this should get called whenever the client changes levels or servers.
void CHud::ReloadClient()
{
	m_Music->Reload();
}

/*int CHud::MsgFunc_Logo(const char *pszName,  int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	// update Train data
	m_iLogo = READ_BYTE();

	return 1;
}*/

float g_lastFOV = 0.0;

/*
============
COM_FileBase
============
*/
// Extracts the base name of a file (no path, no extension, assumes '/' as path separator)
void COM_FileBase(const char *in, char *out)
{
	int len, start, end;

	len = strlen(in);

	// scan backward for '.'
	end = len - 1;
	while (end && in[end] != '.' && in[end] != '/' && in[end] != '\\')
		end--;

	if (in[end] != '.') // no '.', copy to end
		end = len - 1;
	else
		end--; // Found ',', copy to left of '.'

	// Scan backward for '/'
	start = len - 1;
	while (start >= 0 && in[start] != '/' && in[start] != '\\')
		start--;

	if (in[start] != '/' && in[start] != '\\')
		start = 0;
	else
		start++;

	// Length of new sting
	len = end - start + 1;

	// Copy partial string
	strncpy(out, &in[start], len);
	// Terminate it
	out[len] = 0;
}

/*
=================
HUD_IsGame

=================
*/
int HUD_IsGame(const char *game)
{
	const char *gamedir;
	char gd[1024];

	gamedir = gEngfuncs.pfnGetGameDirectory();
	if (gamedir && gamedir[0])
	{
		COM_FileBase(gamedir, gd);
		if (!_stricmp(gd, game))
			return 1;
	}
	return 0;
}

/*
=====================
HUD_GetFOV

Returns last FOV
=====================
*/
float HUD_GetFOV(void)
{
	if (gEngfuncs.pDemoAPI->IsRecording())
	{
		// Write it
		int i = 0;
		unsigned char buf[100];

		// Active
		*(float *)&buf[i] = g_lastFOV;
		i += sizeof(float);

		Demo_WriteBuffer(TYPE_ZOOM, i, buf);
	}

	if (gEngfuncs.pDemoAPI->IsPlayingback())
	{
		g_lastFOV = g_demozoom;
	}
	return g_lastFOV;
}

float ClFOV = 0; //FOV, in radians
int CHud::MsgFunc_SetFOV(const char *pszName, int iSize, void *pbuf)
{
	startdbg;
	dbg("Begin");

	BEGIN_READ(pbuf, iSize);

	int newfov = READ_BYTE();
	int def_fov = gEngfuncs.pfnGetCvarFloat("default_fov");

	g_lastFOV = newfov;

	if (newfov == 0)
	{
		m_iFOV = def_fov;
	}
	else
	{
		m_iFOV = newfov;
	}

	ClFOV = cosf((m_iFOV / 2.0) * (M_PI / 180));

	// the clients fov is actually set in the client data update section of the hud

	// Set a new sensitivity
	if (m_iFOV == def_fov)
	{
		// reset to saved sensitivity
		m_flMouseSensitivity = 0;
	}
	else
	{
		// set a new sensitivity that is proportional to the change from the FOV default
		m_flMouseSensitivity = sensitivity->value * ((float)newfov / (float)def_fov) * CVAR_GET_FLOAT("zoom_sensitivity_ratio");
	}
	enddbg;

	return 1;
}

void CHud::AddHudElem(CHudBase *phudelem)
{
	if (!phudelem)
		return;

	m_HudList.push_back(phudelem);
}

float CHud::GetSensitivity(void)
{
	return m_flMouseSensitivity;
}

// MiB MAR2015_01 [LOCAL_PANEL] - Read data for local panel
int __MsgFunc_LocalPanel(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int iMode = READ_BYTE();
	CLocalizedPanel *pLocal;

	if (!gViewPort || !gViewPort->m_pLocalizedMenu)
		return 0;

	pLocal = gViewPort->m_pLocalizedMenu;

	switch (iMode)
	{
	case 0: // Reset
	{
		pLocal->Reset();
		break;
	}
	case 1: // Open
	{
		pLocal->SetServerEntString(READ_STRING());
		pLocal->Show();
		break;
	}
	case 2: // Close
	{
		pLocal->Hide();
		break;
	}
	case 3: // Register menu
	{
		pLocal->Reset();
		pLocal->SetPanelTitle(READ_STRING());
		break;
	}
	case 4: // Register button
	{
		msstring sText = READ_STRING();
		bool bEnabled = READ_BYTE() ? true : false;
		bool bCloseOnClick = READ_BYTE() ? true : false;
		int cbType = READ_BYTE();
		msstring sCallBack = READ_STRING();
		pLocal->AddButton(sText, bEnabled, bCloseOnClick, cbType, sCallBack);
		break;
	}
	case 5: // Register paragraphs
	{
		int iType = READ_BYTE();
		msstring src = READ_STRING();
		if (!iType)
			pLocal->ReadParagraphsFromFile(src);
		else if (iType == 1)
			pLocal->ReadParagraphsFromLocalized(src);
		else
			pLocal->AddParagraph(src);
		break;
	}
	// MIB FEB2019_22 [LOCAL_PANEL_IMAGE]
	case 6: // Register image
	{
		msstring						vName = READ_STRING();
		bool							bIsTga = READ_BYTE() == 1;
		int								vFrame = bIsTga ? 0 : READ_BYTE();
		bool							bBorder = READ_BOOL();
		pLocal->AddImage(vName, bIsTga, bBorder, vFrame);
	}
	}
	return 1;
}