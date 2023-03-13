#include "msdllheaders.h"
#include "msitemdefs.h"
#include "animation.h"
#include "weapons/weapons.h"
#include "player/player.h"
#include "shield.h"
#include "logger.h"

void MSTraceLine(const Vector &vecSrc, const Vector &vecEnd, IGNORE_MONSTERS igmon, edict_t *pentIgnore, TraceResult &tr, int flags)
{
	startdbg; //Do exception handling on the traceline so if it fails
	bool fSolidShields = FBitSet(flags, MSTRACE_SOLIDSHIELDS) ? true : false;
	bool fEnlargeboxes = FBitSet(flags, MSTRACE_LARGEHITBOXES) ? true : false;
	bool fEnableCorpses = FBitSet(flags, MSTRACE_HITCORPSES) ? true : false;

	//G_SolidifyEnts( true, fSolidShields, fEnableCorpses, fEnlargeboxes );
	dbg("Call UTIL_TraceLine");
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, pentIgnore, &tr); //At least the objects' bboxes will be cleaned up
	//G_SolidifyEnts( false, fSolidShields, fEnableCorpses, fEnlargeboxes );
	enddbg;
}

void G_SolidifyEnts(bool fEnable, bool fSolidShields, bool fEnableCorpses, bool fEnlargeboxes)
{
	startdbg;
	dbg("Begin");
	//Make corpses solid here too...
	CBaseEntity *pEnt = NULL;
	edict_t *pEdict = g_engfuncs.pfnPEntityOfEntIndex(1);
	for (int i = 1; i < gpGlobals->maxEntities; i++, pEdict++)
	{
		if (pEdict->free) // Not in use
			continue;

		CBaseEntity *pEnt = MSInstance(pEdict);
		if (!pEnt)
			continue;

		dbg("Check if monster");
		if (pEnt->IsMSMonster())
		{
			dbg("Handle Monster");
			//Enlarge the hitbox on the live monsters
			if (fEnlargeboxes && pEnt->pev->health > 0 && pEnt->pev->model && (pEnt->pev->solid == SOLID_BBOX))
			{
				if (fEnable)
				{
					dbg("Expand Monster HitBox");
					pEnt->OldBounds[0] = pEnt->pev->mins;
					pEnt->OldBounds[1] = pEnt->pev->maxs;
					Vector Mins, Maxs;
					ExtractBbox(GET_MODEL_PTR(pEnt->edict()), pEnt->pev->sequence, Mins, Maxs);
					UTIL_SetSize(pEnt->pev, Mins, Maxs);
				}
				else
				{
					dbg("Contract Monster Hitbox");
					UTIL_SetSize(pEnt->pev, pEnt->OldBounds[0], pEnt->OldBounds[1]);
				}
				//UTIL_SetOrigin( pEnt->pev, pEnt->pev->origin );
			}

			continue; //Don't making corpses solid anymore.  It's causing an exception in UTIL_Traceline

			//Make the dead monsters solid
			if (!fEnableCorpses)
				continue;

			//Must be dead
			if (pEnt->pev->deadflag != DEAD_DEAD)
				continue;

			//dbg( "Check Skinnable" );
			//Must be a skinnable monster
			//CMSMonster *pMonster = (CMSMonster *)pEnt;
			//if( !pMonster->Skin ) continue;
		}
		else if (!fSolidShields || pEnt->MSProperties() != MS_SHIELD)
			continue;

		dbg("Set Solididity");
		pEnt->pev->solid = fEnable ? SOLID_BBOX : SOLID_NOT;
		//UTIL_SetOrigin( pEnt->pev, pEnt->pev->origin );
	}

	dbg(msstring("End (") + (fEnable ? "Enabled" : "Disabled") + ")");
	enddbg;
}

//
//  CShieldArea
//  �����������
void CShieldArea ::Spawn()
{
	pev->movetype = MOVETYPE_FLY;
	pev->effects |= EF_NODRAW;
	SET_MODEL(edict(), STRING(pev->model));
	FollowThink();
}
float CShieldArea::TraceAttack(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType, int iAccuracyRoll)
{
	return flDamage;
}
void CShieldArea ::FollowThink()
{
	CBasePlayerItem *pOwner = (CBasePlayerItem *)MSInstance(pev->owner);
	if (!pOwner || !pOwner->pev->iuser1)
	{
		SUB_Remove();
		return;
	}
	CBaseMonster *pPlayer = (CBaseMonster *)MSInstance(pOwner->pev->owner);
	if (!pPlayer)
	{
		SUB_Remove();
		return;
	}

	UTIL_MakeVectors(pPlayer->pev->v_angle);
	float fTemp = -pPlayer->pev->v_angle.x;
	pev->origin = pPlayer->pev->origin +
				  pPlayer->pev->view_ofs + (gpGlobals->v_forward * 16) //24
											   * V_max(1 - ((fTemp < 0 ? fTemp : fTemp * 2.11) / 190), 0.1);

	pev->angles = UTIL_VecToAngles(gpGlobals->v_forward);

	UTIL_SetSize(pev, Vector(-30, -30, -30), Vector(30, 30, 30));
	UTIL_SetOrigin(pev, pev->origin);

	SetThink(&CShieldArea::FollowThink);
	pev->nextthink = gpGlobals->time;
}
void CShieldArea ::CounterEffect(CBaseEntity *pInflictor, int iEffect, void *pExtraData)
{
	CBaseEntity *pPlayerShield = MSInstance(pev->owner);
	if (!pPlayerShield)
		return;
	pPlayerShield->CounterEffect(pInflictor, CE_SHIELDHIT, pExtraData);
}
