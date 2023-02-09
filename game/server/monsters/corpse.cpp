#include "inc_weapondefs.h"

#include "player/modeldefs.h"

#include "corpse.h"
extern int iBeam;

LINK_ENTITY_TO_CLASS(player_corpse, CCorpse);

void CCorpse::Spawn()
{
	pev->classname = MAKE_STRING(CLASS_CORPSE);
	//	pev->flags |= FL_DORMANT;
	m_Gold = 0;
	m_Volume = 0;
}
float CCorpse::DamageForce(float damage)
{
	float force = damage * ((32 * 32 * 72.0) / (pev->size.x * pev->size.y * pev->size.z)) * 5;

	if (force > 1000.0)
		force = 1000.0;

	return force;
}

void CCorpse::CreateCorpse(CMSMonster* pSource, float LoseGoldPercent)
{
	//	if (pSource->pev->effects & EF_NODRAW) return;

	CBasePlayer* pPlayer = NULL;
	if (pSource->IsPlayer()) {
		pPlayer = (CBasePlayer*)pSource;
		pPlayerSource = pPlayer;
	}

	Spawn();
	int LoseGold = pSource->m_Gold * (LoseGoldPercent * 0.01);
	m_Gold = LoseGold;
	pSource->m_Gold -= LoseGold;
	m_Volume = pSource->Volume();
	//If dead, Make the bounding box the dead body size
	//SetSequenceBox( 16 );
	pev->maxs = pSource->pev->maxs;
	pev->maxs.z = -20;
	pev->mins = pSource->pev->mins;
	pev->mins.z = -36;
	UTIL_SetSize(pev, pev->mins, pev->maxs);

	//Let the player hit it
	pev->health = pSource->pev->max_health;
	pev->takedamage = DAMAGE_YES;
	CBaseEntity* pCorpse = NULL;

	/*if( pSource->IsPlayer() ) pev->nextthink = gpGlobals->time + 300.0; //player get a good five minutes
	else */
	pev->nextthink = gpGlobals->time + MSITEM_TIME_EXPIRE;
	pev->ltime = gpGlobals->time; //save the time the corpse was created

	float CheckRange = 1024;
	Vector delta = Vector(CheckRange, CheckRange, CheckRange);
	CBaseEntity* pEnt[100];
	int count = UTIL_EntitiesInBox(pEnt, 100, pev->origin - delta, pev->origin + delta, NULL);
	for (int i = 0; i < count; i++)
	{
		//Cut every corpse's expire time in half
		pCorpse = pEnt[i];
		if (!pCorpse || pCorpse == this || !pCorpse->MyMonsterPointer() || pCorpse->IsAlive() || pCorpse->m_pfnThink != &CBaseEntity::SUB_StartFadeOut)
			continue;
		float flExpireDelay = pCorpse->pev->nextthink - pCorpse->pev->ltime;
		flExpireDelay *= 0.5;
		pCorpse->pev->nextthink = pCorpse->pev->ltime + flExpireDelay;
	}

	SetThink(&CBaseEntity::SUB_StartFadeOut);

	/*if( pSource->Body ) {
		Body = pSource->Body->Duplicate();
		Body->Set( BPS_OWNER, this );
		Body->Think( pSource );						//Think once, to set the proper armor models
	}*/

	m_DisplayName = UTIL_VarArgs("%s's corpse", pSource->DisplayName());
	pev->netname = pSource->pev->netname;
	pev->origin = pSource->pev->origin;
	pev->angles = pSource->pev->angles;
	pev->model = pSource->pev->model;
	pev->body = pSource->pev->body;
	pev->modelindex = pSource->pev->modelindex;
	pev->sequence = pSource->pev->sequence;
	pev->gaitsequence = pSource->pev->gaitsequence;
	//pev->colormap	= pSource->pev->colormap;
	pev->gravity = pSource->pev->gravity;
	;
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_NOT;
	//---------------------------
	pev->velocity = pSource->pev->velocity;

	//if the corpse was made by a player pass the edicts.
	pev->flags = FL_MONSTER;

	pev->deadflag = pSource->pev->deadflag;
	//pev->rendermode	= kRenderTransAlpha;	//Currently showing player the model and not using gear
	//pev->renderamt = 1;
	//	pev->renderfx	= kRenderFxDeadPlayer;
	//	pev->renderamt	= ENTINDEX( pSource->edict() );

	InitBoneControllers();
	ResetSequenceInfo();
	pev->frame = 0;

	//Test the dead body size
	/*BeamEffect( pev->absmin.x, pev->absmin.y, pev->absmin.z, pev->absmax.x,
				pev->absmax.y, pev->absmax.z, iBeam, 0, 0, 250, 20, 0, 255, 255,255,
				255, 20 );
	*/
}

bool CCorpse::AddItem(CGenericItem* pItem, bool ToHand, bool CheckWeight, int ForceHand)
{
	bool fReturn = true;
	pItem->AddToOwner(this);

	if (FBitSet(pItem->MSProperties(), ITEM_ARMOR))
	{
		if (!pItem->WearItem())
		{
			pItem->SUB_Remove();
			fReturn = FALSE;
		}
	}
	else
		fReturn = Gear.AddItem(pItem);

	return fReturn;
}
bool CCorpse::RemoveItem(CGenericItem* pItem)
{
	pItem->RemoveFromOwner();
	return true;
}
void CCorpse::Deactivate()
{
	//	pev->effects |= EF_NODRAW;

	// Drop all corpse items.
	//Get a pointer to each item in my gear, then call droo on each item
	int count = Gear.size();
	int i = 0;

	CGenericItem** Gearlist = new (CGenericItem * [count]);
	for (i = 0; i < count; i++)
		Gearlist[i] = Gear[i];

	for (i = 0; i < (signed)count; i++)
	{
		Gearlist[i]->Drop();
		Gearlist[i]->pev->angles = Vector(0, RANDOM_FLOAT(0, 359), 0);
		Gearlist[i]->pev->origin = pev->origin + Vector(RANDOM_FLOAT(-75, 75), RANDOM_FLOAT(-75, 75), 0);
	}

	CMSMonster::Deactivate();

	/*	pev->model = MAKE_STRING(SKELETON_MODEL);
	SET_MODEL( edict(), STRING(pev->model) );
	pev->mins = Vector(-13,-29,0);
	pev->maxs = Vector(13,29,6);
	UTIL_SetSize(pev, pev->mins, pev->maxs);
	UTIL_SetOrigin( pev, pev->origin );

	pev->deadflag	= 0;
	pev->renderfx	= kRenderFxNone;//kRenderFxDeadPlayer;
	pev->renderamt	= 0;

	pev->sequence = 20;
	pev->frame = 0;
	pev->framerate = 0.0;
	pev->flags &= ~FL_ONGROUND;

	//Traceline your way to the ground now
	TraceResult tr;
	Vector vecSrc	= pev->origin;
	Vector vecEnd	= vecSrc - Vector(0,0,4000);
	gpGlobals->trace_flags |= FTRACE_SIMPLEBOX;
	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, edict(), &tr );
	if ( tr.flFraction < 1.0 ) {
		pev->origin = tr.vecEndPos + Vector(0,0,3);
		UTIL_SetOrigin( pev, pev->origin );
	}
	else {
		SkeletonDie( );
		return;
	}

//	BeamEffect( pev->absmin, pev->absmax, iBeam, 0, 0, 100, 10, 0, 255, 255,255, 255, 20 );

	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;
	pev->takedamage = DAMAGE_NO;
	SetThink( NULL );
	pev->nextthink = 0;

	SetThink( SkeletonDie );
	pev->nextthink = gpGlobals->time + 60.0;*/
}
int CCorpse::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	// kill the corpse if enough damage was done to destroy the corpse and the
	// damage is of a type that is allowed to destroy the corpse.

	if (!(bitsDamageType & DMG_GIB_CORPSE))
		return 0;

	pev->health -= flDamage;

	if (pev->health > 0)
		return flDamage;

	GibMonster();

	return flDamage;
}
