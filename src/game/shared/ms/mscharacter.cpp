/*
	MSCharacter.cpp - Shared character definitions
*/

#include "inc_weapondefs.h"
#include "stats/stats.h"
#ifdef VALVE_DLL
#include "global.h"
#include "fn/FNSharedDefs.h"
#else
#include "inc_huditem.h"
#include "ms/clglobal.h"
#include "vgui_scorepanel.h"
#endif
#include "mscharacter.h"
#include "magic.h"
#include "script.h"

#ifndef _WIN32
#include "sys/io.h"
#include <sys/stat.h>
#include <sys/types.h>
#else
#include <direct.h> //for mkdir()
#endif

//Vector	MSChar_Interface::LastGoodPos,
//		MSChar_Interface::LastGoodAng;

#ifdef VALVE_DLL
void ReplaceChar(char *pString, char org, char dest);

const char *GetSaveFileName(int iCharacter, CBasePlayer *pPlayer)
{
	static char cFileName[MAX_PATH];

	msstring FileID = GETPLAYERAUTHID(pPlayer->edict());
	msstring Prefix = FNShared::IsEnabled() ? "central_" : "";

	//Thothie MAR2010_08 emergency work around
	//iCharacter = pPlayer->m_CharacterNum;
	//Print("CHAR_SAVE_DEBUG [MSCharacter]: %i vs %i\n",iCharacter+1,pPlayer->m_CharacterNum);

	_snprintf(cFileName, MAX_PATH,  "%s/save/%s%s_%i.char", EngineFunc::GetGameDir(), Prefix.c_str(), FileID.c_str(), iCharacter + 1);
	ReplaceChar(cFileName, ':', '-');

	return cFileName;
}

const char *GetSaveFileName(int iCharacter, const char *AuthID)
{
	static char cFileName[MAX_PATH];

	//Server
	Print("CHAR_SAVE_DEBUG [GetSaveFileName]: %s %#i\n", cFileName, iCharacter + 1);
	_snprintf(cFileName, MAX_PATH, "%s/save/%s_%i.char", EngineFunc::GetGameDir(), AuthID, iCharacter + 1);
	ReplaceChar(cFileName, ':', '-');

	return cFileName;
}
#endif

jointype_e MSChar_Interface::CanJoinThisMap(savedata_t &Data, const std::vector<std::string> &VisitedMaps)
{
	//phase this function out.  Use the one below
	jointype_e JoinType = JN_NOTALLOWED;
	if (MSGlobals::CanCreateCharOnMap)
		JoinType = JN_STARTMAP;		//Can create a character on this map

	if (!_stricmp(Data.MapName, MSGlobals::MapName) || //Already in this map Or trying to
			 !_stricmp(Data.NextMap, MSGlobals::MapName))   //transition to this map
		JoinType = JN_TRAVEL;
	else if (HasVisited(MSGlobals::MapName, VisitedMaps) &&
			 GetOtherPlayerTransition(NULL))
		JoinType = JN_STARTMAP; // Already visited this map before and at least
								//1 other player is currently playing it
	else if (Data.IsElite)
		JoinType = JN_ELITE; //GM.  You can always join any may

	if (strcmp(Data.Name, "LOAD_ERROR-RECONNECT") == 0)
	{
		JoinType = JN_NOTALLOWED;
	}
	if (strcmp(Data.Name, "LOAD_FAILED-RECONNECT") == 0)
	{
		JoinType = JN_NOTALLOWED;
	}
	//If this character is one that shouldn't be loaded
	//then don't allow it to be loaded.  ---MiB---

	return JoinType;
}
jointype_e MSChar_Interface::CanJoinThisMap(charinfo_t &CharData, const std::vector<std::string> &VisitedMaps)
{
	jointype_e JoinType = JN_NOTALLOWED;
	if (MSGlobals::CanCreateCharOnMap)
		JoinType = JN_STARTMAP;	//Can create a character on this map

	if (!_stricmp(CharData.MapName, MSGlobals::MapName) || //Already in this map Or trying to
			 !_stricmp(CharData.NextMap, MSGlobals::MapName))   //transition to this map
		JoinType = JN_TRAVEL;
	else if (HasVisited(MSGlobals::MapName, VisitedMaps) &&
			 GetOtherPlayerTransition(NULL))
		JoinType = JN_STARTMAP; // Already visited this map before and at least
								//1 other player is currently playing it
	else if (CharData.IsElite)
		JoinType = JN_ELITE; //GM.  You can always join any may

	return JoinType;
}

bool MSChar_Interface::HasVisited(const char* MapName, const std::vector<std::string> &VisitedMaps)
{
	for (const std::string &Map : VisitedMaps)
		if (Map == MapName)
			return true;
	return false;
}
