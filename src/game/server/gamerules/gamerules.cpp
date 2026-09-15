/***
*
*	Copyright (c) 1999, 2000 Valve LLC. All rights reserved.
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
//=========================================================
// GameRules.cpp
//=========================================================

#include "msdllheaders.h"
#include "player/player.h"
#include "weapons/weapons.h"
#include "gamerules.h"
#include "skill.h"
#include "msgamerules.h"

extern edict_t *EntSelectSpawnPoint(CBaseEntity *pPlayer);

DLL_GLOBAL CGameRules *g_pGameRules = nullptr;
extern DLL_GLOBAL BOOL g_fGameOver;
extern int gmsgDeathMsg; // client dll messages
extern int gmsgScoreInfo;
extern int gmsgMOTD;

//=========================================================
//=========================================================
BOOL CGameRules::CanHaveAmmo(CBasePlayer *pPlayer, const char *pszAmmoName, int iMaxCarry)
{
	return FALSE;
}

//=========================================================
//=========================================================
edict_t *CGameRules ::GetPlayerSpawnSpot(CBasePlayer *pPlayer)
{
	return nullptr;
}

//=========================================================
//=========================================================
/*BOOL CGameRules::CanHavePlayerItem( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon )
{
	// only living players can have items
	if ( pPlayer->pev->deadflag != DEAD_NO )
		return FALSE;

	if ( pWeapon->pszAmmo1() )
	{
		if ( !CanHaveAmmo( pPlayer, pWeapon->pszAmmo1(), pWeapon->iMaxAmmo1() ) )
		{
			// we can't carry anymore ammo for this gun. We can only 
			// have the gun if we aren't already carrying one of this type
			if ( pPlayer->HasPlayerItem( pWeapon ) )
			{
				return FALSE;
			}
		}
	}
	else
	{
		// weapon doesn't use ammo, don't take another if you already have it.
		if ( pPlayer->HasPlayerItem( pWeapon ) )
		{
			return FALSE;
		}
	}

	// note: will fall through to here if GetItemInfo doesn't fill the struct!
	return TRUE;
}*/

//=========================================================
// load the SkillData struct with the proper values based on the skill level.
//=========================================================
void CGameRules::RefreshSkillData(void)
{
	int iSkill;

	iSkill = (int)CVAR_GET_FLOAT("skill");

	if (iSkill < 1)
	{
		iSkill = 1;
	}
	else if (iSkill > 3)
	{
		iSkill = 3;
	}

	gSkillData.iSkillLevel = iSkill;

	//ALERT ( at_console, "\nGAME SKILL LEVEL:%d\n",iSkill );
}

//=========================================================
// instantiate the proper game rules object
//=========================================================

CGameRules *InstallGameRules(void)
{
	SERVER_COMMAND("exec game.cfg\n");
	SERVER_EXECUTE();

	return msnew CHalfLifeMultiplay;
}
