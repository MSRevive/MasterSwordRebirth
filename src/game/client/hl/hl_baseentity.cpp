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

/*
==========================
This file contains "stubs" of class member implementations so that we can predict certain
 weapons client side.  From time to time you might find that you need to implement part of the
 these functions.  If so, cut it from here, paste it in hl_weapons.cpp or somewhere else and
 add in the functionality you need.
==========================
*/
#include "../hud.h"

#include "msdllheaders.h"
#include "player/player.h"
#include "weapons/weapons.h"
#include "nodes.h"
#include "ms/clglobal.h"

#include "cl_entity.h"
#include "event_api.h"
#include "mslogger.h"

// Globals used by client.dll
const Vector g_vecZero = Vector(0, 0, 0);
int gmsgWeapPickup = 0;
enginefuncs_t g_engfuncs;
globalvars_t *gpGlobals;
constexpr int NEG_REF = -1;

//-----------

void EMIT_SOUND_DYN(edict_t *entity, int channel, const char *sample, float volume, float attenuation, int flags, float pitch)
{

	entvars_t *pev = (entvars_t *)entity;

	//	Print( "Sample: %s, %i\n", (char *)sample, pev->iuser1 );
	//	gEngfuncs.pfnPlaySoundByNameAtLocation( (char *)sample, volume, (float *)&pEntity->pev->origin );
	gEngfuncs.pEventAPI->EV_PlaySound(pev->iuser1, (float *)pev->origin, channel, sample, volume, attenuation, flags, pitch);
}

// CBaseEntity Stubs
int CBaseEntity ::TakeHealth(float flHealth, int bitsDamageType) { return NULL; }
int CBaseEntity ::TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) { return NULL; }
CBaseEntity *CBaseEntity::GetNextTarget(void) { return NULL; }
int CBaseEntity::Save(CSave &save) { return NULL; }
int CBaseEntity::Restore(CRestore &restore) { return NULL; }
void CBaseEntity::SetObjectCollisionBox(void) {}
int CBaseEntity ::Intersects(CBaseEntity *pOther) { return NULL; }
void CBaseEntity ::MakeDormant(void) {}
int CBaseEntity ::IsDormant(void) { return NULL; }
BOOL CBaseEntity ::IsInWorld(void) { return true; }
int CBaseEntity::ShouldToggle(USE_TYPE useType, BOOL currentState) { return NULL; }
int CBaseEntity ::DamageDecal(int bitsDamageType) { return NEG_REF; }
CBaseEntity *CBaseEntity::Create(const char *szName, const Vector &vecOrigin, const Vector &vecAngles, edict_t *pentOwner) { return NULL; }
void CBaseEntity::SUB_Remove(void)
{
	Deactivate();
	MS_DEBUG("DELETE ITEM: %s (%i)", DisplayName(), IsMSItem() ? ((CBasePlayerItem*)this)->m_iId : NULL);
	MSCLGlobals::RemoveEnt(this, true);
}
//void CBaseEntity::StruckSound( CBaseEntity *pInflicter, CBaseEntity *pAttacker, float flDamage, TraceResult *ptr, int bitsDamageType ) { }
void CBaseEntity::CounterEffect(CBaseEntity *pInflictor, int iEffect, void *pExtraData) {}

// CBaseDelay Stubs
void CBaseDelay ::KeyValue(struct KeyValueData_s *) {}
int CBaseDelay::Restore(class CRestore &) { return true; }
int CBaseDelay::Save(class CSave &) { return true; }
CBaseEntity *CBaseDelay::GetSelf() { return this; }

// CBaseAnimating Stubs
int CBaseAnimating::Restore(class CRestore &) { return true; }
int CBaseAnimating::Save(class CSave &) { return true; }

// DEBUG Stubs
edict_t *DBG_EntOfVars(const entvars_t *pev) { return NULL; }
void DBG_AssertFunction(BOOL fExpr, const char *szExpr, const char *szFile, int szLine, const char *szMessage) {}

// UTIL_* Stubs
void UTIL_PrecacheOther(const char *szClassname) {}
void UTIL_BloodDrips(const Vector &origin, const Vector &direction, int color, int amount) {}
void UTIL_DecalTrace(TraceResult *pTrace, int decalNumber) {}
void UTIL_GunshotDecalTrace(TraceResult *pTrace, int decalNumber) {}
void UTIL_MakeVectors(const Vector &vecAngles) {}
BOOL UTIL_IsValidEntity(edict_t *pent) { return true; }
void UTIL_SetOrigin(entvars_t *, const Vector &org) {}
BOOL UTIL_GetNextBestWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon) { return true; }
void UTIL_LogPrintf(const char *, ...) {}
void UTIL_ClientPrintAll(int, char const *, char const *, char const *, char const *, char const *) {}
void PrintNotifyMsg(char *pszmsg);
void ClientPrint(entvars_t *client, int msg_dest, const char *msg_name, const char *param1, const char *param2, const char *param3, const char *param4)
{
	PrintNotifyMsg((char *)msg_name);
	//gEngfuncs.pfnConsolePrint( (char *)msg_name );
}

// CBaseToggle Stubs
int CBaseToggle::Restore(class CRestore &) { return true; }
int CBaseToggle::Save(class CSave &) { return true; }
void CBaseToggle ::KeyValue(struct KeyValueData_s *) {}

// CGrenade Stubs
void CGrenade::BounceSound(void) {}
void CGrenade::Explode(Vector, Vector) {}
void CGrenade::Explode(TraceResult *, int) {}
void CGrenade::Killed(entvars_t *, int) {}
void CGrenade::Spawn(void) {}

CBaseEntity *CBaseMonster ::CheckTraceHullAttack(float flDist, int iDamage, int iDmgType) { return NULL; }
void CBaseMonster ::Eat(float flFullDuration) {}
BOOL CBaseMonster ::FShouldEat(void) { return true; }
void CBaseMonster ::BarnacleVictimBitten(entvars_t *pevBarnacle) {}
void CBaseMonster ::BarnacleVictimReleased(void) {}
void CBaseMonster ::Listen(void) {}
float CBaseMonster::FLSoundVolume(CSound* pSound) { return NULL; }
BOOL CBaseMonster ::FValidateHintType(short sHint) { return false; }
void CBaseMonster ::Look(int iDistance) {}
int CBaseMonster ::ISoundMask(void) { return NULL; }
CSound *CBaseMonster ::PBestSound(void) { return NULL; }
CSound *CBaseMonster ::PBestScent(void) { return NULL; }
float CBaseAnimating ::StudioFrameAdvance(float flInterval) { return NULL; }
void CBaseMonster ::MonsterThink(void) {}
void CBaseMonster ::MonsterUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value) {}
int CBaseMonster ::IgnoreConditions(void) { return NULL; }
void CBaseMonster ::RouteClear(void) {}
void CBaseMonster ::RouteNew(void) {}
BOOL CBaseMonster ::FRouteClear(void) { return false; }
BOOL CBaseMonster ::FRefreshRoute(void) { return false; }
BOOL CBaseMonster::MoveToEnemy(Activity movementAct, float waitTime) { return false; }
BOOL CBaseMonster::MoveToLocation(Activity movementAct, float waitTime, const Vector &goal) { return false; }
BOOL CBaseMonster::MoveToTarget(Activity movementAct, float waitTime) { return false; }
BOOL CBaseMonster::MoveToNode(Activity movementAct, float waitTime, const Vector &goal) { return false; }
int ShouldSimplify(int routeType) { return true; }
void CBaseMonster ::RouteSimplify(CBaseEntity *pTargetEnt) {}
BOOL CBaseMonster ::FBecomeProne(void) { return true; }
BOOL CBaseMonster ::CheckRangeAttack1(float flDot, float flDist) { return false; }
BOOL CBaseMonster ::CheckRangeAttack2(float flDot, float flDist) { return false; }
BOOL CBaseMonster ::CheckMeleeAttack1(float flDot, float flDist) { return false; }
BOOL CBaseMonster ::CheckMeleeAttack2(float flDot, float flDist) { return false; }
void CBaseMonster ::CheckAttacks(CBaseEntity *pTarget, float flDist) {}
BOOL CBaseMonster ::FCanCheckAttacks(void) { return false; }
int CBaseMonster ::CheckEnemy(CBaseEntity *pEnemy) { return false; }
void CBaseMonster ::PushEnemy(CBaseEntity *pEnemy, Vector &vecLastKnownPos) {}
BOOL CBaseMonster ::PopEnemy() { return false; }
void CBaseMonster ::SetActivity(Activity NewActivity) {}
void CBaseMonster ::SetSequenceByName(char *szSequence) {}
int CBaseMonster ::CheckLocalMove(const Vector &vecStart, const Vector &vecEnd, CBaseEntity *pTarget, float *pflDist) { return NULL; }
float CBaseMonster::OpenDoorAndWait(entvars_t* pevDoor) { return NULL; }
void CBaseMonster ::AdvanceRoute(float distance) {}
int CBaseMonster::RouteClassify(int iMoveFlag) { return NULL; }
BOOL CBaseMonster ::BuildRoute(const Vector &vecGoal, int iMoveFlag, CBaseEntity *pTarget) { return false; }
void CBaseMonster ::InsertWaypoint(Vector vecLocation, int afMoveFlags) {}
BOOL CBaseMonster ::FTriangulate(const Vector &vecStart, const Vector &vecEnd, float flDist, CBaseEntity *pTargetEnt, Vector *pApex) { return false; }
void CBaseMonster ::Move(float flInterval) {}
BOOL CBaseMonster::ShouldAdvanceRoute(float flWaypointDist) { return false; }
void CBaseMonster::MoveExecute(CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval) {}
void CBaseMonster ::MonsterInit(void) {}
void CBaseMonster ::MonsterInitThink(void) {}
void CBaseMonster ::StartMonster(void) {}
void CBaseMonster ::MovementComplete(void) {}
int CBaseMonster::TaskIsRunning(void) { return false; }
int CBaseMonster::IRelationship(CBaseEntity *pTarget) { return NULL; }
BOOL CBaseMonster ::FindCover(Vector vecThreat, Vector vecViewOffset, float flMinDist, float flMaxDist) { return false; }
BOOL CBaseMonster ::BuildNearestRoute(Vector vecThreat, Vector vecViewOffset, float flMinDist, float flMaxDist) { return false; }
CBaseEntity *CBaseMonster ::BestVisibleEnemy(void) { return NULL; }
bool CBaseMonster ::FInViewCone(CBaseEntity *pEntity) { return false; }
bool CBaseMonster ::FInViewCone(Vector &Origin) { return false; }
BOOL CBaseEntity ::FVisible(CBaseEntity *pEntity) { return false; }
BOOL CBaseEntity ::FVisible(const Vector &vecOrigin) { return false; }
BOOL CBaseEntity ::FMVisible(CBaseEntity *pEntity) { return false; }
BOOL CBaseEntity ::FMVisible(const Vector &vecOrigin) { return false; }
void CBaseMonster ::MakeIdealYaw(Vector vecTarget) {}
float CBaseMonster::FlYawDiff(void) { return NULL; }
float CBaseMonster::ChangeYaw(int yawSpeed) { return NULL; }
float CBaseMonster::VecToYaw(Vector vecDir) { return NULL; }
int CBaseAnimating ::LookupActivity(int activity) { return NULL; }
int CBaseAnimating ::LookupActivityHeaviest(int activity) { return NULL; }
void CBaseMonster ::SetEyePosition(void) {}
int CBaseAnimating ::LookupSequence(const char *label) { return NULL; }
void CBaseAnimating ::ResetSequenceInfo() {}
BOOL CBaseAnimating ::GetSequenceFlags() { return false; }
void CBaseAnimating ::DispatchAnimEvents(float flInterval) {}
void CBaseMonster ::HandleAnimEvent(MonsterEvent_t *pEvent) {}
float CBaseAnimating::SetBoneController(int iController, float flValue) { return NULL; }
void CBaseAnimating ::InitBoneControllers(void) {}
float CBaseAnimating ::SetBlending(int iBlender, float flValue) { return NULL; }
void CBaseAnimating ::GetBonePosition(int iBone, Vector &origin, Vector &angles) {}
void CBaseAnimating ::GetAttachment(int iAttachment, Vector &origin, Vector &angles) {}
int CBaseAnimating ::FindTransition(int iEndingSequence, int iGoalSequence, int *piDir) { return NEG_REF; }
void CBaseAnimating ::GetAutomovement(Vector &origin, Vector &angles, float flInterval) {}
void CBaseAnimating ::SetBodygroup(int iGroup, int iValue) {}
int CBaseAnimating ::GetBodygroup(int iGroup) { return NULL; }
Vector CBaseMonster ::GetGunPosition(void) { return g_vecZero; }
void CBaseEntity::FireBullets(unsigned long cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker) {}
void CBaseEntity ::TraceBleed(float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) {}
void CBaseMonster ::MakeDamageBloodDecal(int cCount, float flNoise, TraceResult *ptr, const Vector &vecDir) {}
BOOL CBaseMonster ::FGetNodeRoute(Vector vecDest) { return true; }
int CBaseMonster ::FindHintNode(void) { return NO_NODE; }
void CBaseMonster::ReportAIState(void) {}
void CBaseMonster ::KeyValue(KeyValueData *pkvd) {}
BOOL CBaseMonster ::FCheckAITrigger(void) { return false; }
int CBaseMonster ::CanPlaySequence(BOOL fDisregardMonsterState, int interruptLevel) { return false; }
BOOL CBaseMonster ::FindLateralCover(const Vector &vecThreat, const Vector &vecViewOffset) { return false; }
Vector CBaseMonster ::ShootAtEnemy(const Vector &shootOrigin) { return g_vecZero; }
BOOL CBaseMonster ::FacingIdeal(void) { return false; }
BOOL CBaseMonster ::FCanActiveIdle(void) { return false; }
void CBaseMonster::PlaySentence(const char *pszSentence, float duration, float volume, float attenuation) {}
void CBaseMonster::PlayScriptedSentence(const char *pszSentence, float duration, float volume, float attenuation, BOOL bConcurrent, CBaseEntity *pListener) {}
void CBaseMonster::SentenceStop(void) {}
void CBaseMonster::CorpseFallThink(void) {}
void CBaseMonster ::MonsterInitDead(void) {}
BOOL CBaseMonster ::BBoxFlat(void) { return true; }
BOOL CBaseMonster ::GetEnemy(void) { return false; }
void CBaseMonster ::TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) {}
CBaseEntity *CBaseMonster ::DropItem(char *pszItemName, const Vector &vecPos, const Vector &vecAng) { return NULL; }
BOOL CBaseMonster ::ShouldFadeOnDeath(void) { return false; }
void CBaseMonster ::RadiusDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType) {}
void CBaseMonster ::RadiusDamage(Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType) {}
void CBaseMonster::FadeMonster(void) {}
void CBaseMonster ::GibMonster(void) {}
BOOL CBaseMonster ::HasHumanGibs(void) { return false; }
BOOL CBaseMonster ::HasAlienGibs(void) { return false; }
Activity CBaseMonster ::GetDeathActivity(void) { return ACT_DIE_HEADSHOT; }
MONSTERSTATE CBaseMonster ::GetIdealState(void) { return MONSTERSTATE_ALERT; }
Schedule_t *CBaseMonster ::GetScheduleOfType(int Type) { return NULL; }
Schedule_t *CBaseMonster ::GetSchedule(void) { return NULL; }
void CBaseMonster ::RunTask(Task_t *pTask) {}
void CBaseMonster ::StartTask(Task_t *pTask) {}
Schedule_t *CBaseMonster::ScheduleFromName(const char *pName) { return NULL; }
void CBaseMonster::BecomeDead(void) {}
void CBaseMonster ::RunAI(void) {}
void CBaseMonster ::Killed(entvars_t *pevAttacker, int iGib) {}
int CBaseMonster ::TakeHealth(float flHealth, int bitsDamageType) { return NULL; }
int CBaseMonster::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) {return NULL;}
int CBaseMonster::Restore(class CRestore &) { return true; }
int CBaseMonster::Save(class CSave &) { return true; }

int TrainSpeed(int iSpeed, int iMax) { return NULL; }

//Master Sword
void CBasePlayer::PainSound() {}
void CBasePlayer::TakeDamageEffect(CBaseEntity *pInflictor, CBaseEntity *pAttacker, float flDamage, int bitsDamageType) {}
int CBasePlayer::IRelationship(CBaseEntity *pTarget) { return NULL; }

int CBasePlayerWeapon::Restore(class CRestore &) { return true; }
int CBasePlayerWeapon::Save(class CSave &) { return true; }

void CBasePlayer::StruckSound(CBaseEntity *pInflicter, CBaseEntity *pAttacker, float flDamage, TraceResult *ptr, int bitsDamageType) {}
void CBasePlayer::DeathSound(void) {}
int CBasePlayer::TakeHealth(float flHealth, int bitsDamageType) { return NULL; }
int CBasePlayer::TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) { return NULL; }
void CBasePlayer::PackDeadPlayerItems(void) {}
void CBasePlayer::WaterMove() {}
BOOL CBasePlayer::IsOnLadder(void) { return false; }
void CBasePlayer::PlayerDeathThink(void) {}
void CBasePlayer::StartDeathCam(void) {}
void CBasePlayer::StartObserver(Vector vecPosition, Vector vecViewAngle) {}
void CBasePlayer::PlayerUse(void) {}
void CBasePlayer::Jump() {}
void CBasePlayer::Duck() {}
int CBasePlayer::Classify(void) { return NULL; }
//void CBasePlayer :: PlayStepSound(int step, float fvol) { }
//void CBasePlayer :: UpdateStepSound( void ) { }
void CBasePlayer::PreThink(void) {}
void CBasePlayer::CheckTimeBasedDamage() {}
void CBasePlayer ::UpdateGeigerCounter(void) {}
void CBasePlayer ::UpdatePlayerSound(void) {}
void CBasePlayer::PostThink() {}
void CBasePlayer ::Precache(void) {}
int CBasePlayer::Save(CSave &save) { return NULL; }
void CBasePlayer::RenewItems(void) {}
int CBasePlayer::Restore(CRestore &restore) { return NULL; }
void CBasePlayer::SelectNextItem(int iItem) {}
BOOL CBasePlayer::HasWeapons(void) { return false; }
void CBasePlayer::SelectPrevItem(int iItem) {}
CBaseEntity *FindEntityForward(CBaseEntity *pMe) { return NULL; }
//BOOL CBasePlayer :: FlashlightIsOn( void ) { return false; }
//void CBasePlayer :: FlashlightTurnOn( void ) { }
//void CBasePlayer :: FlashlightTurnOff( void ) { }
//void CBasePlayer :: ForceClientDllUpdate( void ) { }
void CBasePlayer::ImpulseCommands() {}
void CBasePlayer::CheatImpulseCommands(int iImpulse) {}
void CBasePlayer::ItemPreFrame() {}
void CBasePlayer::ItemPostFrame() {}
int CBasePlayer::AmmoInventory(int iAmmoIndex) { return NEG_REF; }
//Master Sword
void CBasePlayer ::UpdateClientData(void) {}
BOOL CBasePlayer ::FBecomeProne(void) { return true; }
void CBasePlayer ::BarnacleVictimBitten(entvars_t *pevBarnacle) {}
void CBasePlayer ::BarnacleVictimReleased(void) {}
int CBasePlayer ::Illumination(void) { return NULL; }
void CBasePlayer ::EnableControl(BOOL fControl) {}
Vector CBasePlayer ::GetAutoaimVector(float flDelta) { return g_vecZero; }
//Vector CBasePlayer :: AutoaimDeflection( Vector &vecSrc, float flDist, float flDelta  ) { return g_vecZero; }
void CBasePlayer ::ResetAutoaim() {}
void CBasePlayer ::SetCustomDecalFrames(int nFrames) {}
int CBasePlayer ::GetCustomDecalFrames(void) { return NEG_REF; }
Vector CBasePlayer ::GetGunPosition(void) { return g_vecZero; }
//Master Sword
void CBasePlayer::Trade() {}
bool CBasePlayer::AcceptOffer() { return false; }
tradeinfo_t *CBasePlayer::TradeItem(tradeinfo_t *ptiTradeInfo) { return NULL; }
BOOL CBasePlayer::SkinMonster(CMSMonster *pDeadMonster) { return false; }
void CBasePlayer::AddNoise(float flNoiseAmt) {}
void CBasePlayer::Attacked(CBaseEntity *pAttacker, float flDamage, int bitsDamageType) {}
void CBasePlayer::Seen(CMSMonster *pMonster) {}
//------------

void G_MakeSHIELDs() {}
void G_RemoveSHIELDs() {}
DLL_GLOBAL Vector g_vecAttackDir;
void UTIL_SetSize(entvars_t *pev, const Vector &vecMin, const Vector &vecMax) {}
//int gmsgCurWeapon = 0; //from server dll player.cpp

CBaseEntity *EHANDLE ::operator->() { return NULL; }
