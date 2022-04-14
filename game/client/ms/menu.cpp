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
// menu.cpp
//
// generic menu handler
//

//Entity Stuff
#include "../inc_weapondefs.h"
#include "pm_defs.h"
#include "event_api.h"
#include "r_efx.h"
#include "entity_types.h"
#include "clglobal.h"
#include "logger.h"

//HUD stuff
#include "inc_huditem.h"
#include "../vgui_teamfortressviewport.h"

#include "hudmisc.h"
#include "hudmagic.h"
#include "action.h"
#include "menu.h"

#define MAX_MENU_STRING 512
//char m.cMenuText[MAX_MENU_STRING];
char g_szPrelocalisedMenuString[MAX_MENU_STRING];

int KB_ConvertString(char *in, char **ppout);

MS_DECLARE_MESSAGE(m_Menu, ShowMenu);

int CHudMenu ::Init(void)
{
	memset(&m, 0, sizeof(m));

	gHUD.AddHudElem(this);

	HOOK_MESSAGE(ShowMenu);

	InitHUDData();

	return 1;
}

void CHudMenu ::InitHUDData(void)
{
	m_fMenuDisplayed = 0;
	m_bitsValidSlots = 0;
	Reset();
}

void CHudMenu ::Reset(void)
{
	g_szPrelocalisedMenuString[0] = 0;
	m_fWaitingForMore = FALSE;
}

int CHudMenu ::VidInit(void)
{
	return 1;
}

int CHudMenu ::Draw(float flTime)
{
	// check for if menu is set to disappear
	if (m_flShutoffTime > 0)
	{
		if (m_flShutoffTime <= gHUD.m_flTime)
		{ // times up, shutoff
			m_fMenuDisplayed = 0;
			m_iFlags &= ~HUD_ACTIVE;
			return 1;
		}
	}

	// don't draw the menu if the scoreboard is being shown
	if (gViewPort && gViewPort->IsScoreBoardVisible())
		return 1;

	// draw the menu, along the left-hand side of the screen

	// count the number of newlines
	int nlc = 0;
	for (int i = 0; i < MAX_MENU_STRING && m.cMenuText[i] != '\0'; i++)
	{
		if (m.cMenuText[i] == '\n')
			nlc++;
	}

	// center it
	int y = (ScreenHeight / 2) - ((nlc / 2) * 12) - 40; // make sure it is above the say text
	int x = 20;
	int i = 0;

	while (i < MAX_MENU_STRING && m.cMenuText[i] != '\0')
	{
		gHUD.DrawHudString(x, y, 320, m.cMenuText + i, 255, 255, 255);
		y += 12;

		while (i < MAX_MENU_STRING && m.cMenuText[i] != '\0' && m.cMenuText[i] != '\n')
			i++;
		if (m.cMenuText[i] == '\n')
			i++;
	}

	return 1;
}

// selects an item from the menu
void CHudMenu ::SelectMenuItem(int menu_item)
{
	// if menu_item is in a valid slot,  send a menuselect command to the server
	if ((menu_item > 0) && (m_bitsValidSlots & (1 << (menu_item - 1))))
	{
		m_fOfferedNextMenu = false;

		if (m.m_MenuCallback)
			m.m_MenuCallback(menu_item - 1, &m);

		// remove the menu
		if (!m_fOfferedNextMenu)
			HideMyMenu(m.m_MenuType);
	}
}

void Menu_ServerMenuCallback(int idx, struct TCallbackMenu *pcbMenu)
{
	ClientCmd(UTIL_VarArgs("menuselect %d\n", idx));
}
// Message handler for ShowMenu message
// takes four values:
//		short: a bitfield of keys that are valid input
//		char : the duration, in seconds, the menu should stay up. -1 means is stays until something is chosen.
//		byte : a boolean, TRUE if there is more string yet to be received before displaying the menu, FALSE if it's the last string
//		string: menu string to display
// if this message is never received, then scores will simply be the combined totals of the players.
int CHudMenu ::MsgFunc_ShowMenu(const char *pszName, int iSize, void *pbuf)
{
	char *temp = NULL;

	BEGIN_READ(pbuf, iSize);

	m_bitsValidSlots = READ_SHORT();
	int DisplayTime = READ_CHAR();
	int NeedMore = READ_BYTE();

	if (DisplayTime > 0)
		m_flShutoffTime = DisplayTime + gHUD.m_flTime;
	else
		m_flShutoffTime = -1;

	if (m_bitsValidSlots)
	{
		if (!m_fWaitingForMore) // this is the start of a new menu
		{
			strncpy(g_szPrelocalisedMenuString, READ_STRING(), MAX_MENU_STRING);
		}
		else
		{ // append to the current menu string
			strncat(g_szPrelocalisedMenuString, READ_STRING(), MAX_MENU_STRING - strlen(g_szPrelocalisedMenuString));
		}
		g_szPrelocalisedMenuString[MAX_MENU_STRING - 1] = 0; // ensure null termination (strncat/strncpy does not)

		if (!NeedMore)
		{ // we have the whole string, so we can localise it now
			 strncpy(m.cMenuText,  gHUD.m_TextMessage.BufferedLocaliseTextString(g_szPrelocalisedMenuString), sizeof(m.cMenuText) );

			// Swap in characters
			if (KB_ConvertString(m.cMenuText, &temp))
			{
				 strncpy(m.cMenuText,  temp, sizeof(m.cMenuText) );
				free(temp);
			}
			m.m_MenuType = MENU_NORMAL;
		}

		m.m_MenuCallback = Menu_ServerMenuCallback;
		m_fMenuDisplayed = 1;
		m_iFlags |= HUD_ACTIVE;
	}
	else
	{
		m_fMenuDisplayed = 0; // no valid slots means that the menu should be turned off
		m_iFlags &= ~HUD_ACTIVE;
	}

	m_fWaitingForMore = NeedMore;

	return 1;
}
int CHudMenu ::ShowMenu(int bitsValidSlots, const char *pcMenuText, MenuCallback CallBack, MenuType mtMenuType, float ShutOffTime)
{
	//	if( !pcMenuText ) return FALSE;
	m_fOfferedNextMenu = true;
	m_bitsValidSlots = bitsValidSlots;
	 strncpy(m.cMenuText,  pcMenuText, sizeof(m.cMenuText) );
	m.m_MenuCallback = CallBack;
	m.m_MenuType = mtMenuType;
	m_flShutoffTime = ShutOffTime;
	m_fMenuDisplayed = 1;
	m_iFlags |= HUD_ACTIVE;
	return TRUE;
}

//Hides a certain menu only if that menu is open
bool CHudMenu ::HideMyMenu(int mtMenuType)
{
	if (!gHUD.m_Menu->m_fMenuDisplayed ||
		gHUD.m_Menu->m.m_MenuType != mtMenuType)
		return false;

	gHUD.m_Menu->m_fMenuDisplayed = 0;
	gHUD.m_Menu->m_iFlags &= ~HUD_ACTIVE;
	return true;
}
