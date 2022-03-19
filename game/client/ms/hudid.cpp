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
// HUDId.cpp - Displays info about the player, monster, or item being looked at
//
#include "inc_huditem.h"

//Player functionality
#include "../inc_weapondefs.h"
//#include "monsters/Script.h"
#include "pm_defs.h"
#include "event_api.h"
#include "r_efx.h"

extern physent_t *MSUTIL_EntityByIndex(int playerindex);
extern void HUD_PrepEntity(CBaseEntity *pEntity, CBasePlayer *pWeaponOwner);
//----------------

#include "hudid.h"
#include "vgui_hud.h"
#include "logfile.h"

MS_DECLARE_MESSAGE(m_HUDId, EntInfo);

int CHudID::Init(void)
{
	Reset();

	HOOK_MESSAGE(EntInfo);

	gHUD.AddHudElem(this);

	return 1;
}

void CHudID::Reset(void)
{
	m_iFlags |= HUD_ACTIVE;
	TimeDecAlpha = 0;
	Alpha = 0;
	pActiveInfo = pDrawInfo = NULL;
}
void CHudID ::InitHUDData(void)
{
	Reset();
	player.m_EntInfo.clear();
}

int CHudID::Draw(float flTime)
{
	startdbg;
	dbg("Begin");

	if (!FBitSet(m_iFlags, HUD_ACTIVE) || !ShowHUD())
		return 1;
	if (player.m_CharacterState == CHARSTATE_UNLOADED)
		return 1;

	dbg("Call SearchThink");
	SearchThink();
	//if( Alpha <= 0 || !pDrawInfo || !pDrawInfo->Name.len() ) return 1;

	HUD_DrawID(pDrawInfo);
	enddbg;

	return 1;

	/*if( !pActiveInfo && flTime > TimeDecAlpha ) {
		Alpha -= 10;
		TimeDecAlpha = flTime + 0.1;
	}

	int TextHeight, TextWidth;
	dbg( "Call GetConsoleStringSize" );
	GetConsoleStringSize( pDrawInfo->Name, &TextWidth, &TextHeight );

	int Y_START;
	if ( ScreenHeight >= 480 )
		Y_START = ScreenHeight - 128;
	else
		Y_START = ScreenHeight - 64;

	int x = 5;
	int y = Y_START - TextHeight; // draw along bottom of screen
	dbg( "Draw Strings" );

	msstring String = pDrawInfo->Name + "\n";
	CharUpperBuff( String, 1 );
	DrawConsoleString( x, y, String );

	if( pDrawInfo->Type == ENT_FRIENDLY ) {
		String = "Friendly";
		gEngfuncs.pfnDrawSetTextColor( 0, 255, 0 );
	}
	else if( pDrawInfo->Type == ENT_WARY ) {
		String = "Wary";
		gEngfuncs.pfnDrawSetTextColor( 255, 255, 0 );
	}
	else if( pDrawInfo->Type == ENT_HOSTILE ) {
		String = "Hostile";
		gEngfuncs.pfnDrawSetTextColor( 255, 0, 0 );
	}
	else if( pDrawInfo->Type == ENT_DEADLY ) {
		String = "Deadly";
		gEngfuncs.pfnDrawSetTextColor( 255, 0, 0 );
	}
	else String = "";

	DrawConsoleString( x, y + TextHeight, String );

	enddbg( "CHudID::Draw()" );
	return 1; */
}

void CHudID::SearchThink()
{
	//This code uses EV_PlayerTrace( ).
	//EV_PlayerTrace() fails when you try to call it while changing levels and
	//Think() is called while changing levels, so this code is now called from Draw()
	//instead of Think();

	pActiveInfo = gHUD.m_HUDId->GetEntInFrontOfMe(4096);
	//if( pActiveInfo )
	//{
	//	Alpha = 255;
	pDrawInfo = pActiveInfo;
	//}
}

// Message handler for EntInfo message
// accepts 3 values:
//		short  : 0 = Engine index of entity
//		string : Full name of entity (i.e "An Iron Shield")
//		byte   : Relative entity alignment (Neutral, Friendly, Hostile)
int CHudID::MsgFunc_EntInfo(const char *pszName, int iSize, void *pbuf)
{
	startdbg;
	dbg("Begin");
	BEGIN_READ(pbuf, iSize);

	entinfo_t EntData;

	EntData.entindex = READ_SHORT();
	EntData.Name = READ_STRING();
	EntData.Type = (EntType)READ_BYTE();

	//Search for a current entry with this info
	for (int i = 0; i < player.m_EntInfo.size(); i++)
		if (player.m_EntInfo[i].entindex == EntData.entindex)
		{
			player.m_EntInfo[i] = EntData;
			return 1;
		}

	//Not found, create a new one
	player.m_EntInfo.add(EntData);

	enddbg;
	return 1;
}

entinfo_t *CHudID::GetEntInFrontOfMe(float Range)
{
	pmtrace_t tr;
	Vector vForward;
	Vector vViewAngle;
	gEngfuncs.GetViewAngles(vViewAngle);
	cl_entity_s *clplayer = gEngfuncs.GetLocalPlayer();
	AngleVectors(vViewAngle, vForward, NULL, NULL);
	Vector vecSrc = clplayer->origin, vecEnd,
		   viewOfs;
	gEngfuncs.pEventAPI->EV_LocalPlayerViewheight(viewOfs);
	vecSrc = vecSrc + viewOfs;
	vecEnd = vecSrc + vForward * Range; //RANGE: 72

	gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction(false, false);

	physent_t *pPhyplayer = gEngfuncs.pEventAPI->EV_GetPhysent(clplayer->index);
	if (!pPhyplayer)
		return NULL;

	// Now add in all of the players.
	gEngfuncs.pEventAPI->EV_SetSolidPlayers(clplayer->index - 1);

	gEngfuncs.pEventAPI->EV_SetTraceHull(2);
	gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, PM_STUDIO_BOX, pPhyplayer->info, &tr);

	if (tr.fraction < 1.0 && tr.ent)
	{
		physent_t *pe = gEngfuncs.pEventAPI->EV_GetPhysent(tr.ent);
		for (int i = 0; i < player.m_EntInfo.size(); i++)
			if (player.m_EntInfo[i].entindex == pe->info)
				return &player.m_EntInfo[i];
	}

	return NULL;
}
