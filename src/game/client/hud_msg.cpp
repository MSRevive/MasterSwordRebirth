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
#include "mscharacter.h"
#include "clglobal.h"
#include "clenv.h"
#include "vgui_choosecharacter.h"
#include "monsters/msmonster.h"
#include "mslogger.h"
#include "ms/angelscript/CAngelScriptManager.h"
#include <angelscript.h>

int CHud ::MsgFunc_ResetHUD(const char *pszName, int iSize, void *pbuf)
{
	//ASSERT( iSize == 0 );

	// clear all hud data
	for (auto hudElement : m_HudList)
	{
		hudElement->Reset();
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
	MS_INFO("[MsgFunc_InitHUD: EndMap]");
	MSGlobals::EndMap(); //End old map

	//Copy over the mapname here because the engine doesn't send it
	//interally until later
	MS_INFO("[MsgFunc_InitHUD: Globals]");
	BEGIN_READ(pbuf, iSize);
	MSGlobals::ServerName = READ_STRING();
	MSGlobals::MapName = READ_STRING();
	//g_NetCode.m.HostIP = READ_STRING();

	MS_INFO("[MsgFunc_InitHUD: CLEnt Readin]");
	for (int i = 0; i < CLPERMENT_TOTALK; i++)
		MSGlobals::ClEntities[i] = READ_SHORT();
	int flags = READ_BYTE();

	MS_INFO("[MsgFunc_InitHUD: SetupDefGlobals]");
	MSCLGlobals::OnMyOwnListenServer = (flags & (1 << 0)) ? true : false;
	MSGlobals::IsLanGame = (flags & (1 << 1)) ? true : false;
	MSGlobals::CanCreateCharOnMap = (flags & (1 << 2)) ? true : false;
	MSGlobals::GameType = (flags & (1 << 3)) ? GAMETYPE_ADVENTURE : GAMETYPE_CHALLENGE;
	MSGlobals::ServerSideChar = (flags & (1 << 4)) ? true : false;
	MSCLGlobals::OtherPlayers = (flags & (1 << 5)) ? true : false;

	MSCLGlobals::AuthID = READ_STRING();
	int VotesAllowed = READ_BYTE();
	MS_INFO("[MsgFunc_InitHUD: AuthID] %s", MSCLGlobals::AuthID.c_str());

	ChooseChar_Interface::ServerCharNum = READ_BYTE(); //Number of characters the server allows you to have
	MS_INFO("[MsgFunc_InitHUD: Charnum] %d", ChooseChar_Interface::ServerCharNum);

	MS_INFO("[MsgFunc_InitHUD: Clearvotes]");
	vote_t::VotesTypesAllowed.clearitems();
	for (int i = 0; i < vote_t::VotesTypes.size(); i++)
		if (FBitSet(VotesAllowed, (1 << i)))
			vote_t::VotesTypesAllowed.add(vote_t::VotesTypes[i]);

	// prepare all hud data

	MS_INFO("[MsgFunc_InitHUD: InitHUDData]");
	for (auto hudElement : m_HudList)
	{
		hudElement->InitHUDData();
	}

	//This would normally be called only after the scripts
	//were downloaded... but since downloading new scripts
	//isn't supported anymore, just call it
	MS_INFO("[MsgFunc_InitHUD: SpawnIntoServer]");
	MSCLGlobals::SpawnIntoServer();

	MS_INFO("[MsgFunc_InitHUD: NewMap]");
	MSGlobals::NewMap(); //Start new map

	//Do this last
	MS_INFO("[MsgFunc_InitHUD: InitNewLevel]");
	CEnvMgr::InitNewLevel();

	MS_INFO("[MsgFunc_InitHUD: Complete]");

	// Call client-side VGUI test script
	MS_INFO("[MsgFunc_InitHUD: Calling CreateTestControlPanel]");

	asIScriptEngine* engine = CAngelScriptManager::Instance()->GetEngine();
	if (engine)
	{
		asUINT moduleCount = engine->GetModuleCount();
		MS_INFO("[MsgFunc_InitHUD: Found %u loaded modules]", moduleCount);

		// Search all modules for the CreateTestControlPanel function
		asIScriptFunction* func = nullptr;
		asIScriptModule* foundModule = nullptr;

		for (asUINT i = 0; i < moduleCount; i++)
		{
			asIScriptModule* module = engine->GetModuleByIndex(i);
			if (!module)
				continue;

			const char* moduleName = module->GetName();
			MS_INFO("[MsgFunc_InitHUD: Checking module '%s']", moduleName);

			func = module->GetFunctionByName("CreateTestControlPanel");
			if (func)
			{
				foundModule = module;
				MS_INFO("[MsgFunc_InitHUD: Found CreateTestControlPanel in module '%s']", moduleName);
				break;
			}
		}

		if (func && foundModule)
		{
			// Create and execute the function
			asIScriptContext* ctx = engine->CreateContext();
			if (ctx)
			{
				int r = ctx->Prepare(func);
				if (r >= 0)
				{
					r = ctx->Execute();
					if (r == asEXECUTION_FINISHED)
					{
						MS_INFO("[MsgFunc_InitHUD: CreateTestControlPanel executed successfully]");
					}
					else if (r == asEXECUTION_EXCEPTION)
					{
						MS_ERROR("[MsgFunc_InitHUD: Exception in CreateTestControlPanel: %s]",
								 ctx->GetExceptionString());
					}
					else
					{
						MS_ERROR("[MsgFunc_InitHUD: CreateTestControlPanel execution failed: %d]", r);
					}
				}
				else
				{
					MS_ERROR("[MsgFunc_InitHUD: Failed to prepare CreateTestControlPanel function]");
				}
				ctx->Release();
			}
			else
			{
				MS_ERROR("[MsgFunc_InitHUD: Failed to create AngelScript context]");
			}
		}
		else
		{
			MS_INFO("[MsgFunc_InitHUD: CreateTestControlPanel function not found in any module]");
		}
	}
	else
	{
		MS_ERROR("[MsgFunc_InitHUD: AngelScript engine not available]");
	}
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
