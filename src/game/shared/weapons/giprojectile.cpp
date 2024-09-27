/***
*
*	Copyright (c) 2000, Kenneth "Dogg" Early.
*	
*	Email kene@maverickdev.com or
*		  kearly@crosswinds.net
*
****/
//
//	Special arrow behavior - Should be all server side...
//

#ifdef VALVE_DLL

#include "inc_weapondefs.h"
#include "stats/stats.h"
#include "stats/statdefs.h"
#include "logger.h"

struct projectiledata_t
{
	float Damage;
	float flDamageAOERange;
	float flDamageAOEAttn;
	string_i sDamageType;
	bool CollideHitBox; //Collide with hitboxes instead of just bboxes
	//Dynamic Data
	int StatPower, PropPower;
	int StatExp, PropExp; //Stat and prop that receives exp from this attack
	entityinfo_t OriginalOwner;
	float Speed;
	bool IgnoreNPC;	  //Ignore NPCs, collide with world only
	bool IgnoreWorld; //Float through walls
};

#define TypeCheck        \
	if (!ProjectileData) \
	return

void CGenericItem::RegisterProjectile()
{
	if (ProjectileData)
		delete ProjectileData;

	ProjectileData = new (projectiledata_t);
	memset(ProjectileData, 0, sizeof(projectiledata_t));

	ProjectileData->Damage = atof(GetFirstScriptVar("reg.proj.dmg"));
	ProjectileData->flDamageAOERange = atof(GetFirstScriptVar("reg.proj.aoe.range"));
	ProjectileData->flDamageAOEAttn = atof(GetFirstScriptVar("reg.proj.aoe.falloff"));
	//ProjectileData->TimeStickTime = atof(GetFirstScriptVar("reg.proj.stick.duration"));
	ProjectileData->CollideHitBox = atoi(GetFirstScriptVar("reg.proj.collidehitbox")) ? true : false;
	//ProjectileData->sAttackStat = ALLOC_STRING( GetFirstScriptVar("PROJ_DAMAGESTAT") );
	ProjectileData->sDamageType = GetFirstScriptVar("reg.proj.dmgtype");
	ProjectileData->IgnoreNPC = atoi(GetFirstScriptVar("reg.proj.ignorenpc")) ? true : false;	  //Thothie DEC2007a - for certain magic projectiles that pass through models
	ProjectileData->IgnoreWorld = atoi(GetFirstScriptVar("reg.proj.ignoreworld")) ? true : false; //Thothie FEB2009_16 - float through walls
}
void CGenericItem::TossProjectile(CBaseEntity *pTossDevice, Vector &vOrigin, Vector &vVelocity, float flDamage, int iStat, int iProp)
{
	TypeCheck;
	if (!pTossDevice)
		return;

	float flDamageMulti; //OCT2007a
	float flStartingDamage = ProjectileData->Damage;

	if (pTossDevice->IsMSItem())
	{
		CGenericItem *pTossItem = (CGenericItem *)pTossDevice;
		if (!pTossItem->CurrentAttack)
			return;
		ProjectileData->StatPower = pTossItem->CurrentAttack->StatPower;
		ProjectileData->PropPower = pTossItem->CurrentAttack->PropPower;
		ProjectileData->StatExp = pTossItem->CurrentAttack->StatExp;
		ProjectileData->PropExp = pTossItem->CurrentAttack->PropExp;
		if (pTossItem->CurrentAttack->f1DmgMulti > 0) //OCT2007a
		{
			flDamageMulti = pTossItem->CurrentAttack->f1DmgMulti;
			ProjectileData->Damage = flStartingDamage * flDamageMulti; //*= pTossItem->CurrentAttack->f1DmgMulti was not working
		}
		pev->owner = pTossItem->m_pOwner->edict();
		ProjectileData->OriginalOwner = pTossItem->m_pOwner;
		StoreEntity(pTossItem->m_pOwner, ENT_EXPOWNER);
	}
	else
	{
		if (pTossDevice->IsMSMonster())
		{
			if (iStat)
			{
				bool RandProp = false;
				if (iProp < 0)
				{
					RandProp = true;
				}

				ProjectileData->StatPower = iStat;
				ProjectileData->PropPower = !RandProp ? iProp : 2;
				ProjectileData->StatExp = iStat;
				ProjectileData->PropExp = !RandProp ? iProp : -1;
			}
			CBaseEntity *pOwner = pTossDevice->RetrieveEntity(ENT_EXPOWNER);
			if (pOwner)
				StoreEntity(pOwner, ENT_EXPOWNER);
			else
				StoreEntity(pTossDevice, ENT_EXPOWNER);
		}
		pev->owner = pTossDevice->edict();
	}

	ClearBits(lProperties, GI_JUSTSPAWNED);
	SetBits(Properties, ITEM_PROJECTILE);
	if (WorldModel.len())
		SET_MODEL(edict(), WorldModel);
	pev->origin = vOrigin;
	pev->velocity = vVelocity;
	ProjectileData->Speed = vVelocity.Length();
	pev->angles = UTIL_VecToAngles(pev->velocity);
	SetTouch(&CGenericItem::ProjectileTouch);
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;
	if (flDamage != -99999.0f)
		ProjectileData->Damage = flDamage;
	//if ( thoth_dmg_multi > 0.0 ) ProjectileData->Damage *= thoth_dmg_multi; //OCT2007a
	m_TimeExpire = gpGlobals->time + ExpireTime;
	float Width = CBaseEntity::Volume(), Height = CBaseEntity::Volume();
	UTIL_SetSize(pev, Vector(-(Width / 2), -(Width / 2), 0), Vector((Width / 2), (Width / 2), Height));
	//UTIL_SetSize( pev, Vector(-1, -1, 0), Vector(1, 1, 1));
	UTIL_SetOrigin(pev, pev->origin);
	pTossDevice->StoreEntity(this, ENT_LASTPROJECTILE);
	CallScriptEvent("game_tossprojectile");
	Think(); //Check the first frame
}
void CGenericItem::ProjectileTouch(CBaseEntity *pOther)
{
	//if ( ProjectileData->IgnoreNPC ) return;

	startdbg;

	TypeCheck;

	if (pev->owner == pOther->edict() || pOther->MSMoveType == MOVETYPE_ARROW || !FBitSet(MSProperties(), ITEM_PROJECTILE))
		return;

	float dmgMultiplier = 1.0f;

	//Thothie - trying to fix sounds of arrows always coming from 0x0x0
	Vector old_location;
	old_location = pev->origin;

	Vector vForward;
	if (pev->velocity.Length())
		vForward = pev->velocity;
	else
	{
		UTIL_MakeVectorsPrivate(Vector(-pev->angles.x, pev->angles.y, pev->angles.z), pev->velocity, NULL, NULL);
		pev->velocity *= ProjectileData->Speed;
	}

	if ((pOther->MyMonsterPointer() && !pOther->IsAlive()))
		return; //Hit a dead monster, keep going

	damage_t Damage;
	CBaseEntity *pDamageEnt = NULL;

	CBaseEntity *pOwner = RetrieveEntity(ENT_EXPOWNER);
	if (pOwner && !ProjectileData->IgnoreNPC)
	{
		if (pOwner->IsPlayer())
		{
			CMSMonster *pMonsterOwner = (CMSMonster *)pOwner;
			dmgMultiplier = pMonsterOwner->GetSkillStat(ProjectileData->StatPower, ProjectileData->PropPower) / STATPROP_MAX_VALUE;
			dmgMultiplier = V_max(dmgMultiplier, 0.001f);
		}

		clrmem(Damage);
		Damage.pInflictor = this;
		Damage.pAttacker = pOwner;
		Damage.vecSrc = pev->origin;
		Damage.vecEnd = pev->origin + vForward;
		Damage.flRange = ProjectileData->flDamageAOERange;
		Damage.flDamage = ProjectileData->Damage * dmgMultiplier;
		Damage.iDamageType = DMG_NEVERGIB | DMG_NOSKIN;
		Damage.flHitPercentage = 100.0f;
		Damage.sDamageType = ProjectileData->sDamageType;
		Damage.flAOERange = ProjectileData->flDamageAOERange;
		Damage.flAOEAttn = ProjectileData->flDamageAOEAttn;
		Damage.ExpUseProps = true;
		Damage.ExpStat = ProjectileData->StatExp;
		Damage.ExpProp = ProjectileData->PropExp;

		hitent_list Hits;
		DoDamage(Damage, Hits);
		pDamageEnt = Hits.ClosestHit();
	}

	//Thothie DEC2010_06 - trying to fix projectiles vs. big beasts (commented this out)
	if (pOther->IsMSMonster() && ProjectileData->CollideHitBox && !pDamageEnt)
	{
		//Just keep going if I hit a npc bounding box but not an actual hitbox
		return;
	}

	pev->origin = old_location; //Thothie AUG2011_15 - move back to location so sound plays from right spot
	CallScriptEvent("game_projectile_landed");

	//if( pDamageEnt )
	if (!ProjectileData->IgnoreNPC)
		pev->origin = Damage.outTraceResult.vecEndPos;

	if (pDamageEnt &&
		pDamageEnt->pev->takedamage && !ProjectileData->IgnoreWorld)
	{
		if (FBitSet(pDamageEnt->pev->flags, FL_GODMODE) && !ProjectileData->IgnoreNPC) //Monster has godmode
		{
			//Delay removal, so further arrow code doesn't crash
			DelayedRemove();
			return;
		}

		dbg("Params List");
		static msstringlist Params;
		dbg("Clear List");
		Params.clearitems();
		dbg("PDamageEnt");
		Params.add(EntToString(pDamageEnt));
		dbg("SendEvent");
		pev->origin = old_location; //Thothie AUG2011_15 - move back to location so sound plays from right spot
		CallScriptEvent("game_projectile_hitnpc", &Params);
		dbg("IgnoreNPC");
		if (ProjectileData->IgnoreNPC)
		{
			return;
		}
		else
		{
			pev->solid = SOLID_NOT;
			//ExpireTime = ProjectileData->TimeStickTime;
			MSMoveType = MOVETYPE_STUCKARROW;
		}

		//JUL2006 Thothie - Disable this buggy ass shit
		//Stick in enemy
		//pDamageEnt->pev->velocity = g_vecZero;

		//SetFollow( pDamageEnt, 0 );			//Activate the special follow.
	}
	else
	{
		//No monster or monster I hit is dead or doesn't take damage
		pev->owner = NULL;
		pev->origin = old_location; //Thothie AUG2010_26 - move back to location so sound plays from right spot
		CallScriptEvent("game_projectile_hitwall");
	}

	//if ( ProjectileData->IgnoreNPC ) return;

	m_TimeExpire = gpGlobals->time + ExpireTime;
	ClearBits(Properties, ITEM_PROJECTILE);

	pev->movetype = MOVETYPE_NONE;
	//pev->velocity = g_vecZero;
	SetTouch(NULL);
	//Think( );

	enddbg;
}
void CGenericItem::Projectile_Move()
{
	TypeCheck;

	if (MSMoveType == MOVETYPE_ARROW)
	{
		//Arrow flying through the air
		pev->angles = UTIL_VecToAngles(pev->velocity);

		if (FBitSet(MSProperties(), ITEM_PROJECTILE))
		{
			//Arrow that was shot - check for a hit
			pev->nextthink = gpGlobals->time;
			Projectile_CheckHit();
		}
	}
	else if (MSMoveType == MOVETYPE_STUCKARROW)
	{
		//Arrow stuck in a monster
		CBaseEntity *pOwner = m_AttachToEnt.Entity();
		if (!pOwner || !pOwner->IsAlive())
		{
			//Drop
			pev->movetype = MOVETYPE_TOSS;
			SetFollow(NULL);
			MSMoveType = MOVETYPE_NORMAL;
		}
	}
}
void CGenericItem::Projectile_CheckHit(void)
{
	if (ProjectileData->IgnoreNPC)
		return;

	TraceResult tr;
	Vector vMoveDir = pev->angles;
	vMoveDir.x = -vMoveDir.x;
	UTIL_MakeVectors(vMoveDir);
	Vector vecEnd = pev->origin + pev->velocity.Normalize() * 36;

	//int trflags = MSTRACE_SOLIDSHIELDS|MSTRACE_LARGEHITBOXES;
	int trflags = MSTRACE_LARGEHITBOXES; //AUG2013_22 Thothie - seeing as how we don't use MSTRACE_SOLIDSHIELDS anymore, removing it, case it is causing the hitbox problem
	//if( pAttacker ) SetBits( trflags, MSTRACE_HITCORPSES );

	MSTraceLine(pev->origin, vecEnd, dont_ignore_monsters, edict(), tr, trflags);

	if (tr.flFraction < 1.0)
	{
		CBaseEntity *pEntity = MSInstance(tr.pHit);
		if (pEntity)
		{
			/*if( pEntity->MSProperties() == MS_SHIELD ) 
			{
				CGenericItem *pShield = (CGenericItem *)CBaseEntity::Instance(pEntity->pev->owner);
				CBasePlayer *pPlayer = pShield->m_pPlayer->IsPlayer() ? pShield->m_pPlayer : NULL;
			
				pev->solid = SOLID_NOT;
				pev->velocity = -pev->velocity/6;
				if (pev->velocity.z<8)
					pev->velocity.z = pev->velocity.z;
				else
					pev->velocity.z = 8;
				//pev->velocity.z = min(pev->velocity.z,8);
				pev->origin = tr.vecEndPos;
				if( pPlayer ) 
				{
					pPlayer->pev->punchangle = Vector(RANDOM_FLOAT(-5,5),RANDOM_FLOAT(-5,5),RANDOM_FLOAT(-5,5));
					msstring ShieldName = SPEECH::ItemName( pShield );
					pPlayer->SendInfoMsg( "You block %s %s with %s %s\n", SPEECH::ItemName(this), ShieldName );
				}
				m_TimeExpire = gpGlobals->time + ExpireTime;
				SetTouch( NULL );
				ClearBits( Properties, ITEM_PROJECTILE );
			}
			else*/
			ProjectileTouch(pEntity);
		}
	}
	//BeamEffect( pev->origin, vecEnd, iBeam, 0, 0, 200, 2, 0, RANDOM_LONG(1,255), 255,255, 255, 20 );
}

//Thothie DEC2012_12 - Enable game_touch for projectiles
void CGenericItem::Projectile_TouchEnable(bool touchon)
{
	m_HandleTouch = touchon;
}

void CGenericItem::Projectile_Solidify()
{
	float Width = 6, Height = 6;
	Vector vForward, vecEnd;
	TraceResult tr;
	UTIL_MakeVectorsPrivate(Vector(-pev->angles.x, pev->angles.y, pev->angles.z), vForward, NULL, NULL);
	gpGlobals->trace_flags |= FTRACE_SIMPLEBOX;
	vecEnd = pev->origin - vForward * sqrt(pow(Width * 4, 2) + pow(Height * 4, 2));
	UTIL_TraceLine(pev->origin + vForward * 16, vecEnd, dont_ignore_monsters, edict(), &tr);
	if (tr.flFraction == 1.0)
	{
		pev->solid = SOLID_BBOX;
		UTIL_SetSize(pev, Vector(-(Width / 2), -(Width / 2), 0), Vector((Width / 2), (Width / 2), Height));
	}
	MSMoveType = MOVETYPE_NORMAL;

	m_HandleTouch = true;
}

//DEC2012_12 - Thothie - re-add climb arrows in non-sploity fashion
void CGenericItem ::Touch(CBaseEntity *pOther)
{
	//	if( FBitSet(pev->flags,FL_FLOAT) ) ALERT( at_console, "Touching %s\n", STRING(pOther->pev->classname) );
	CBaseEntity ::Touch(pOther);
	if (m_HandleTouch && (pOther->IsPlayer() || pOther->IsMSMonster()))
	{
		msstringlist Parameters;
		Parameters.add(EntToString(pOther));
		CallScriptEvent("game_touch", &Parameters);
	}
}

/*
void CGenericItem :: Used( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if( m_HandleTouch && ( pActivator && pActivator->IsPlayer( ) ) )
	{
		CBasePlayer *pPlayer = (CBasePlayer *)pActivator;

		//Using pev->euser1 is the old way, phase it out
		pev->euser1 = pActivator->edict( );

		msstringlist Parameters;
		Parameters.add( EntToString(pActivator) );
		CallScriptEvent("game_playerused", &Parameters );

		pev->euser1 = NULL;
	};
}
*/
#endif
