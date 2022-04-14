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
// hud_redraw.cpp
//

#include <math.h>
#include "hud.h"
#include "cl_util.h"
#include "vgui_teamfortressviewport.h"

//Master Sword
#include "logger.h"
#include "clglobal.h"
#include "sharedutil.h"

#define MAX_LOGO_FRAMES 56

int grgLogoFrame[MAX_LOGO_FRAMES] =
	{
		1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, 13, 13, 13, 13, 12, 11, 10, 9, 8, 14, 15,
		16, 17, 18, 19, 20, 20, 20, 20, 20, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
		29, 29, 29, 29, 29, 28, 27, 26, 25, 24, 30, 31};

extern int g_iVisibleMouse;

float HUD_GetFOV(void);

extern cvar_t *sensitivity;
// Think
void CHud::Think(void)
{
	startdbg;

	dbg("Begin");

	//Master Sword
	dbg("Call MSCLGlobals::Think");
	MSCLGlobals::Think();
	//------------
	int newfov;
	HUDLIST *pList = m_pHudList;

	dbg("Call Think on All HUDs");
	while (pList)
	{
		dbg(msstring("Call Think on HUD: ") + pList->p->Name);

		if (pList->p->m_iFlags & HUD_ACTIVE)
		{
			//Debug - pretty useful too
			/*if( pList->p == (CHudBase *)m_Action ) logfile << "[Action]";
			else if( pList->p == (CHudBase *)&m_Ammo ) logfile << "[Ammo]";
			else if( pList->p == (CHudBase *)m_Fatigue ) logfile << "[Fatigue]";
			else if( pList->p == (CHudBase *)m_Hands ) logfile << "[Hands]";
			else if( pList->p == (CHudBase *)m_Health ) logfile << "[Heath]";
			else if( pList->p == (CHudBase *)m_Magic ) logfile << "[Magic]";
			else if( pList->p == (CHudBase *)m_Menu ) logfile << "[Menu]";
			else if( pList->p == (CHudBase *)m_Misc ) logfile << "[Misc]";
			else if( pList->p == (CHudBase *)m_Music ) logfile << "[Music]";
			else if( pList->p == (CHudBase *)m_HUDScript ) logfile << "[Script]";
			else if( pList->p == (CHudBase *)m_HUDId ) logfile << "[ID]";*/

			pList->p->Think();
			//logfile << "[HUDTHINKDONE]\r\n";
		}
		pList = pList->pNext;
	}

	dbg("FOV Operations");
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
		m_iFOV = max(default_fov->value, 90);
	}

	dbg("End");

	enddbg;
}

// Redraw
// step through the local data,  placing the appropriate graphics & text as appropriate
// returns 1 if they've changed, 0 otherwise
int CHud ::Redraw(float flTime, int intermission)
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

	HUDLIST *pList = m_pHudList;

	while (pList)
	{
		if (!intermission)
		{
			if ((pList->p->m_iFlags & HUD_ACTIVE) && !(m_iHideHUDDisplay & HIDEHUD_ALL))
				pList->p->Draw(flTime);
		}
		else
		{ // it's an intermission,  so only draw hud elements that are set to draw during intermissions
			if (pList->p->m_iFlags & HUD_INTERMISSION)
				pList->p->Draw(flTime);
		}

		pList = pList->pNext;
	}

	// are we in demo mode? do we need to draw the logo in the top corner?
	if (m_iLogo)
	{
		int x, y, i;

		if (m_hsprLogo == 0)
			m_hsprLogo = LoadSprite("sprites/%d_logo.spr");

		SPR_Set(m_hsprLogo, 250, 250, 250);

		x = SPR_Width(m_hsprLogo, 0);
		x = ScreenWidth - x;
		y = SPR_Height(m_hsprLogo, 0) / 2;

		// Draw the logo at 20 fps
		int iFrame = (int)(flTime * 20) % MAX_LOGO_FRAMES;
		i = grgLogoFrame[iFrame] - 1;

		SPR_DrawAdditive(i, x, y, NULL);
	}

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

	return 1;
}

void ScaleColors(int &r, int &g, int &b, int a)
{
	float x = (float)a / 255;
	r = (int)(r * x);
	g = (int)(g * x);
	b = (int)(b * x);
}

int CHud ::DrawHudString(int xpos, int ypos, int iMaxX, char *szIt, int r, int g, int b)
{
	// draw the string until we hit the null character or a newline character
	for (; *szIt != 0 && *szIt != '\n'; szIt++)
	{
		int next = xpos + gHUD.m_scrinfo.charWidths[*szIt]; // variable-width fonts look cool
		if (next > iMaxX)
			return xpos;

		TextMessageDrawChar(xpos, ypos, *szIt, r, g, b);
		xpos = next;
	}

	return xpos;
}

int CHud ::DrawHudNumberString(int xpos, int ypos, int iMinX, int iNumber, int r, int g, int b)
{
	char szString[32];
	 _snprintf(szString, sizeof(szString),  "%d",  iNumber );
	return DrawHudStringReverse(xpos, ypos, iMinX, szString, r, g, b);
}

// draws a string from right to left (right-aligned)
int CHud ::DrawHudStringReverse(int xpos, int ypos, int iMinX, char *szString, int r, int g, int b)
{
	char *szIt = NULL;

	// find the end of the string
	for (szIt = szString; *szIt != 0; szIt++)
	{ // we should count the length?
	}

	// iterate throug the string in reverse
	for (szIt--; szIt != (szString - 1); szIt--)
	{
		int next = xpos - gHUD.m_scrinfo.charWidths[*szIt]; // variable-width fonts look cool
		if (next < iMinX)
			return xpos;
		xpos = next;

		TextMessageDrawChar(xpos, ypos, *szIt, r, g, b);
	}

	return xpos;
}

int CHud ::DrawHudNumber(int x, int y, int iFlags, int iNumber, int r, int g, int b)
{
	int iWidth = GetSpriteRect(m_HUD_number_0).right - GetSpriteRect(m_HUD_number_0).left;
	int k;

	if (iNumber > 0)
	{
		// SPR_Draw 100's
		if (iNumber >= 100)
		{
			k = iNumber / 100;
			SPR_Set(GetSprite(m_HUD_number_0 + k), r, g, b);
			SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HUD_number_0 + k));
			x += iWidth;
		}
		else if (iFlags & (DHN_3DIGITS))
		{
			//SPR_DrawAdditive( 0, x, y, &rc );
			x += iWidth;
		}

		// SPR_Draw 10's
		if (iNumber >= 10)
		{
			k = (iNumber % 100) / 10;
			SPR_Set(GetSprite(m_HUD_number_0 + k), r, g, b);
			SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HUD_number_0 + k));
			x += iWidth;
		}
		else if (iFlags & (DHN_3DIGITS | DHN_2DIGITS))
		{
			//SPR_DrawAdditive( 0, x, y, &rc );
			x += iWidth;
		}

		// SPR_Draw ones
		k = iNumber % 10;
		SPR_Set(GetSprite(m_HUD_number_0 + k), r, g, b);
		SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HUD_number_0 + k));
		x += iWidth;
	}
	else if (iFlags & DHN_DRAWZERO)
	{
		SPR_Set(GetSprite(m_HUD_number_0), r, g, b);

		// SPR_Draw 100's
		if (iFlags & (DHN_3DIGITS))
		{
			//SPR_DrawAdditive( 0, x, y, &rc );
			x += iWidth;
		}

		if (iFlags & (DHN_3DIGITS | DHN_2DIGITS))
		{
			//SPR_DrawAdditive( 0, x, y, &rc );
			x += iWidth;
		}

		// SPR_Draw ones

		SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HUD_number_0));
		x += iWidth;
	}

	return x;
}
//Master Sword
int CHud ::DrawHudNumberSML(int x, int y, int iFlags, int iNumber, int r, int g, int b)
{
	int iWidth;
	int k, idigits, itemp, ix = x;

	idigits = numofdigits(iNumber);
	if (idigits >= 256)
		ConsolePrint("ERROR: numofdigits( )\n");
	/*		if( iNumber < 3 ) { 
		char a[50];
		 _snprintf(a, sizeof(a),  "[%i][%i](%i)[%i] \n",  idigits,  power10(idigits),  iNumber,  iMaxMP );
		ConsolePrint( a );
	}*/
	itemp = idigits;
	while (itemp)
	{
		if (itemp == idigits)
			k = iNumber / pow(10.0, itemp - 1);
		else if (itemp == 1)
			k = iNumber % 10;
		else
			k = (iNumber % (int)pow(10.0, itemp)) / pow(10.0, itemp - 1);
		k = min(k, 9);
		iWidth = GetSpriteRect(m_HUD_numberSML_0 + k).right - GetSpriteRect(m_HUD_numberSML_0 + k).left;
		SPR_Set(GetSprite(m_HUD_numberSML_0 + k), r, g, b);
		SPR_DrawAdditive(0, ix, y, &GetSpriteRect(m_HUD_numberSML_0 + k));
		ix += iWidth;
		itemp--;
	}
	return ix;
}
int CHud ::DrawHudStringSML(int x, int y, char *pcString, int r, int g, int b, int iExtraSpace)
{
	//char a[50];
	int i = 0, iStringSize = strlen(pcString), ix = x, iWidth, iChar;
	while (i < iStringSize)
	{
		if (pcString[i] == '/')
		{
			iWidth = GetSpriteRect(m_HUD_char_slashSML).right - GetSpriteRect(m_HUD_char_slashSML).left;
			SPR_Set(GetSprite(m_HUD_char_slashSML), r, g, b);
			SPR_DrawAdditive(0, ix, y, &GetSpriteRect(m_HUD_char_slashSML));
			ix += iWidth;
		}
		else if (pcString[i] == ':')
		{
			iWidth = GetSpriteRect(m_HUD_char_colon).right - GetSpriteRect(m_HUD_char_colon).left;
			SPR_Set(GetSprite(m_HUD_char_colon), r, g, b);
			SPR_DrawAdditive(0, ix, y, &GetSpriteRect(m_HUD_char_colon));
			ix += iWidth;
		}
		else if (pcString[i] >= '0' && pcString[i] <= '9')
		{
			ix = DrawHudNumberSML(ix, y + 3, NULL, pcString[i] - '0', r, g, b);
		}
		else if (pcString[i] == ' ')
			ix += 9;
		else
		{
			iChar = pcString[i] - 'A';
			if (iChar >= 0 && iChar < 26)
			{
				iWidth = GetSpriteRect(m_HUD_char_A + iChar).right - GetSpriteRect(m_HUD_char_A + iChar).left;
				SPR_Set(GetSprite(m_HUD_char_A + iChar), r, g, b);
				SPR_DrawAdditive(0, ix, y, &GetSpriteRect(m_HUD_char_A + iChar));
				ix += iWidth + iExtraSpace;
			}
			else if (iChar >= 32 && iChar < 60)
			{
				iChar -= (26 + 6); //The extra 6 because there are 6 chars between Z and a
				iWidth = GetSpriteRect(m_HUD_char_a + iChar).right - GetSpriteRect(m_HUD_char_a + iChar).left;
				SPR_Set(GetSprite(m_HUD_char_a + iChar), r, g, b);
				SPR_DrawAdditive(0, ix, y, &GetSpriteRect(m_HUD_char_a + iChar));
				ix += iWidth + iExtraSpace;
			}
			else
				ix += 10;
		}
		i++;
	}
	return ix;
}

int CHud::GetNumWidth(int iNumber, int iFlags)
{
	if (iFlags & (DHN_3DIGITS))
		return 3;

	if (iFlags & (DHN_2DIGITS))
		return 2;

	if (iNumber <= 0)
	{
		if (iFlags & (DHN_DRAWZERO))
			return 1;
		else
			return 0;
	}

	if (iNumber < 10)
		return 1;

	if (iNumber < 100)
		return 2;

	return 3;
}
