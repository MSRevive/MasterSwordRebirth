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
// Hands.cpp
//
// implementation of CHudMisc class - Show your currently held items
//

//Entity Stuff
#include "inc_weapondefs.h"
#include "pm_defs.h"
#include "event_api.h"
#include "r_efx.h"
#include "entity_types.h"
#include "clglobal.h"

#include "inc_huditem.h"

#include "hudmisc.h"
#include "hudid.h"
#include "menu.h"
#include "logger.h"

void VectorAngles(const float *forward, float *angles);
void CHudMisc_SelectMenuItem(int idx, TCallbackMenu *pcbMenu)
{
	gHUD.m_Misc->SelectMenuItem(idx, pcbMenu);
}
//MS_DECLARE_MESSAGE( m_Misc, Arrow )
MS_DECLARE_COMMAND(m_Misc, ChangeSayType);
MS_DECLARE_COMMAND(m_Misc, RemovePack);
MS_DECLARE_COMMAND(m_Misc, Offer);
MS_DECLARE_COMMAND(m_Misc, Accept);
//MS_DECLARE_COMMAND( m_Misc, ListSkills );
//

int CHudMisc::Init(void)
{
	//HOOK_MESSAGE(Arrow);
	HOOK_COMMAND("setsay", ChangeSayType);
	//HOOK_COMMAND("remove", RemovePack);
	//HOOK_COMMAND("offer", Offer);
	HOOK_COMMAND("accept", Accept);
	//HOOK_COMMAND("listskills", ListSkills);

	gHUD.AddHudElem(this);

	m_iFlags |= HUD_ACTIVE;
	return 1;
}

void CHudMisc::Reset(void) {}

void CHudMisc ::InitHUDData(void)
{
	clrmem(m_OfferInfo);
	player.m_SayType = SPEECH_GLOBAL;
}

bool ScriptRecvRead();
int CHudMisc::Draw(float flTime)
{
	/*if ( (gHUD.m_iHideHUDDisplay&HIDEHUD_ALL) || !(m_iFlags&HUD_ACTIVE) ) return 1;

	int SpriteIndex;
	switch( player.m_SayType )
	{
		case SPEECH_GLOBAL: SpriteIndex = gHUD.GetSpriteIndex("hud_shout"); break;
		case SPEECH_LOCAL: SpriteIndex = gHUD.GetSpriteIndex("hud_talk"); break;
		case SPEECH_PARTY: SpriteIndex = gHUD.GetSpriteIndex("hud_party"); break;
	}

	SPR_Set(gHUD.GetSprite(SpriteIndex), 255, 255, 255 );
	SPR_DrawAdditive( 0, ScreenWidth - 70, ScreenHeight - 70, &gHUD.GetSpriteRect(SpriteIndex));*/

	return 1;
}
void CHudMisc::Think(void)
{
	startdbg;
	//SaveCharSend(); //Done every frame so I can retry the connect

	//Handle character upload to server
	dbg("Think_SendCharData");
	MSChar_Interface::Think_SendChar(&player);

	enddbg;
}

void CHudMisc ::UserCmd_ChangeSayType(void)
{
	player.m_SayType++;
	if (player.m_SayType > SPEECH_PARTY)
		player.m_SayType = SPEECH_GLOBAL;

	msstring SayString = "You ";
	switch (player.m_SayType)
	{
	case SPEECH_GLOBAL:
		SayString += "begin to shout!\n";
		break;
	case SPEECH_PARTY:
		SayString += "aim your voice toward your party.\n";
		break;
	default:
		SayString += "speak normally.\n";
		break;
	}

	if (player.m_CharacterState == CHARSTATE_LOADED)
		player.SendEventMsg(HUDEVENT_NORMAL, SayString);

	SayString = msstring("setsay ") + player.m_SayType + "\n";
	ServerCmd(SayString);
}

void CHudMisc ::UserCmd_RemovePack(void)
{
	startdbg;
	dbg("Begin");

	//Menu is already on, turn it off
	if (gHUD.m_Menu->HideMyMenu(MENU_REMOVEPACK))
		return;
	if (gHUD.m_iHideHUDDisplay & HIDEHUD_ALL)
		return;

	//Disallow removing items by unsetting HIDEHUD_ALL
	if (gHUD.m_iHideHUDDisplay & HIDEHUD_ALL)
		return;

	int iBitsValid = 0;
	char MenuText[1024];

	int test = *(int *)&m_RemoveList;
	int test2 = m_RemoveList.size();

	 strncpy(MenuText,  "Remove item:\n\n", sizeof(MenuText) );

	m_RemoveList.clear();
	for (int i = 0; i < player.Gear.size(); i++)
	{
		CGenericItem *pGearItem = player.Gear[i];
		if (!FBitSet(pGearItem->MSProperties(), ITEM_WEARABLE) || pGearItem->m_Location == ITEMPOS_HANDS)
			continue;

		msstring ItemName;
		ItemName += SPEECH_GetItemName(pGearItem);
		toupper(ItemName[0]);

		msstring SendString;
		int Size = m_RemoveList.size();
		SendString += Size + 1;
		SendString += ". ";
		SendString += (ItemName + "\n").c_str();
		strncat(MenuText, SendString, SendString.len());
		iBitsValid += pow(2.0, Size);

		m_RemoveList.add(pGearItem->m_iId);
	}

	if (m_RemoveList.size())
		gHUD.m_Menu->ShowMenu(iBitsValid, MenuText, CHudMisc_SelectMenuItem, MENU_REMOVEPACK);
	else
		player.SendEventMsg(HUDEVENT_UNABLE, "You are not wearing anything\n");

	enddbg;
}

void CHudMisc::SelectMenuItem(int idx, TCallbackMenu *pcbMenu)
{
	char szString[32];
	int SelectedItem = idx;
	switch (pcbMenu->m_MenuType)
	{
	case MENU_REMOVEPACK:
		 _snprintf(szString, sizeof(szString),  "remove %u\n",  m_RemoveList[SelectedItem] );
		ServerCmd(szString);
		break;
	case MENU_OFFER:
	{
		if (!m_OfferInfo.GoldScreen)
		{
			if (SelectedItem >= 1 && SelectedItem < MAX_PLAYER_HANDS + 1)
			{
				int iHand = m_OfferInfo.OfferItem[SelectedItem - 1];

				CGenericItem *pItem = player.Hand(iHand);
				if (!pItem)
					break;

				player.SendInfoMsg("You offer %s %s\n", m_OfferTarget.Name.c_str(), SPEECH::ItemName(pItem));
				 _snprintf(szString, sizeof(szString),  "offer %i %i\n",  m_OfferTarget.entindex,  iHand );
				ServerCmd(szString);
				break;
			}
			else
			{
				m_OfferInfo.GoldScreen = true;
				m_OfferInfo.GoldAmt = 0;
			}
		}
		else
		{
			if (m_OfferInfo.GoldAmt < MAX_GOLD_OFFER)
			{
				int iAdd = (idx == 9) ? 0 : idx + 1;
				m_OfferInfo.GoldAmt = m_OfferInfo.GoldAmt * 10 + iAdd;
			}
		}

		if (m_OfferInfo.GoldScreen)
			gHUD.m_Menu->ShowMenu(pow(2.0, 10) - 1, UTIL_VarArgs("You have %i gold coins.\n\nOffer gold: %i\n", player.m_Gold, m_OfferInfo.GoldAmt), CHudMisc_SelectMenuItem, MENU_OFFER);

		break;
	}
		/*	case MENU_LISTSKILLS:
		 _snprintf(szString, sizeof(szString),  "useskill %i\n",  SelectedItem );
		ServerCmd( szString );
		break;*/
	}
}

void CHudMisc ::UserCmd_Offer(void)
{
	startdbg;
	dbg("Begin");

	//Menu is already on, turn it off
	if (gHUD.m_Menu->HideMyMenu(MENU_OFFER))
		return;
	if (gHUD.m_iHideHUDDisplay & HIDEHUD_ALL)
		return;

	entinfo_t *pEntInfo = gHUD.m_HUDId->GetEntInFrontOfMe(72); //Look 72 units ahead
	if (!pEntInfo)
		return;

	entinfo_t &EntInfo = *pEntInfo;
	if (EntInfo.Type > ENT_FRIENDLY)
		return;

	m_OfferTarget = EntInfo;

	int iBitsValid = 0;
	char MenuText[1024];
	clrmem(m_OfferInfo);
	//bool fOfferedSomething = false;

	 _snprintf(MenuText, sizeof(MenuText),  "Offer to %s:\n",  pEntInfo->Name.c_str() );

	//if( player.m_Gold )
	{
		strncat(MenuText, "\n\n1. Gold\n\n", 15);
		iBitsValid |= (1 << 0);
		//fOfferedSomething = true;
	}

	int r = 0;
	for (int i = 0; i < MAX_PLAYER_HANDS; i++)
	{
		if (!player.Hand(i))
			continue;

		const char *arg = UTIL_VarArgs("%i. %s hand: %s\n", r + 2, SPEECH_IntToHand(i, true), SPEECH::ItemName(player.Hand(i), true));
		strncat(MenuText, arg, strlen(arg));
		m_OfferInfo.OfferItem[r] = i;
		iBitsValid |= (1 << (++r)); //Starts at 1
									//fOfferedSomething = true;
	}

	/*if( !fOfferedSomething )
	{
		player.SendInfoMsg( "You have nothing to offer %s\n", pEntInfo->Name );
		return;
	}*/

	strncat(MenuText, "\n(Press 'offer' again to cancel)\n", 35);
	gHUD.m_Menu->ShowMenu(iBitsValid, MenuText, CHudMisc_SelectMenuItem, MENU_OFFER);
	enddbg;
}

void CHudMisc ::UserCmd_Accept(void)
{
	startdbg;
	dbg("Begin");

	//Override 'accept' for when entering gold amounts
	char sz[128];
	if (m_OfferInfo.GoldScreen)
	{
		gHUD.m_Menu->HideMyMenu(MENU_OFFER);

		m_OfferInfo.GoldScreen = false;
		if (player.m_Gold < m_OfferInfo.GoldAmt)
		{
			player.SendInfoMsg("You don't have %i gold coins!\n", m_OfferInfo.GoldAmt);
			return;
		}
		player.SendInfoMsg("You offer %s %i gold coins\n", m_OfferTarget.Name.c_str(), m_OfferInfo.GoldAmt);
		 _snprintf(sz, sizeof(sz),  "offer %i gold %i\n",  m_OfferTarget.entindex,  m_OfferInfo.GoldAmt );
		ServerCmd(sz);
		return;
	}

	ServerCmd("accept\n");
	enddbg;
}
/*void CHudMisc :: UserCmd_ListSkills( void )
{
	//Menu is already on, turn it off
//	if( gHUD.m_Menu->HideMyMenu( MENU_LISTSKILLS ) ) return;

	//if( gHUD.m_iHideHUDDisplay&HIDEHUD_ALL ) return;

	//if( player.Class && player.Class->id == CLASS_ROGUE )
	//	gHUD.m_Menu->ShowMenu( 1, "Use skill:\n\n1. Shadow Stealth\n", MENU_LISTSKILLS );
}*/
