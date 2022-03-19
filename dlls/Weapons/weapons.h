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
#ifndef WEAPONS_H
#define WEAPONS_H

#include "Player/player.h"

extern int gmsgWeapPickup;

//Master Sword
#define MakeAnim(ref) strncpy(m_pOwner->m_szAnimExtention, ref, 32)
#define MakeAnimLR(ref) strncpy(m_pOwner->m_szAnimExtention, UTIL_VarArgs("%s_%s", ref, (m_pOwner->iCurrentHand == 0) ? "L" : "R"), 32)

#ifdef VALVE_DLL
void G_SolidifyEnts(bool fEnable, bool fSolidShields, bool fEnableCorpses, bool fEnlargeboxes);

#define MSTRACE_SOLIDSHIELDS (1 << 0)
#define MSTRACE_LARGEHITBOXES (1 << 1)
#define MSTRACE_HITCORPSES (1 << 2)

void MSTraceLine(const Vector &vecSrc, const Vector &vecEnd, IGNORE_MONSTERS igmon, edict_t *pentIgnore, TraceResult &tr, int flags);

struct hitent_t
{
	float Dist;
	CBaseEntity *pEntity;
	hitent_t() {}
	hitent_t(float dist, CBaseEntity *pentity)
	{
		Dist = dist;
		pEntity = pentity;
	}
};
class hitent_list : public mslist<hitent_t>
{
public:
	CBaseEntity *ClosestHit()
	{
		CBaseEntity *pClosest = NULL;
		float ClosestDist = 9999999;
		for (int h = 0; h < size(); h++)
		{
			hitent_t &Hit = operator[](h);
			if (Hit.Dist < ClosestDist)
			{
				pClosest = Hit.pEntity;
				ClosestDist = Hit.Dist;
			}
		}
		return pClosest;
	}
};

void DoDamage(damage_t &Damage, hitent_list &HitList /* Out */);
#endif

//------------

// Contact Grenade / Timed grenade / Satchel Charge
class CGrenade : public CBaseMonster
{
public:
	void Spawn(void);

	typedef enum
	{
		SATCHEL_DETONATE = 0,
		SATCHEL_RELEASE
	} SATCHELCODE;

	static CGrenade *ShootTimed(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time);
	static CGrenade *ShootContact(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity);
	static CGrenade *ShootSatchelCharge(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity);
	static void UseSatchelCharges(entvars_t *pevOwner, SATCHELCODE code);

	void Explode(Vector vecSrc, Vector vecAim);
	void Explode(TraceResult *pTrace, int bitsDamageType);
	void EXPORT Smoke(void);

	void EXPORT BounceTouch(CBaseEntity *pOther);
	void EXPORT SlideTouch(CBaseEntity *pOther);
	void EXPORT ExplodeTouch(CBaseEntity *pOther);
	void EXPORT DangerSoundThink(void);
	void EXPORT PreDetonate(void);
	void EXPORT Detonate(void);
	void EXPORT DetonateUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void EXPORT TumbleThink(void);

	virtual void BounceSound(void);
	virtual int BloodColor(void) { return DONT_BLEED; }
	virtual void Killed(entvars_t *pevAttacker, int iGib);

	BOOL m_fRegisteredSound; // whether or not this grenade has issued its DANGER sound to the world sound list yet.
};

// constant items
#define ITEM_HEALTHKIT 1
#define ITEM_ANTIDOTE 2
#define ITEM_SECURITY 3
#define ITEM_BATTERY 4

#define WEAPON_NONE 0
#define WEAPON_CROWBAR 1
#define WEAPON_GLOCK 2
#define WEAPON_PYTHON 3
#define WEAPON_MP5 4
#define WEAPON_CHAINGUN 5
#define WEAPON_CROSSBOW 6
#define WEAPON_SHOTGUN 7
#define WEAPON_RPG 8
#define WEAPON_GAUSS 9
#define WEAPON_EGON 10
#define WEAPON_HORNETGUN 11
#define WEAPON_HANDGRENADE 12
#define WEAPON_TRIPMINE 13
#define WEAPON_SATCHEL 14
#define WEAPON_SNARK 15

#define WEAPON_ALLWEAPONS (~(1 << WEAPON_SUIT))

#define WEAPON_SUIT 31 // ?????

#define MAX_WEAPONS 32

#define MAX_NORMAL_BATTERY 100

// the maximum amount of ammo each weapon's clip can hold
#define WEAPON_NOCLIP -1

// bullet types
typedef enum
{
	BULLET_NONE = 0,
	BULLET_PLAYER_9MM,		// glock
	BULLET_PLAYER_MP5,		// mp5
	BULLET_PLAYER_357,		// python
	BULLET_PLAYER_BUCKSHOT, // shotgun
	BULLET_PLAYER_CROWBAR,	// crowbar swipe

	BULLET_MONSTER_9MM,
	BULLET_MONSTER_MP5,
	BULLET_MONSTER_12MM,
} Bullet;

#define ITEM_FLAG_SELECTONEMPTY 1
#define ITEM_FLAG_NOAUTORELOAD 2
#define ITEM_FLAG_NOAUTOSWITCHEMPTY 4
#define ITEM_FLAG_LIMITINWORLD 8
#define ITEM_FLAG_EXHAUSTIBLE 16 // A player can totally exhaust their ammo supply and lose this weapon

#define WEAPON_IS_ONTARGET 0x40

typedef struct
{
	int iSlot;
	int iPosition;
	const char *pszAmmo1; // ammo 1 type
	int iMaxAmmo1;		  // max ammo 1
	const char *pszAmmo2; // ammo 2 type
	int iMaxAmmo2;		  // max ammo 2
	const char *pszName;
	int iMaxClip;
	int iId;
	int iFlags;
	int iWeight; // this value used to determine this weapon's importance in autoselection.
} ItemInfo;

typedef struct
{
	const char *pszName;
	int iId;
} AmmoInfo;

// Items that the player has in their inventory that they can use
class CBasePlayerItem : public CBaseAnimating
{
public:
	//Master Sword
	CBasePlayer *m_pPlayer;
	CMSMonster *m_pOwner;
	ulong m_iId;
	uint m_Value; //Agreed value of this item (in gold)
	bool m_NotUseable;
	int Properties, iViewModel;
	int SpriteFrame;	 //Shuriken FEB2008a
	string_i ItemName,	 //MS classname (i.e "dagger", "broadwsord")
		WorldModel,		 //Sets this model automatically on drop & fall
		HandSpriteName,	 //Name of the hand sprite
		TradeSpriteName; //Name of the trade/barter sprite
	float ExpireTime, IdleTime, fNextActionTime;

	BOOL Wielded; //Yes even items can be wielded
	virtual void PreSpawn(void);
	bool IsMSItem() { return true; }
	virtual int MSProperties() { return Properties; }
	virtual bool CanDrop() { return true; }
	//virtual void Drop( int ParamsFilled = 0, const Vector &Velocity = g_vecZero, const Vector &Angles = g_vecZero, const Vector &Origin = g_vecZero );
	virtual void ExpireThink(void);
	//virtual void AttackButtonDown( void ) { } // "+ATTACK"
	//virtual void AttackButtonUp( void ) { }
	virtual void Attack2ButtonDown(void); // "+ATTACK2"
	virtual void Attack2ButtonUp(void) {}
	virtual void AllButtonsReleased(void) {}
	virtual void Idle(void) {}
	virtual bool ShouldIdle(void) { return true; }
	virtual void Wield(void) {}
	virtual void UnWield(void) {}
	virtual bool GiveTo(CMSMonster *pReciever, bool fSound = true);
	virtual void ItemPostFrame(void); // called each frame by the player PostThink
	virtual void Materialize(void);	  // make a weapon visible and tangible
	virtual void FallInit(void);
	virtual void FallThink(void); // when an item is first spawned, this think is run to determine when the object has hit the ground.
	//----------------------------

	virtual void SetObjectCollisionBox(void);

#ifdef VALVE_DLL
	virtual int Save(CSave &save);
	virtual int Restore(CRestore &restore);
#endif

	static TYPEDESCRIPTION m_SaveData[];

	virtual int AddDuplicate(CBasePlayerItem *pItem) { return FALSE; } // return TRUE if you want your duplicate removed from world
	//void EXPORT DestroyItem( void );
	void EXPORT AttemptToMaterialize(void); // the weapon desires to become visible and tangible, if the game rules allow for it
	CBaseEntity *Respawn(void);				// copy a weapon
	void CheckRespawn(void);
	virtual int GetItemInfo(ItemInfo *p) { return 0; }; // returns 0 if struct not filled out
	virtual bool CanDeploy(void) { return true; };
	virtual bool Deploy(); // returns is deploy was successful

	virtual bool CanHolster(void) { return true; }; // can this weapon be put away right now?
	virtual void Holster();
	virtual void UpdateItemInfo(void) { return; };

	virtual void ItemPreFrame(void) { return; } // called each frame by the player PreThink

	virtual void Kill(void);

	virtual int PrimaryAmmoIndex() { return -1; };
	virtual int SecondaryAmmoIndex() { return -1; };

	virtual int UpdateClientData(CBasePlayer *pPlayer) { return 0; }

	virtual CBasePlayerItem *GetWeaponPtr(void) { return NULL; };

	static ItemInfo ItemInfoArray[MAX_WEAPONS];

	CBasePlayerItem *m_pNext;

	virtual int iItemSlot(void) { return 0; } // return 0 to MAX_ITEMS_SLOTS, used in hud

	int iItemPosition(void) { return ItemInfoArray[m_iId].iPosition; }
	const char *pszAmmo1(void) { return ItemInfoArray[m_iId].pszAmmo1; }
	int iMaxAmmo1(void) { return ItemInfoArray[m_iId].iMaxAmmo1; }
	const char *pszAmmo2(void) { return ItemInfoArray[m_iId].pszAmmo2; }
	int iMaxAmmo2(void) { return ItemInfoArray[m_iId].iMaxAmmo2; }
	const char *pszName(void) { return ItemInfoArray[m_iId].pszName; }
	int iMaxClip(void) { return ItemInfoArray[m_iId].iMaxClip; }
	int iWeight(void) { return ItemInfoArray[m_iId].iWeight; }
	int iFlags(void) { return ItemInfoArray[m_iId].iFlags; }
};

// inventory items that
class CBasePlayerWeapon : public CBasePlayerItem
{
public:
	//Master Sword Variables/Functions

	virtual bool ShouldIdle(void);
	virtual void Idle(void){};
	virtual void FinishReload(void) { return; }
	// ----------------

	virtual int Save(CSave &save);
	virtual int Restore(CRestore &restore);

	static TYPEDESCRIPTION m_SaveData[];

	// generic weapon versions of CBasePlayerItem calls
	virtual int AddDuplicate(CBasePlayerItem *pItem);

	virtual void UpdateItemInfo(void){}; // updates HUD state

	int m_iPlayEmptySound;
	int m_fFireOnEmpty; // True when the gun is empty and the player is still holding down the
						// attack key(s)
	virtual BOOL PlayEmptySound(void);
	virtual void ResetEmptySound(void);

	virtual void SendWeaponAnim(int iAnim, int skiplocal = 0); // skiplocal is 1 if client is predicting weapon animations

	virtual bool CanDeploy(void);
	virtual BOOL IsUseable(void);
	BOOL DefaultDeploy(char *szViewModel, char *szWeaponModel, int iAnim, char *szAnimExt, int skiplocal = 0);
	int DefaultReload(int iClipSize, int iAnim, float fDelay);

	virtual void Reload(void) { return; }				// do "+RELOAD"
	virtual int UpdateClientData(CBasePlayer *pPlayer); // sends hud info to client dll, if things have changed
	virtual void Holster(int skiplocal = 0);
	virtual BOOL UseDecrement(void) { return FALSE; };

	void PrintState(void);

	virtual CBasePlayerItem *GetWeaponPtr(void) { return (CBasePlayerItem *)this; };

	float m_flNextPrimaryAttack;   // soonest time ItemPostFrame will call PrimaryAttack
	float m_flNextSecondaryAttack; // soonest time ItemPostFrame will call SecondaryAttack
	float m_flTimeWeaponIdle;	   // soonest time ItemPostFrame will call WeaponIdle
	int m_iPrimaryAmmoType;		   // "primary" ammo index into players m_rgAmmo[]
	int m_iSecondaryAmmoType;	   // "secondary" ammo index into players m_rgAmmo[]
	int m_iClip;				   // number of shots left in the primary weapon clip, -1 it not used
	int m_iClientClip;			   // the last version of m_iClip sent to hud dll
	int m_iClientWeaponState;	   // the last version of the weapon state sent to hud dll (is current weapon, is on target)
	int m_fInReload;			   // Are we in the middle of a reload;

	int m_iDefaultAmmo; // how much ammo you get when you pick up this weapon as placed by a level designer.
};

class CBasePlayerAmmo : public CBaseEntity
{
public:
	/*	virtual void Spawn( void );
	virtual BOOL AddAmmo( CBaseEntity *pOther ) { return TRUE; };

	CBaseEntity* Respawn( void );
	void EXPORT Materialize( void );*/
};

//extern DLL_GLOBAL	short	g_sModelIndexLaser;// holds the index for the laser beam
//extern DLL_GLOBAL	const char *g_pModelNameLaser;

//extern DLL_GLOBAL	short	g_sModelIndexLaserDot;// holds the index for the laser beam dot
extern DLL_GLOBAL short g_sModelIndexExplosion;	 // holds the index for the fireball
extern DLL_GLOBAL short g_sModelIndexSmoke;		 // holds the index for the smoke cloud
extern DLL_GLOBAL short g_sModelIndexWExplosion; // holds the index for the underwater explosion
extern DLL_GLOBAL short g_sModelIndexBubbles;	 // holds the index for the bubbles model
extern DLL_GLOBAL short g_sModelIndexBloodDrop;	 // holds the sprite index for blood drops
extern DLL_GLOBAL short g_sModelIndexBloodSpray; // holds the sprite index for blood spray (bigger)

extern void ClearMultiDamage(void);
extern void ApplyMultiDamage(entvars_t *pevInflictor, entvars_t *pevAttacker);
extern void AddMultiDamage(entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType);

extern void DecalGunshot(TraceResult *pTrace, int iBulletType);
extern void SpawnBlood(Vector vecSpot, int bloodColor, float flDamage);
extern int DamageDecal(CBaseEntity *pEntity, int bitsDamageType);
extern void RadiusDamage(Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType);

typedef struct
{
	CBaseEntity *pEntity;
	float amount;
	int type;
} MULTIDAMAGE;

extern MULTIDAMAGE gMultiDamage;

#define LOUD_GUN_VOLUME 1000
#define NORMAL_GUN_VOLUME 600
#define QUIET_GUN_VOLUME 200

#define BRIGHT_GUN_FLASH 512
#define NORMAL_GUN_FLASH 256
#define DIM_GUN_FLASH 128

#define BIG_EXPLOSION_VOLUME 2048
#define NORMAL_EXPLOSION_VOLUME 1024
#define SMALL_EXPLOSION_VOLUME 512

#define WEAPON_ACTIVITY_VOLUME 64

#define VECTOR_CONE_1DEGREES Vector(0.00873, 0.00873, 0.00873)
#define VECTOR_CONE_2DEGREES Vector(0.01745, 0.01745, 0.01745)
#define VECTOR_CONE_3DEGREES Vector(0.02618, 0.02618, 0.02618)
#define VECTOR_CONE_4DEGREES Vector(0.03490, 0.03490, 0.03490)
#define VECTOR_CONE_5DEGREES Vector(0.04362, 0.04362, 0.04362)
#define VECTOR_CONE_6DEGREES Vector(0.05234, 0.05234, 0.05234)
#define VECTOR_CONE_7DEGREES Vector(0.06105, 0.06105, 0.06105)
#define VECTOR_CONE_8DEGREES Vector(0.06976, 0.06976, 0.06976)
#define VECTOR_CONE_9DEGREES Vector(0.07846, 0.07846, 0.07846)
#define VECTOR_CONE_10DEGREES Vector(0.08716, 0.08716, 0.08716)
#define VECTOR_CONE_15DEGREES Vector(0.13053, 0.13053, 0.13053)
#define VECTOR_CONE_20DEGREES Vector(0.17365, 0.17365, 0.17365)

#endif // WEAPONS_H
