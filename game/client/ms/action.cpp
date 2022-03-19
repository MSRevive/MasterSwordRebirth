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
// Action.cpp - Allow client-side choosing of special actions
//
#include "inc_huditem.h"
#include "menu.h"
#include "string.h"
#include "msdllheaders.h"
#include "scriptedeffects.h"
#include "action.h"
#include "logfile.h"

void CHudAction_SelectMenuItem(int idx, TCallbackMenu *pcbMenu);

MS_DECLARE_COMMAND(m_Action, Action);

int CHudAction::Init(void)
{
	gHUD.AddHudElem(this);

	HOOK_COMMAND("act", Action);

	m_iFlags = HUD_ACTIVE;

	return 1;
}

int CHudAction::MsgFunc_Action(const char *pszName, int iSize, void *pbuf)
{
	startdbg;
	dbg("Begin");

	byte AddOrRemove = READ_BYTE();
	msstring ID = READ_STRING();

	if (AddOrRemove) //Add
	{
		playeraction_t *ModifyAction = NULL;
		playeraction_t NewAction;
		NewAction.Name = READ_STRING();
		NewAction.ID = ID;

		for (int i = 0; i < PlayerActions.size(); i++)
			if (PlayerActions[i].ID == NewAction.ID)
			{
				PlayerActions[i] = NewAction;
				return 1;
			}

		PlayerActions.add(NewAction);
	}
	else //Remove
	{
		for (int i = 0; i < PlayerActions.size(); i++)
			if (PlayerActions[i].ID == ID)
			{
				PlayerActions.erase(i);
				break;
			}
	}

	enddbg;
	return 1;
}
void CHudAction::UserCmd_Action(void)
{

	//Print("DEBUG: UserCmd_Action: entered\n");

	if (gHUD.m_Menu->m_fMenuDisplayed && gHUD.m_Menu->m.m_MenuType == MENU_ACTIONS) //Menu is already on, turn it off
	{
		gHUD.m_Menu->HideMyMenu(gHUD.m_Menu->m.m_MenuType);
		return;
	}

	//Print("DEBUG: UserCmd_Action: display\n");

	if (!FBitSet(m_iFlags, HUD_ACTIVE))
		return;

	//Print("DEBUG: UserCmd_Action: hud active - #actions: %i\n",PlayerActions.size());

	int iBitsValid = 0;
	msstring MenuText(msstring(Localized("#ACTION_QUERY")) + "\n\n");
	for (int i = 0; i < PlayerActions.size(); i++)
	{
		//Print("DEBUG: UserCmd_Action: loop: %i %s %s\n",i,PlayerActions[i].Name.c_str(),PlayerActions[i].ID.c_str());
		MenuText += (i + 1);
		MenuText += ". ";
		MenuText += Localized(PlayerActions[i].Name);
		MenuText += "\n";
		iBitsValid += (1 << i);
	}

	//Activate
	//Print("DEBUG: UserCmd_Action: Activate %i %s %s\n",iBitsValid, MenuText.c_str(),PlayerActions[0].Name);
	if (iBitsValid)
		gHUD.m_Menu->ShowMenu(iBitsValid, MenuText, CHudAction_SelectMenuItem, MENU_ACTIONS);
}
void CHudAction_SelectMenuItem(int idx, TCallbackMenu *pcbMenu) //menu_item starts at 1
{
	Print("DEBUG: CHudAction_SelectMenuItem\n");
	if (!gHUD.m_Action)
		return;

	if (idx >= (signed)gHUD.m_Action->PlayerActions.size())
		return;

	playeraction_t &Action = gHUD.m_Action->PlayerActions[idx];

	Print("DEBUG: Send Action\n");

	msstring SendCmd("action ");
	SendCmd += Action.ID;
	SendCmd += "\n";

	ClientCmd(SendCmd);
}
