/***
*
*	Copyright (c) 1999, 2000 Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
#ifndef MONSTERS_H
#include "skill.h"
#define MONSTERS_H

/*

===== monsters.h ========================================================

  Header file for monster-related utility code

*/

// CHECKLOCALMOVE result types

enum localmove_e {
	LOCALMOVE_INVALID = 0,					 // move is not possible
	LOCALMOVE_INVALID_DONT_TRIANGULATE = 1,	 // move is not possible, don't try to triangulate
	LOCALMOVE_VALID = 2						 // move is possible
};

enum hitgroup_e {
	// Hit Group standards

	HITGROUP_GENERIC = 0,
	HITGROUP_HEAD = 1,
	HITGROUP_CHEST = 2,
	HITGROUP_STOMACH = 3,
	HITGROUP_LEFTARM = 4,
	HITGROUP_RIGHTARM = 5,
	HITGROUP_LEFTLEG = 6,
	HITGROUP_RIGHTLEG = 7
};

enum monster_sf_e {
	// Monster Spawnflags
	 SF_MONSTER_WAIT_TILL_SEEN = 1, // spawnflag that makes monsters wait until player can see them before attacking.
	 SF_MONSTER_GAG = 2,			// no idle noises from this monster
	 SF_MONSTER_HITMONSTERCLIP = 4,
//								8
	SF_MONSTER_PRISONER = 16, // monster won't attack anyone, no one will attacke him.
//										32
//										64
	SF_MONSTER_WAIT_FOR_SCRIPT = 128, //spawnflag that makes monsters wait to check for attacking until the script is done or they've been attacked
	SF_MONSTER_PREDISASTER = 256,	   //this is a predisaster scientist or barney. Influences how they speak.
	SF_MONSTER_FADECORPSE = 512,	   // Fade out corpse after death
	SF_MONSTER_FALL_TO_GROUND = 0x80000000,

// specialty spawnflags

	SF_MONSTER_TURRET_AUTOACTIVATE = 32,
	SF_MONSTER_TURRET_STARTINACTIVE = 64,
	SF_MONSTER_WAIT_UNTIL_PROVOKED = 64 // don't attack the player unless provoked
};

enum move_origin_e {
	// MoveToOrigin stuff
	MOVE_NORMAL = 0 ,// normal move in the direction monster is facing
	MOVE_STRAFE = 1 ,// moves in direction specified, no matter which way monster is facing
	MOVE_STUCK_DIST = 32,		// if a monster can't step this far, it is stuck.
	MOVE_START_TURN_DIST = 64 // when this far away from moveGoal, start turning to face next goal
};

// spawn flags 256 and above are already taken by the engine
extern void UTIL_MoveToOrigin(edict_t *pent, const Vector &vecGoal, float flDist, int iMoveType);

Vector VecCheckToss(entvars_t *pev, const Vector &vecSpot1, Vector vecSpot2, float flGravityAdj = 1.0);
Vector VecCheckThrow(entvars_t *pev, const Vector &vecSpot1, Vector vecSpot2, float flSpeed, float flGravityAdj = 1.0);
extern DLL_GLOBAL Vector g_vecAttackDir;
extern DLL_GLOBAL float g_flMeleeRange;
extern DLL_GLOBAL float g_flMediumRange;
extern DLL_GLOBAL float g_flLongRange;
extern void EjectBrass(const Vector &vecOrigin, const Vector &vecVelocity, float rotation, int model, int soundtype);
extern void ExplodeModel(const Vector &vecOrigin, float speed, int model, int count);

BOOL FBoxVisible(entvars_t *pevLooker, entvars_t *pevTarget);
BOOL FBoxVisible(entvars_t *pevLooker, entvars_t *pevTarget, Vector &vecTargetOrigin, float flSize = 0.0);

// monster to monster relationship types

enum monster_relationship_e {
	R_AL = -2,	// (ALLY) pals. Good alternative to R_NO when applicable.
	R_FR = -1,	// (FEAR)will run
	R_NO = 0,	// (NO RELATIONSHIP) disregard
	R_DL = 1,	// (DISLIKE) will attack
	R_HT = 2,	// (HATE)will attack this character instead of any visible DISLIKEd characters
	R_NM = 3	// (NEMESIS)  A monster Will ALWAYS attack its nemsis, no matter what
};

// these bits represent the monster's memory

enum monster_bits_e {

	MEMORY_CLEAR = 0,
	bits_MEMORY_PROVOKED = (1 << 0),	  // right now only used for houndeyes.
	bits_MEMORY_INCOVER = (1 << 1),		  // monster knows it is in a covered position.
	bits_MEMORY_SUSPICIOUS = (1 << 2),	  // Ally is suspicious of the player, and will move to provoked more easily
	bits_MEMORY_PATH_FINISHED = (1 << 3), // Finished monster path (just used by big momma for now)
	bits_MEMORY_ON_PATH = (1 << 4),		  // Moving on a path
	bits_MEMORY_MOVE_FAILED = (1 << 5),	  // Movement has already failed
	bits_MEMORY_FLINCHED = (1 << 6),	  // Has already flinched
	bits_MEMORY_KILLED = (1 << 7),		  // HACKHACK -- remember that I've already called my Killed()
	bits_MEMORY_CUSTOM4 = (1 << 28),	  // Monster-specific memory
	bits_MEMORY_CUSTOM3 = (1 << 29),	  // Monster-specific memory
	bits_MEMORY_CUSTOM2 = (1 << 30),	  // Monster-specific memory
	bits_MEMORY_CUSTOM1 = (1 << 31),	  // Monster-specific memory

};
// trigger conditions for scripted AI
// these MUST match the CHOICES interface in halflife.fgd for the base monster
enum aitrigger_e
{
	AITRIGGER_NONE = 0,
	AITRIGGER_SEEPLAYER_ANGRY_AT_PLAYER,
	AITRIGGER_TAKEDAMAGE,
	AITRIGGER_HALFHEALTH,
	AITRIGGER_DEATH,
	AITRIGGER_SQUADMEMBERDIE,
	AITRIGGER_SQUADLEADERDIE,
	AITRIGGER_HEARWORLD,
	AITRIGGER_HEARPLAYER,
	AITRIGGER_HEARCOMBAT,
	AITRIGGER_SEEPLAYER_UNCONDITIONAL,
	AITRIGGER_SEEPLAYER_NOT_IN_COMBAT,
};
/*
		0 : "No Trigger"
		1 : "See Player"
		2 : "Take Damage"
		3 : "50% Health Remaining"
		4 : "Death"
		5 : "Squad Member Dead"
		6 : "Squad Leader Dead"
		7 : "Hear World"
		8 : "Hear Player"
		9 : "Hear Combat"
*/

//
// A gib is a chunk of a body, or a piece of wood/metal/rocks/etc.
//
class CGib : public CBaseEntity
{
public:
	void Spawn(const char *szGibModel);
	void BounceGibTouch(CBaseEntity *pOther);
	void StickyGibTouch(CBaseEntity *pOther);
	void WaitTillLand(void);
	void LimitVelocity(void);

	virtual int ObjectCaps(void) { return (CBaseEntity ::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DONT_SAVE; }
	static void SpawnHeadGib(entvars_t *pevVictim);
	static void SpawnRandomGibs(entvars_t *pevVictim, int cGibs, int human);
	static void SpawnStickyGibs(entvars_t *pevVictim, Vector vecOrigin, int cGibs);

	int m_bloodColor;
	int m_cBloodDecals;
	int m_material;
	float m_lifeTime;
};

#define CUSTOM_SCHEDULES                                     \
	virtual Schedule_t *ScheduleFromName(const char *pName); \
	static Schedule_t *m_scheduleList[];

#define DEFINE_CUSTOM_SCHEDULES(derivedClass) \
	Schedule_t *derivedClass::m_scheduleList[] =

#define IMPLEMENT_CUSTOM_SCHEDULES(derivedClass, baseClass)                                       \
	Schedule_t *derivedClass::ScheduleFromName(const char *pName)                                 \
	{                                                                                             \
		Schedule_t *pSchedule = ScheduleInList(pName, m_scheduleList, std::size(m_scheduleList)); \
		if (!pSchedule)                                                                           \
			return baseClass::ScheduleFromName(pName);                                            \
		return pSchedule;                                                                         \
	}

#endif //MONSTERS_H
