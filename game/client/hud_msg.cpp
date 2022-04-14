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

#include "msdllheaders.h"
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "string.h"
#include "logger.h"
#include "msnetcodeclient.h"
#include "mscharacter.h"
#include "clglobal.h"
#include "clenv.h"
#include "vgui_choosecharacter.h"
#include "monsters/msmonster.h"

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

	logfile << Logger::LOG_INFO << "[MsgFunc_InitHUD: EndMap]\n";
	MSGlobals::EndMap(); //End old map

	//Copy over the mapname here because the engine doesn't send it
	//interally until later
	logfile << Logger::LOG_INFO << "[MsgFunc_InitHUD: Globals]\n";
	BEGIN_READ(pbuf, iSize);
	MSGlobals::ServerName = READ_STRING();
	MSGlobals::MapName = READ_STRING();
	//g_NetCode.m.HostIP = READ_STRING();
	logfile << Logger::LOG_INFO << "[MsgFunc_InitHUD: CLEnt Readin]\n";
	for (int i = 0; i < CLPERMENT_TOTALK; i++)
		MSGlobals::ClEntities[i] = READ_SHORT();
	int flags = READ_BYTE();
	logfile << Logger::LOG_INFO << "[MsgFunc_InitHUD: SetupDefGlobals]\n";
	MSCLGlobals::OnMyOwnListenServer = (flags & (1 << 0)) ? true : false;
	MSGlobals::IsLanGame = (flags & (1 << 1)) ? true : false;
	MSGlobals::CanCreateCharOnMap = (flags & (1 << 2)) ? true : false;
	MSGlobals::GameType = (flags & (1 << 3)) ? GAMETYPE_ADVENTURE : GAMETYPE_CHALLENGE;
	MSGlobals::ServerSideChar = (flags & (1 << 4)) ? true : false;
	MSCLGlobals::OtherPlayers = (flags & (1 << 5)) ? true : false;

	logfile << Logger::LOG_INFO << "[MsgFunc_InitHUD: AuthID]\n";
	MSCLGlobals::AuthID = READ_STRING();
	int VotesAllowed = READ_BYTE();
	logfile << Logger::LOG_INFO << "[MsgFunc_InitHUD: Charnum]\n";
	ChooseChar_Interface::ServerCharNum = READ_BYTE(); //Number of characters the server allows you to have

	logfile << Logger::LOG_INFO << "[MsgFunc_InitHUD: Clearvotes]\n";
	vote_t::VotesTypesAllowed.clearitems();
	for (int i = 0; i < vote_t::VotesTypes.size(); i++)
		if (FBitSet(VotesAllowed, (1 << i)))
			vote_t::VotesTypesAllowed.add(vote_t::VotesTypes[i]);

	logfile << Logger::LOG_INFO << "Server IP: " << g_NetCode.m.HostIP.c_str() << "\n";

	dbg("Call InitHUDData() on all");
	// prepare all hud data
	HUDLIST *pList = m_pHudList;

	logfile << Logger::LOG_INFO << "[MsgFunc_InitHUD: InitHUDData]\n";
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
	logfile << Logger::LOG_INFO << "[MsgFunc_InitHUD: SpawnIntoServer]\n";
	dbg("Call SpawnIntoServer( )");
	MSCLGlobals::SpawnIntoServer();
	logfile << Logger::LOG_INFO << "[MsgFunc_InitHUD: NewMap]\n";
	MSGlobals::NewMap(); //Start new map

	//Do this last
	logfile << Logger::LOG_INFO << "[MsgFunc_InitHUD: InitNewLevel]\n";
	CEnvMgr::InitNewLevel();
	logfile << Logger::LOG_INFO << "[MsgFunc_InitHUD: Complete]\n";
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
