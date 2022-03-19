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
/*

===== weapons.cpp ========================================================

  functions governing the selection/use of weapons for players

*/

#include "msdllheaders.h"
#include "player/player.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "soundent.h"
#include "decals.h"
#ifdef VALVE_DLL
#include "gamerules/gamerules.h"
#endif
#include "msitemdefs.h"
#include "syntax/syntax.h"
#include "logfile.h"

#ifndef VALVE_DLL
#include "usercmd.h"
#include "entity_state.h"
#include "demo_api.h"
#include "pm_defs.h"
#include "event_api.h"
#include "r_efx.h"

#include "../hud_iface.h"
#include "../com_weapons.h"
#endif

extern CGraph WorldGraph;
extern int gEvilImpulse101;

#define NOT_USED 255

//DLL_GLOBAL	short	g_sModelIndexLaser;// holds the index for the laser beam
//DLL_GLOBAL  const char *g_pModelNameLaser = "sprites/laserbeam.spr";
//DLL_GLOBAL	short	g_sModelIndexLaserDot;// holds the index for the laser beam dot
DLL_GLOBAL short g_sModelIndexSmoke;	  // holds the index for the smoke cloud
DLL_GLOBAL short g_sModelIndexExplosion;  // holds the index for the explosion
DLL_GLOBAL short g_sModelIndexWExplosion; // holds the index for the underwater explosion
DLL_GLOBAL short g_sModelIndexBubbles;	  // holds the index for the bubbles model
DLL_GLOBAL short g_sModelIndexBloodDrop;  // holds the sprite index for the initial blood
DLL_GLOBAL short g_sModelIndexBloodSpray; // holds the sprite index for splattered blood

ItemInfo CBasePlayerItem::ItemInfoArray[MAX_WEAPONS];
//AmmoInfo CBasePlayerItem::AmmoInfoArray[MAX_AMMO_SLOTS];

//extern int gmsgCurWeapon;

MULTIDAMAGE gMultiDamage;

//=========================================================
// MaxAmmoCarry - pass in a name and this function will tell
// you the maximum amount of that type of ammunition that a
// player can carry.
//=========================================================
int MaxAmmoCarry(int iszName)
{
	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		if (CBasePlayerItem::ItemInfoArray[i].pszAmmo1 && !strcmp(STRING(iszName), CBasePlayerItem::ItemInfoArray[i].pszAmmo1))
			return CBasePlayerItem::ItemInfoArray[i].iMaxAmmo1;
		if (CBasePlayerItem::ItemInfoArray[i].pszAmmo2 && !strcmp(STRING(iszName), CBasePlayerItem::ItemInfoArray[i].pszAmmo2))
			return CBasePlayerItem::ItemInfoArray[i].iMaxAmmo2;
	}

	ALERT(at_console, "MaxAmmoCarry() doesn't recognize '%s'!\n", STRING(iszName));
	return -1;
}

/*
==============================================================================

MULTI-DAMAGE

Collects multiple small damages into a single damage

==============================================================================
*/

//
// ClearMultiDamage - resets the global multi damage accumulator
//
void ClearMultiDamage(void)
{
	gMultiDamage.pEntity = NULL;
	gMultiDamage.amount = 0;
	gMultiDamage.type = 0;
}

//
// ApplyMultiDamage - inflicts contents of global multi damage register on gMultiDamage.pEntity
//
// GLOBALS USED:
//		gMultiDamage

void ApplyMultiDamage(entvars_t *pevInflictor, entvars_t *pevAttacker)
{
	Vector vecSpot1; //where blood comes from
	Vector vecDir;	 //direction blood should go
	TraceResult tr;

	if (!gMultiDamage.pEntity)
		return;

	gMultiDamage.pEntity->TakeDamage(pevInflictor, pevAttacker, gMultiDamage.amount, gMultiDamage.type);
}

// GLOBALS USED:
//		gMultiDamage

void AddMultiDamage(entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType)
{
	if (!pEntity)
		return;

	gMultiDamage.type |= bitsDamageType;

	if (pEntity != gMultiDamage.pEntity)
	{
		ApplyMultiDamage(pevInflictor, pevInflictor); // UNDONE: wrong attacker!
		gMultiDamage.pEntity = pEntity;
		gMultiDamage.amount = 0;
	}

	gMultiDamage.amount += flDamage;
}

/*
================
SpawnBlood
================
*/
void SpawnBlood(Vector vecSpot, int bloodColor, float flDamage)
{
	UTIL_BloodDrips(vecSpot, g_vecAttackDir, bloodColor, (int)flDamage);
}

int DamageDecal(CBaseEntity *pEntity, int bitsDamageType)
{
	if (!pEntity)
		return (DECAL_GUNSHOT1 + RANDOM_LONG(0, 4));

	return pEntity->DamageDecal(bitsDamageType);
}

void DecalGunshot(TraceResult *pTrace, int iBulletType)
{
	// Is the entity valid
	if (!UTIL_IsValidEntity(pTrace->pHit))
		return;

	if (VARS(pTrace->pHit)->solid == SOLID_BSP || VARS(pTrace->pHit)->movetype == MOVETYPE_PUSHSTEP)
	{
		CBaseEntity *pEntity = NULL;
		// Decal the wall with a gunshot
		if (!FNullEnt(pTrace->pHit))
			pEntity = CBaseEntity::Instance(pTrace->pHit);

		switch (iBulletType)
		{
		case BULLET_PLAYER_9MM:
		case BULLET_MONSTER_9MM:
		case BULLET_PLAYER_MP5:
		case BULLET_MONSTER_MP5:
		case BULLET_PLAYER_BUCKSHOT:
		case BULLET_PLAYER_357:
		default:
			// smoke and decal
			UTIL_GunshotDecalTrace(pTrace, DamageDecal(pEntity, DMG_BULLET));
			break;
		case BULLET_MONSTER_12MM:
			// smoke and decal
			UTIL_GunshotDecalTrace(pTrace, DamageDecal(pEntity, DMG_BULLET));
			break;
		case BULLET_PLAYER_CROWBAR:
			// wall decal
			UTIL_DecalTrace(pTrace, DamageDecal(pEntity, DMG_CLUB));
			break;
		}
	}
}

//
// EjectBrass - tosses a brass shell from passed origin at passed velocity
//
void EjectBrass(const Vector &vecOrigin, const Vector &vecVelocity, float rotation, int model, int soundtype)
{
	// FIX: when the player shoots, their gun isn't in the same position as it is on the model other players see.

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecOrigin);
	WRITE_BYTE(TE_MODEL);
	WRITE_COORD(vecOrigin.x);
	WRITE_COORD(vecOrigin.y);
	WRITE_COORD(vecOrigin.z);
	WRITE_COORD(vecVelocity.x);
	WRITE_COORD(vecVelocity.y);
	WRITE_COORD(vecVelocity.z);
	WRITE_ANGLE(rotation);
	WRITE_SHORT(model);
	WRITE_BYTE(soundtype);
	WRITE_BYTE(25); // 2.5 seconds
	MESSAGE_END();
}

#if 0
// UNDONE: This is no longer used?
void ExplodeModel( const Vector &vecOrigin, float speed, int model, int count )
{
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecOrigin );
		WRITE_BYTE ( TE_EXPLODEMODEL );
		WRITE_COORD( vecOrigin.x );
		WRITE_COORD( vecOrigin.y );
		WRITE_COORD( vecOrigin.z );
		WRITE_COORD( speed );
		WRITE_SHORT( model );
		WRITE_SHORT( count );
		WRITE_BYTE ( 15 );// 1.5 seconds
	MESSAGE_END();
}
#endif

// called by CWorld::Spawn( );

#ifdef VALVE_DLL
#include "global.h"

void W_Precache(void)
{
	//Master Sword Specific
	// precaches ALL items!
	// Be sure to do this before precaching any monsters
	// Otherwise the monster's weapons won't be available

	//NOV2010_28 - Thothie: Major ancient bugger up fix masked by casual
	//- this was often resulting in wrong map script name, as was not set to lower case!
	char toconv[256];
	 strncpy(toconv,  STRING(gpGlobals->mapname), sizeof(toconv) );
	MSGlobals::MapName = _strlwr(toconv);

	MSGlobalItemInit();
	MSGlobals::NewMap();

	//Precache speech segments
	//CMSMonster::SpeechPrecache( );

	MSGlobals::DevModeEnabled = atoi(CVAR_GET_STRING("ms_dev_mode")) > 0 && !MSGlobals::CentralEnabled ? true : false;

	//Precache dynamic monsters
	CMSMonster::DynamicPrecache();

	//g_sModelIndexWExplosion = PRECACHE_MODEL ("sprites/WXplo1.spr");// underwater fireball
	//g_sModelIndexSmoke = PRECACHE_MODEL ("sprites/steam1.spr");// smoke
	//g_sModelIndexBubbles = PRECACHE_MODEL ("sprites/bubble.spr");//bubbles
	g_sModelIndexBloodSpray = PRECACHE_MODEL("sprites/bloodspray.spr"); // initial blood
	g_sModelIndexBloodDrop = PRECACHE_MODEL("sprites/blood.spr");		// splattered blood

	//g_sModelIndexLaser = PRECACHE_MODEL( (char *)g_pModelNameLaser );
	//g_sModelIndexLaserDot = PRECACHE_MODEL("sprites/laserdot.spr");

	// used by explosions
	//PRECACHE_MODEL ("models/grenade.mdl");
	//PRECACHE_MODEL ("sprites/explode1.spr");

	PRECACHE_SOUND("weapons/debris1.wav"); // explosion aftermaths
	PRECACHE_SOUND("weapons/debris2.wav"); // explosion aftermaths
	PRECACHE_SOUND("weapons/debris3.wav"); // explosion aftermaths

	/*
	PRECACHE_SOUND ("weapons/grenade_hit1.wav");//grenade
	PRECACHE_SOUND ("weapons/grenade_hit2.wav");//grenade
	PRECACHE_SOUND ("weapons/grenade_hit3.wav");//grenade
*/
	PRECACHE_SOUND("weapons/bullet_hit1.wav"); // hit by bullet
	PRECACHE_SOUND("weapons/bullet_hit2.wav"); // hit by bullet

	PRECACHE_SOUND("items/weapondrop1.wav"); // weapon falls to the ground

	//CBaseEntity uses these
	PRECACHE_SOUND("weapons/cbar_hit1.wav");
	PRECACHE_SOUND("weapons/cbar_hit2.wav");

	PRECACHE_SOUND("weapons/cbar_hitbod1.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod2.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod3.wav");
}
#endif

TYPEDESCRIPTION CBasePlayerItem::m_SaveData[] =
	{
		DEFINE_FIELD(CBasePlayerItem, m_pPlayer, FIELD_CLASSPTR),
		DEFINE_FIELD(CBasePlayerItem, m_pNext, FIELD_CLASSPTR),
		//DEFINE_FIELD( CBasePlayerItem, m_fKnown, FIELD_INTEGER ),Reset to zero on load
		DEFINE_FIELD(CBasePlayerItem, m_iId, FIELD_INTEGER),
		// DEFINE_FIELD( CBasePlayerItem, m_iIdPrimary, FIELD_INTEGER ),
		// DEFINE_FIELD( CBasePlayerItem, m_iIdSecondary, FIELD_INTEGER ),
};

#ifndef ISCLIENT
IMPLEMENT_SAVERESTORE(CBasePlayerItem, CBaseAnimating);
#endif

TYPEDESCRIPTION CBasePlayerWeapon::m_SaveData[] =
	{
		DEFINE_FIELD(CBasePlayerWeapon, m_flNextPrimaryAttack, FIELD_TIME),
		DEFINE_FIELD(CBasePlayerWeapon, m_flNextSecondaryAttack, FIELD_TIME),
		DEFINE_FIELD(CBasePlayerWeapon, m_flTimeWeaponIdle, FIELD_TIME),
		DEFINE_FIELD(CBasePlayerWeapon, m_iPrimaryAmmoType, FIELD_INTEGER),
		DEFINE_FIELD(CBasePlayerWeapon, m_iSecondaryAmmoType, FIELD_INTEGER),
		DEFINE_FIELD(CBasePlayerWeapon, m_iClip, FIELD_INTEGER),
		DEFINE_FIELD(CBasePlayerWeapon, m_iDefaultAmmo, FIELD_INTEGER),
		//	DEFINE_FIELD( CBasePlayerWeapon, m_iClientClip, FIELD_INTEGER )	 , reset to zero on load so hud gets updated correctly
		//  DEFINE_FIELD( CBasePlayerWeapon, m_iClientWeaponState, FIELD_INTEGER ), reset to zero on load so hud gets updated correctly
};

#ifndef ISCLIENT
IMPLEMENT_SAVERESTORE(CBasePlayerWeapon, CBasePlayerItem);
#endif

//Before you spawn, PreSpawn
void CBasePlayerItem::PreSpawn(void)
{
	pev->effects |= EF_NODRAW;
}

void CBasePlayerItem ::SetObjectCollisionBox(void)
{
	pev->absmin = pev->origin + Vector(-24, -24, 0);
	pev->absmax = pev->origin + Vector(24, 24, 16);
}

//=========================================================
// Sets up movetype, size, solidtype for a new weapon.
//=========================================================
void CBasePlayerItem ::FallInit(void)
{
#ifdef VALVE_DLL
	ClearBits(pev->effects, EF_NODRAW);
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_BBOX;

	if (WorldModel.len())
		SET_MODEL(edict(), WorldModel);
	pev->absmin = pev->origin - Vector(1, 1, 1);
	pev->absmax = pev->origin + Vector(1, 1, 1);
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0)); //pointsize until it lands on the ground.
	UTIL_SetOrigin(pev, pev->origin);

	ClearBits(pev->flags, FL_ONGROUND);

	pev->nextthink = gpGlobals->time; // + 0.1;
	SetThink(&CBasePlayerItem::FallThink);
#endif
}

//=========================================================
// FallThink - Items that have just spawned run this think
// to catch them when they hit the ground. Once we're sure
// that the object is grounded, we change its solid type
// to trigger and set it in a large box that helps the
// player get it.
//=========================================================
void CBasePlayerItem::FallThink(void)
{
#ifdef VALVE_DLL
	if (pev->flags & FL_ONGROUND)
	{
		//		ALERT( at_console, "\n    ITEM FELL TO GROUND(%s)\n",STRING(pev->classname) );
		//		if ( !FNullEnt( pev->owner ) )
		//		{
		float pitch = 95 + RANDOM_LONG(0, 29);
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "items/weapondrop1.wav", 1, ATTN_NORM, 0, pitch);
		//		}

		// lie flat
		pev->angles.x = 0;
		pev->angles.z = 0;

		Materialize();
	}
	else
		pev->nextthink = gpGlobals->time + 0.1;
#endif
}

//=========================================================
// Materialize - make a CBasePlayerItem visible and tangible
//=========================================================
void CBasePlayerItem::Materialize(void)
{
#ifdef VALVE_DLL
	if (pev->effects & EF_NODRAW)
	{
		// changing from invisible state to visible.
		//		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "items/suitchargeok1.wav", 1, ATTN_NORM, 0, 150 );
		pev->effects &= ~EF_NODRAW;
		pev->effects |= EF_MUZZLEFLASH;
	}

	pev->owner = NULL;
	pev->solid = SOLID_TRIGGER;

	UTIL_SetOrigin(pev, pev->origin); // link into world.
	SetThink(&CBasePlayerItem::ExpireThink);
	pev->nextthink = gpGlobals->time + ExpireTime;
#endif
}
void CBasePlayerItem::ExpireThink(void)
{
	SUB_Remove();
}

//=========================================================
// AttemptToMaterialize - the item is trying to rematerialize,
// should it do so now or wait longer?
//=========================================================
void CBasePlayerItem::AttemptToMaterialize(void)
{
#ifdef VALVE_DLL
	float time = g_pGameRules->FlWeaponTryRespawn(this);

	if (time == 0)
	{
		Materialize();
		return;
	}

	pev->nextthink = gpGlobals->time + time;
#endif
}

//=========================================================
// CheckRespawn - a player is taking this weapon, should
// it respawn?
//=========================================================
void CBasePlayerItem ::CheckRespawn(void)
{
#ifdef VALVE_DLL
	switch (g_pGameRules->WeaponShouldRespawn(this))
	{
	case GR_WEAPON_RESPAWN_YES:
		Respawn();
		break;
	case GR_WEAPON_RESPAWN_NO:
		return;
		break;
	}
#endif
}

//=========================================================
// Respawn- this item is already in the world, but it is
// invisible and intangible. Make it visible and tangible.
//=========================================================
CBaseEntity *CBasePlayerItem::Respawn(void)
{
#ifdef VALVE_DLL
	// make a copy of this weapon that is invisible and inaccessible to players (no touch function). The weapon spawn/respawn code
	// will decide when to make the weapon visible and touchable.
	CBaseEntity *pNewWeapon = CBaseEntity::Create((char *)STRING(pev->classname), g_pGameRules->VecWeaponRespawnSpot(this), pev->angles, pev->owner);

	if (pNewWeapon)
	{
		pNewWeapon->pev->effects |= EF_NODRAW; // invisible for now
		pNewWeapon->SetTouch(NULL);			   // no touch
		pNewWeapon->SetThink(&CBasePlayerItem::AttemptToMaterialize);

		DROP_TO_FLOOR(ENT(pev));

		// not a typo! We want to know when the weapon the player just picked up should respawn! This new entity we created is the replacement,
		// but when it should respawn is based on conditions belonging to the weapon that was taken.
		pNewWeapon->pev->nextthink = g_pGameRules->FlWeaponRespawnTime(this);
	}
	else
	{
		ALERT(at_console, "Respawn failed to create %s!\n", STRING(pev->classname));
	}

	return pNewWeapon;
#endif
	return NULL;
}

/*
	GiveTo - Called whenever an item is transferred to a player from the 
			 ground, a corpse, or the hand or body of another player.
*/
bool CBasePlayerItem::GiveTo(CMSMonster *pReciever, bool fSound)
{
	return TRUE;
}
bool CBasePlayerItem::Deploy(void)
{
	pev->effects &= ~EF_NODRAW;
	return true;
}
void CBasePlayerItem::Attack2ButtonDown(void) {}

BOOL CanAttack(float attack_time, float curtime, BOOL isPredicted)
{
	//#if defined( CLIENT_WEAPONS )
	if (!isPredicted)
	/*#else
	if ( 1 )
#endif*/
	{
		return (attack_time <= curtime) ? TRUE : FALSE;
	}
	else
	{
		return (attack_time <= 0.0) ? TRUE : FALSE;
	}
}
void CBasePlayerItem::ItemPostFrame(void)
{
	startdbg;

	dbg("Begin");

	if (!m_pPlayer)
		return;

	//dbg( "AttackButtonDown" );
	//if( FBitSet(m_pPlayer->pbs.ButtonsDown,IN_ATTACK) )
	//	AttackButtonDown( );
	//else AttackButtonUp( );

	dbg("Attack2ButtonDown");
	if (FBitSet(m_pPlayer->pbs.ButtonsDown, IN_ATTACK2))
		Attack2ButtonDown(); // +attack2
	else
		Attack2ButtonUp();

	dbg("AllButtonsReleased");
	if (!FBitSet(m_pPlayer->pbs.ButtonsDown, IN_ATTACK | IN_ATTACK2))
		AllButtonsReleased(); // no fire buttons down

	dbg("Idle");
	if (ShouldIdle())
		Idle();

	dbg("Think");
#ifdef VALVE_DLL
	if (MSProperties() & ITEM_GENERIC)
		Think();
#endif
	dbg("End");

	enddbg("CBasePlayerItem::ItemPostFrame()");
}

bool CBasePlayerWeapon::ShouldIdle(void)
{
	if (IdleTime < gpGlobals->time &&
		m_flNextPrimaryAttack < gpGlobals->time)
		return true;
	else
		return FALSE;
}
/*void CBasePlayerItem::DestroyItem( void )
{
	// if attached to a player, remove. 
	if ( m_pOwner ) m_pOwner->RemoveItem( this );

	Kill( );
}*/

/*void CBasePlayerItem::Drop( int ParamsFilled, const Vector &Velocity, const Vector &Angles, const Vector &Origin )
{
#ifdef VALVE_DLL
	if( ParamsFilled > 0 ) pev->velocity = Velocity;
	else if( m_pOwner ) pev->velocity = m_pOwner->pev->velocity;
	if( ParamsFilled > 1 ) pev->angles = Angles;
	else if( m_pOwner ) pev->angles = m_pOwner->pev->angles;
	if( ParamsFilled > 2 ) pev->origin = Origin;
	else if( m_pOwner ) pev->origin = m_pOwner->EyePosition( );

//	if( m_pPlayer ) strcpy( m_pPlayer->m_szAnimLegs, "" );
	pev->sequence = 0;
	pev->aiment = NULL;
	//pev->avelocity = Vector( 0,10,0 );
	if( WorldModel.len() ) 
	{
		ClearBits( pev->effects, EF_NODRAW ); //might need to remove this
		SetBits( pev->effects, EF_NOINTERP );
		SET_MODEL(ENT(pev), WorldModel );
	}
#endif

	if( m_pOwner ) m_pOwner->RemoveItem( (CGenericItem *)this );

#ifdef VALVE_DLL
	FallInit( ); //Call FallInit after RemoveItem unsets the think funcrion
#endif
}*/

void CBasePlayerItem::Kill(void)
{
	SetTouch(NULL);
	SetThink(&CBaseEntity::SUB_Remove);
	pev->nextthink = gpGlobals->time + .1;
}

void CBasePlayerItem::Holster()
{
	/*	if( m_pPlayer ) {
		m_pPlayer->pev->viewmodel = 0; 
		m_pPlayer->pev->weaponmodel = 0;
	}*/
}

// CALLED THROUGH the newly-touched weapon's instance. The existing player weapon is pOriginal
int CBasePlayerWeapon::AddDuplicate(CBasePlayerItem *pOriginal)
{
	/*	if ( m_iDefaultAmmo )
	{
		return ExtractAmmo( (CBasePlayerWeapon *)pOriginal );
	}
	else
	{
		// a dead player dropped this.
		return ExtractClipAmmo( (CBasePlayerWeapon *)pOriginal );
	}*/
	return 0;
}

int CBasePlayerWeapon::UpdateClientData(CBasePlayer *pPlayer)
{
	/*	BOOL bSend = FALSE;
	int state = 0;
	if ( pPlayer->m_pActiveItem == this )
	{
		if ( pPlayer->m_fOnTarget )
			state = WEAPON_IS_ONTARGET;
		else
			state = 1;
	}

	// Forcing send of all data!
	if ( !pPlayer->m_fWeapon )
	{
		bSend = TRUE;
	}
	
	// This is the current or last weapon, so the state will need to be updated
	if ( this == pPlayer->m_pActiveItem ||
		 this == pPlayer->m_pClientActiveItem )
	{
		if ( pPlayer->m_pActiveItem != pPlayer->m_pClientActiveItem )
		{
			bSend = TRUE;
		}
	}

	// If the ammo, state, or fov has changed, update the weapon
	if ( m_iClip != m_iClientClip || 
		 state != m_iClientWeaponState || 
		 pPlayer->m_iFOV != pPlayer->m_iClientFOV )
	{
		bSend = TRUE;
	}

	if ( bSend )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgCurWeapon, NULL, pPlayer->pev );
			WRITE_BYTE( state );
			WRITE_BYTE( m_iId );
			WRITE_BYTE( m_iClip );
		MESSAGE_END();

		m_iClientClip = m_iClip;
		m_iClientWeaponState = state;
		pPlayer->m_fWeapon = TRUE;
	}

	if ( m_pNext )
		m_pNext->UpdateClientData( pPlayer );
*/
	return 1;
}

#ifndef VALVE_DLL
void SendViewAnim(CBasePlayer *pPlayer, int iAnim, int iBody)
{
	HUD_SendWeaponAnim(iAnim, iBody, 1);
}
#endif

/*void SetViewModel( const char *pszViewModel )
{
#ifndef VALVE_DLL
	//Always keep the actual view model at null.mdl
	//This is the 'real' HL viewmodel, which is never shown, but always needs a model or it won't hit the renderer
	//What's actually show on the screen are two different models based on what's in your hands
	gEngfuncs.CL_LoadModel( "models/null.mdl", &player.pev->viewmodel );
#endif
}*/

void CBasePlayerWeapon::SendWeaponAnim(int iAnim, int skiplocal)
{
	m_pPlayer->pev->weaponanim = iAnim;

	//#if defined( CLIENT_WEAPONS )
	if (skiplocal && ENGINE_CANSKIP(m_pPlayer->edict()))
		return;
		//#endif

#ifdef VALVE_DLL
	//MP dll ONLY
	MESSAGE_BEGIN(MSG_ONE, SVC_WEAPONANIM, NULL, m_pPlayer->pev);
	WRITE_BYTE(iAnim);	   // sequence number
	WRITE_BYTE(pev->body); // weaponmodel bodygroup.
	MESSAGE_END();
#else
	//Client DLL only
	if (m_pPlayer)
		SendViewAnim(m_pPlayer, iAnim, pev->body);
#endif
}

//=========================================================
// IsUseable - this function determines whether or not a
// weapon is useable by the player in its current state.
// (does it have ammo loaded? do I have any ammo for the
// weapon?, etc)
//=========================================================
BOOL CBasePlayerWeapon ::IsUseable(void)
{
	if (m_iClip <= 0)
	{
		if (m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()] <= 0 && iMaxAmmo1() != -1)
		{
			// clip is empty (or nonexistant) and the player has no more ammo of this type.
			return FALSE;
		}
	}

	return TRUE;
}

bool CBasePlayerWeapon ::CanDeploy(void)
{
	return true; //Master Sword -- You can ALWAYS deploy weapons!!
}

BOOL CBasePlayerWeapon ::DefaultDeploy(char *szViewModel, char *szWeaponModel, int iAnim, char *szAnimExt, int skiplocal /* = 0 */)
{
	/*	if (!CanDeploy( ))
		return FALSE;

	m_pPlayer->pev->viewmodel = MAKE_STRING(szViewModel); //player's WEAPON view model
	m_pPlayer->pev->weaponmodel = MAKE_STRING(szWeaponModel);
	 strncpy(m_pPlayer->m_szAnimExtention,  szAnimExt, sizeof(m_pPlayer->m_szAnimExtention) );
	SendWeaponAnim( iAnim );

	m_pPlayer->m_flNextAttack = gpGlobals->time + 0.5;
	IdleTime = gpGlobals->time + 1.0;
*/
	return TRUE;
}

BOOL CBasePlayerWeapon ::PlayEmptySound(void)
{
	//Master Sword:.. uhh this might not be a gun so don't play this
	return 0;
}

void CBasePlayerWeapon ::ResetEmptySound(void)
{
	m_iPlayEmptySound = 1;
}

//=========================================================
void CBasePlayerWeapon::Holster(int skiplocal /* = 0 */)
{
	m_fInReload = FALSE; // cancel any reload in progress.
	m_pPlayer->pev->viewmodel = 0;
	m_pPlayer->pev->weaponmodel = 0;
}

void CBasePlayerWeapon::PrintState(void)
{
#ifndef VALVE_DLL
	COM_Log("c:\\hl.log", "%.4f ", gpGlobals->time);
	COM_Log("c:\\hl.log", "%.4f ", m_pPlayer->m_flNextAttack);
	COM_Log("c:\\hl.log", "%.4f ", m_flNextPrimaryAttack);
	COM_Log("c:\\hl.log", "%.4f ", m_flTimeWeaponIdle - gpGlobals->time);
	COM_Log("c:\\hl.log", "%i ", m_iClip);
#endif
}

//From Crowbar.cpp, I moved it because now we have more than
//one melee weapon and i can't have it keep redefining it when
//i copy crowbar.cpp to make a new weapon -- Dogg

void FindHullIntersection(const Vector &vecSrc, TraceResult &tr, float *mins, float *maxs, edict_t *pEntity)
{
	int i, j, k;
	float distance;
	float *minmaxs[2] = {mins, maxs};
	TraceResult tmpTrace;
	Vector vecHullEnd = tr.vecEndPos;
	Vector vecEnd;

	distance = 1e6f;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc) * 2);
	UTIL_TraceLine(vecSrc, vecHullEnd, dont_ignore_monsters, pEntity, &tmpTrace);
	if (tmpTrace.flFraction < 1.0)
	{
		tr = tmpTrace;
		return;
	}

	for (i = 0; i < 2; i++)
	{
		for (j = 0; j < 2; j++)
		{
			for (k = 0; k < 2; k++)
			{
				vecEnd.x = vecHullEnd.x + minmaxs[i][0];
				vecEnd.y = vecHullEnd.y + minmaxs[j][1];
				vecEnd.z = vecHullEnd.z + minmaxs[k][2];

				UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, pEntity, &tmpTrace);
				if (tmpTrace.flFraction < 1.0)
				{
					float thisDistance = (tmpTrace.vecEndPos - vecSrc).Length();
					if (thisDistance < distance)
					{
						tr = tmpTrace;
						distance = thisDistance;
					}
				}
			}
		}
	}
}
