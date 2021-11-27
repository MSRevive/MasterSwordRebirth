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
//  hud_msg.cpp
//

#include "MSDLLHeaders.h"
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "string.h"
#include "logfile.h"
#include "MasterSword/MSNetcodeClient.h"
#include "../MSShared/MSCharacter.h"
#include "MasterSword/CLGlobal.h"
#include "MasterSword/CLEnv.h"
#include "MasterSword/vgui_ChooseCharacter.h"
#include "Monsters/MSMonster.h"

int CHud ::MsgFunc_ResetHUD(const char *pszName, int iSize, void *pbuf)
{
	//ASSERT( iSize == 0 );

	// clear all hud data
	HUDLIST *pList = m_pHudList;

	while (pList)
	{
		if (pList->p)
			pList->p->Reset();
		pList = pList->pNext;
	}

	// reset sensitivity
	m_flMouseSensitivity = 0;

	// reset concussion effect
	m_iConcussionEffect = 0;

	return 1;
}

#define CLPERMENT_TOTALK 3

void CHud ::MsgFunc_InitHUD(const char *pszName, int iSize, void *pbuf)
{
	startdbg;
	dbg("Read InitHUD msg");

	logfile << "[MsgFunc_InitHUD: EndMap]"
			<< "\r\n";
	MSGlobals::EndMap(); //End old map

	//Copy over the mapname here because the engine doesn't send it
	//interally until later
	logfile << "[MsgFunc_InitHUD: Globals]"
			<< "\r\n";
	BEGIN_READ(pbuf, iSize);
	MSGlobals::ServerName = READ_STRING();
	MSGlobals::MapName = READ_STRING();
	//g_NetCode.m.HostIP = READ_STRING();
	logfile << "[MsgFunc_InitHUD: CLEnt Readin]"
			<< "\r\n";
	for (int i = 0; i < CLPERMENT_TOTALK; i++)
		MSGlobals::ClEntities[i] = READ_SHORT();
	int flags = READ_BYTE();
	logfile << "[MsgFunc_InitHUD: SetupDefGlobals]"
			<< "\r\n";
	MSCLGlobals::OnMyOwnListenServer = (flags & (1 << 0)) ? true : false;
	MSGlobals::IsLanGame = (flags & (1 << 1)) ? true : false;
	MSGlobals::CanCreateCharOnMap = (flags & (1 << 2)) ? true : false;
	MSGlobals::GameType = (flags & (1 << 3)) ? GAMETYPE_ADVENTURE : GAMETYPE_CHALLENGE;
	MSGlobals::ServerSideChar = (flags & (1 << 4)) ? true : false;
	MSCLGlobals::OtherPlayers = (flags & (1 << 5)) ? true : false;

	logfile << "[MsgFunc_InitHUD: AuthID]"
			<< "\r\n";
	MSCLGlobals::AuthID = READ_STRING();
	int VotesAllowed = READ_BYTE();
	logfile << "[MsgFunc_InitHUD: Charnum]"
			<< "\r\n";
	ChooseChar_Interface::ServerCharNum = READ_BYTE(); //Number of characters the server allows you to have

	logfile << "[MsgFunc_InitHUD: Clearvotes]"
			<< "\r\n";
	vote_t::VotesTypesAllowed.clearitems();
	for (int i = 0; i < vote_t::VotesTypes.size(); i++)
		if (FBitSet(VotesAllowed, (1 << i)))
			vote_t::VotesTypesAllowed.add(vote_t::VotesTypes[i]);

	logfile << "Server IP: " << g_NetCode.m.HostIP.c_str() << "\r\n";

	dbg("Call InitHUDData() on all");
	// prepare all hud data
	HUDLIST *pList = m_pHudList;

	logfile << "[MsgFunc_InitHUD: InitHUDData]"
			<< "\r\n";
	while (pList)
	{
		dbg(msstring("Call InitHUDData on ") + pList->p->Name);
		if (pList->p)
			pList->p->InitHUDData();
		pList = pList->pNext;
	}

	//This would normally be called only after the scripts
	//were downloaded... but since downloading new scripts
	//isn't supported anymore, just call it
	logfile << "[MsgFunc_InitHUD: SpawnIntoServer]"
			<< "\r\n";
	dbg("Call SpawnIntoServer( )");
	MSCLGlobals::SpawnIntoServer();
	logfile << "[MsgFunc_InitHUD: NewMap]"
			<< "\r\n";
	MSGlobals::NewMap(); //Start new map

	//Do this last
	logfile << "[MsgFunc_InitHUD: InitNewLevel]"
			<< "\r\n";
	CEnvMgr::InitNewLevel();
	logfile << "[MsgFunc_InitHUD: Complete]"
			<< "\r\n";
	enddbg;
}

int CHud ::MsgFunc_GameMode(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	m_Teamplay = READ_BYTE();

	return 1;
}

int CHud ::MsgFunc_Damage(const char *pszName, int iSize, void *pbuf)
{
	int armor, blood;
	Vector from;
	int i;
	float count;

	BEGIN_READ(pbuf, iSize);
	armor = READ_BYTE();
	blood = READ_BYTE();

	for (i = 0; i < 3; i++)
		from[i] = READ_COORD();

	count = (blood * 0.5) + (armor * 0.5);

	if (count < 10)
		count = 10;

	// TODO: kick viewangles,  show damage visually

	return 1;
}

int CHud::MsgFunc_ViewModel( const char *pszName, int iSize, void *pbuf )
{
  BEGIN_READ(pbuf, iSize);
  cl_entity_t *view = gEngfuncs.GetViewModel();
  if ( view )
  {
    view->curstate.rendermode       = READ_BYTE();
    view->curstate.renderfx         = READ_BYTE();
    view->curstate.rendercolor.r    = READ_BYTE();
    view->curstate.rendercolor.g    = READ_BYTE();
    view->curstate.rendercolor.b    = READ_BYTE();
    view->curstate.renderamt        = READ_BYTE();
    view->curstate.skin             = READ_BYTE();
    view->curstate.body             = READ_BYTE();
  }
	
	return 1;
}

/*int CHud :: MsgFunc_Concuss( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	m_iConcussionEffect = READ_BYTE();
	if (m_iConcussionEffect)
		this->m_StatusIcons.EnableIcon("dmg_concuss",255,160,0);
	else
		this->m_StatusIcons.DisableIcon("dmg_concuss");
	return 1;
}*/
