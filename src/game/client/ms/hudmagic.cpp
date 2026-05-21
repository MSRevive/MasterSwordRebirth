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
// Magic.cpp - Allow client-side choosing of magic spells
//
#include "inc_weapondefs.h"
#include "inc_huditem.h"
#include "menu.h"
#include "stats/stats.h"
#include "hudmagic.h"
#include "magic.h"

void CHudMagic_SelectMenuItem(int menu_item, TCallbackMenu *pcbMenu);

MS_DECLARE_MESSAGE(m_Magic, Spells);


int CHudMagic::Init(void)
{
	gHUD.AddHudElem(this);

	HOOK_MESSAGE(Spells);
	Reset();

	return 1;
}

int CHudMagic::Draw(float flTime) { return 1; }
void CHudMagic::InitHUDData(void)
{
}

int CHudMagic::VidInit(void)
{

	return 1;
}

void CHudMagic::Reset(void)
{
	m_iFlags |= HUD_ACTIVE;
}

// Think about ?
void CHudMagic::Think()
{
}

// Message handler for Magic message (MiB JAN2010_15)
int CHudMagic::MsgFunc_Spells(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	bool Verbose = (READ_BYTE() == 1);
	player.LearnSpell(READ_STRING(), Verbose);

	return 1;
}

void CHudMagic_SelectMenuItem(int idx, TCallbackMenu *pcbMenu)
{
	if (!gHUD.m_Magic)
		return;

	int iSpell = gHUD.m_Magic->SpellMenuIndex[idx];

	char szString[32];

	 _snprintf(szString, sizeof(szString),  "prep %s\n",  player.m_SpellList[iSpell].c_str() );
	ClientCmd(szString);
}
int CHudMagic::SpellsMemorized(void) { return player.m_SpellList.size(); }
