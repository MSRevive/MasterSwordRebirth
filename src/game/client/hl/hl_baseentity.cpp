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
#include "logger.h"
#include "ms/clglobal.h"

#include "cl_entity.h"
#include "event_api.h"
#include "hl/extdll.h"

// Globals used by client.dll
const Vector g_vecZero = Vector(0, 0, 0);
int gmsgWeapPickup = 0;
enginefuncs_t g_engfuncs;
globalvars_t *gpGlobals;

//#define LOG_ITEMHANDLING

#ifdef LOG_ITEMHANDLING
#define logfileopt logfile
#else
#define logfileopt NullFile
#endif
//-----------

void EMIT_SOUND_DYN(edict_t *entity, int channel, const char *sample, float volume, float attenuation, int flags, float pitch)
{

	entvars_t *pev = (entvars_t *)entity;

	//	Print( "Sample: %s, %i\n", (char *)sample, pev->iuser1 );
	//	gEngfuncs.pfnPlaySoundByNameAtLocation( (char *)sample, volume, (float *)&pEntity->pev->origin );
	gEngfuncs.pEventAPI->EV_PlaySound(pev->iuser1, (float *)pev->origin, channel, sample, volume, attenuation, flags, pitch);
}

// CBaseEntity Stubs
int CBaseEntity ::TakeHealth(float flHealth, int bitsDamageType) { return 1; }
int CBaseEntity ::TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) { return 1; }
CBaseEntity *CBaseEntity::GetNextTarget(void) { return NULL; }
int CBaseEntity::Save(CSave &save) { return 1; }
int CBaseEntity::Restore(CRestore &restore) { return 1; }
void CBaseEntity::SetObjectCollisionBox(void) {}
int CBaseEntity ::Intersects(CBaseEntity *pOther) { return 0; }
void CBaseEntity ::MakeDormant(void) {}
int CBaseEntity ::IsDormant(void) { return 0; }
bool CBaseEntity ::IsInWorld(void) { return true; }
int CBaseEntity::ShouldToggle(USE_TYPE useType, bool currentState) { return 0; }
int CBaseEntity ::DamageDecal(int bitsDamageType) { return -1; }
CBaseEntity *CBaseEntity::Create(char *szName, const Vector &vecOrigin, const Vector &vecAngles, edict_t *pentOwner) { return NULL; }
void CBaseEntity::SUB_Remove(void)
{
	startdbg;

	dbg("Call Deactivate");
	Deactivate();
	if (logfileopt.is_open())
	{
		logfileopt << "DELETE ITEM: " << DisplayName();
		logfileopt << " (" << (IsMSItem() ? ((CBasePlayerItem *)this)->m_iId : 0) << ")\r\n";
	}
	dbg("Delete Entity");
	MSCLGlobals::RemoveEnt(this, true);

	enddbg;
}
//void CBaseEntity::StruckSound( CBaseEntity *pInflicter, CBaseEntity *pAttacker, float flDamage, TraceResult *ptr, int bitsDamageType ) { }
void CBaseEntity::CounterEffect(CBaseEntity *pInflictor, int iEffect, void *pExtraData) {}

// CBaseDelay Stubs
void CBaseDelay ::KeyValue(struct KeyValueData_s *) {}
int CBaseDelay::Restore(class CRestore &) { return 1; }
int CBaseDelay::Save(class CSave &) { return 1; }
CBaseEntity *CBaseDelay::GetSelf() { return this; }

// CBaseAnimating Stubs
int CBaseAnimating::Restore(class CRestore &) { return 1; }
int CBaseAnimating::Save(class CSave &) { return 1; }

// DEBUG Stubs
edict_t *DBG_EntOfVars(const entvars_t *pev) { return NULL; }
void DBG_AssertFunction(bool fExpr, const char *szExpr, const char *szFile, int szLine, const char *szMessage) {}

// UTIL_* Stubs
void UTIL_PrecacheOther(const char *szClassname) {}
void UTIL_BloodDrips(const Vector &origin, const Vector &direction, int color, int amount) {}
void UTIL_DecalTrace(TraceResult *pTrace, int decalNumber) {}
void UTIL_GunshotDecalTrace(TraceResult *pTrace, int decalNumber) {}
void UTIL_MakeVectors(const Vector &vecAngles) {}
bool UTIL_IsValidEntity(edict_t *pent) { return true; }
void UTIL_SetOrigin(entvars_t *, const Vector &org) {}
bool UTIL_GetNextBestWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon) { return true; }
void UTIL_LogPrintf(const char *, ...) {}
void UTIL_ClientPrintAll(int, char const *, char const *, char const *, char const *, char const *) {}
void PrintNotifyMsg(char *pszmsg);
void ClientPrint(entvars_t *client, int msg_dest, const char *msg_name, const char *param1, const char *param2, const char *param3, const char *param4)
{
	PrintNotifyMsg((char *)msg_name);
	//gEngfuncs.pfnConsolePrint( (char *)msg_name );
}

// CBaseToggle Stubs
int CBaseToggle::Restore(class CRestore &) { return 1; }
int CBaseToggle::Save(class CSave &) { return 1; }
void CBaseToggle ::KeyValue(struct KeyValueData_s *) {}

// CGrenade Stubs
void CGrenade::BounceSound(void) {}
void CGrenade::Explode(Vector, Vector) {}
void CGrenade::Explode(TraceResult *, int) {}
void CGrenade::Killed(entvars_t *, int) {}
void CGrenade::Spawn(void) {}

CBaseEntity *CBaseMonster ::CheckTraceHullAttack(float flDist, int iDamage, int iDmgType) { return nullptr; }
void CBaseMonster ::Eat(float flFullDuration) {}
bool CBaseMonster ::FShouldEat(void) { return true; }
void CBaseMonster ::BarnacleVictimBitten(entvars_t *pevBarnacle) {}
void CBaseMonster ::BarnacleVictimReleased(void) {}
void CBaseMonster ::Listen(void) {}
float CBaseMonster ::FLSoundVolume(CSound *pSound) { return 0.0; }
bool CBaseMonster ::FValidateHintType(short sHint) { return false; }
void CBaseMonster ::Look(int iDistance) {}
int CBaseMonster ::ISoundMask(void) { return 0; }
CSound *CBaseMonster ::PBestSound(void) { return nullptr; }
CSound *CBaseMonster ::PBestScent(void) { return nullptr; }
float CBaseAnimating ::StudioFrameAdvance(float flInterval) { return 0.0; }
void CBaseMonster ::MonsterThink(void) {}
void CBaseMonster ::MonsterUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value) {}
int CBaseMonster ::IgnoreConditions(void) { return 0; }
void CBaseMonster ::RouteClear(void) {}
void CBaseMonster ::RouteNew(void) {}
bool CBaseMonster ::FRouteClear(void) { return false; }
bool CBaseMonster ::FRefreshRoute(void) { return 0; }
bool CBaseMonster::MoveToEnemy(Activity movementAct, float waitTime) { return false; }
bool CBaseMonster::MoveToLocation(Activity movementAct, float waitTime, const Vector &goal) { return false; }
bool CBaseMonster::MoveToTarget(Activity movementAct, float waitTime) { return false; }
bool CBaseMonster::MoveToNode(Activity movementAct, float waitTime, const Vector &goal) { return false; }
int ShouldSimplify(int routeType) { return true; }
void CBaseMonster ::RouteSimplify(CBaseEntity *pTargetEnt) {}
bool CBaseMonster ::FBecomeProne(void) { return true; }
bool CBaseMonster ::CheckRangeAttack1(float flDot, float flDist) { return false; }
bool CBaseMonster ::CheckRangeAttack2(float flDot, float flDist) { return false; }
bool CBaseMonster ::CheckMeleeAttack1(float flDot, float flDist) { return false; }
bool CBaseMonster ::CheckMeleeAttack2(float flDot, float flDist) { return false; }
void CBaseMonster ::CheckAttacks(CBaseEntity *pTarget, float flDist) {}
bool CBaseMonster ::FCanCheckAttacks(void) { return false; }
int CBaseMonster ::CheckEnemy(CBaseEntity *pEnemy) { return 0; }
void CBaseMonster ::PushEnemy(CBaseEntity *pEnemy, Vector &vecLastKnownPos) {}
bool CBaseMonster ::PopEnemy() { return false; }
void CBaseMonster ::SetActivity(Activity NewActivity) {}
void CBaseMonster ::SetSequenceByName(char *szSequence) {}
int CBaseMonster ::CheckLocalMove(const Vector &vecStart, const Vector &vecEnd, CBaseEntity *pTarget, float *pflDist) { return 0; }
float CBaseMonster ::OpenDoorAndWait(entvars_t *pevDoor) { return 0.0; }
void CBaseMonster ::AdvanceRoute(float distance) {}
int CBaseMonster ::RouteClassify(int iMoveFlag) { return 0; }
bool CBaseMonster ::BuildRoute(const Vector &vecGoal, int iMoveFlag, CBaseEntity *pTarget) { return false; }
void CBaseMonster ::InsertWaypoint(Vector vecLocation, int afMoveFlags) {}
bool CBaseMonster ::FTriangulate(const Vector &vecStart, const Vector &vecEnd, float flDist, CBaseEntity *pTargetEnt, Vector *pApex) { return false; }
void CBaseMonster ::Move(float flInterval) {}
bool CBaseMonster::ShouldAdvanceRoute(float flWaypointDist) { return false; }
void CBaseMonster::MoveExecute(CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval) {}
void CBaseMonster ::MonsterInit(void) {}
void CBaseMonster ::MonsterInitThink(void) {}
void CBaseMonster ::StartMonster(void) {}
void CBaseMonster ::MovementComplete(void) {}
int CBaseMonster::TaskIsRunning(void) { return 0; }
int CBaseMonster::IRelationship(CBaseEntity *pTarget) { return 0; }
bool CBaseMonster ::FindCover(Vector vecThreat, Vector vecViewOffset, float flMinDist, float flMaxDist) { return false; }
bool CBaseMonster ::BuildNearestRoute(Vector vecThreat, Vector vecViewOffset, float flMinDist, float flMaxDist) { return false; }
CBaseEntity *CBaseMonster ::BestVisibleEnemy(void) { return NULL; }
bool CBaseMonster ::FInViewCone(CBaseEntity *pEntity) { return false; }
bool CBaseMonster ::FInViewCone(Vector &Origin) { return false; }
bool CBaseEntity ::FVisible(CBaseEntity *pEntity) { return false; }
bool CBaseEntity ::FVisible(const Vector &vecOrigin) { return false; }
bool CBaseEntity ::FMVisible(CBaseEntity *pEntity) { return false; }
bool CBaseEntity ::FMVisible(const Vector &vecOrigin) { return false; }
void CBaseMonster ::MakeIdealYaw(Vector vecTarget) {}
float CBaseMonster::FlYawDiff(void) { return 0.0; }
float CBaseMonster::ChangeYaw(int yawSpeed) { return 0; }
float CBaseMonster::VecToYaw(Vector vecDir) { return 0.0; }
int CBaseAnimating ::LookupActivity(int activity) { return 0; }
int CBaseAnimating ::LookupActivityHeaviest(int activity) { return 0; }
void CBaseMonster ::SetEyePosition(void) {}
int CBaseAnimating ::LookupSequence(const char *label) { return 0; }
void CBaseAnimating ::ResetSequenceInfo() {}
int CBaseAnimating ::GetSequenceFlags() { return false; }
void CBaseAnimating ::DispatchAnimEvents(float flInterval) {}
void CBaseMonster ::HandleAnimEvent(MonsterEvent_t *pEvent) {}
float CBaseAnimating ::SetBoneController(int iController, float flValue) { return 0.0; }
void CBaseAnimating ::InitBoneControllers(void) {}
float CBaseAnimating ::SetBlending(int iBlender, float flValue) { return 0; }
void CBaseAnimating ::GetBonePosition(int iBone, Vector &origin, Vector &angles) {}
void CBaseAnimating ::GetAttachment(int iAttachment, Vector &origin, Vector &angles) {}
int CBaseAnimating ::FindTransition(int iEndingSequence, int iGoalSequence, int *piDir) { return -1; }
void CBaseAnimating ::GetAutomovement(Vector &origin, Vector &angles, float flInterval) {}
void CBaseAnimating ::SetBodygroup(int iGroup, int iValue) {}
int CBaseAnimating ::GetBodygroup(int iGroup) { return 0; }
Vector CBaseMonster ::GetGunPosition(void) { return g_vecZero; }
void CBaseEntity::FireBullets(unsigned int cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker) {}
void CBaseEntity ::TraceBleed(float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) {}
void CBaseMonster ::MakeDamageBloodDecal(int cCount, float flNoise, TraceResult *ptr, const Vector &vecDir) {}
bool CBaseMonster ::FGetNodeRoute(Vector vecDest) { return true; }
int CBaseMonster ::FindHintNode(void) { return NO_NODE; }
void CBaseMonster::ReportAIState(void) {}
void CBaseMonster ::KeyValue(KeyValueData *pkvd) {}
bool CBaseMonster ::FCheckAITrigger(void) { return false; }
int CBaseMonster ::CanPlaySequence(bool fDisregardMonsterState, int interruptLevel) { return false; }
bool CBaseMonster ::FindLateralCover(const Vector &vecThreat, const Vector &vecViewOffset) { return false; }
Vector CBaseMonster ::ShootAtEnemy(const Vector &shootOrigin) { return g_vecZero; }
bool CBaseMonster ::FacingIdeal(void) { return false; }
bool CBaseMonster ::FCanActiveIdle(void) { return false; }
void CBaseMonster::PlaySentence(const char *pszSentence, float duration, float volume, float attenuation) {}
void CBaseMonster::PlayScriptedSentence(const char *pszSentence, float duration, float volume, float attenuation, bool bConcurrent, CBaseEntity *pListener) {}
void CBaseMonster::SentenceStop(void) {}
void CBaseMonster::CorpseFallThink(void) {}
void CBaseMonster ::MonsterInitDead(void) {}
bool CBaseMonster ::BBoxFlat(void) { return true; }
bool CBaseMonster ::GetEnemy(void) { return false; }
void CBaseMonster ::TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) {}
CBaseEntity *CBaseMonster ::DropItem(char *pszItemName, const Vector &vecPos, const Vector &vecAng) { return NULL; }
bool CBaseMonster ::ShouldFadeOnDeath(void) { return false; }
void CBaseMonster ::RadiusDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType) {}
void CBaseMonster ::RadiusDamage(Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType) {}
void CBaseMonster::FadeMonster(void) {}
void CBaseMonster ::GibMonster(void) {}
bool CBaseMonster ::HasHumanGibs(void) { return false; }
bool CBaseMonster ::HasAlienGibs(void) { return false; }
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
int CBaseMonster ::TakeHealth(float flHealth, int bitsDamageType) { return 0; }
int CBaseMonster ::TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) { return 0; }
int CBaseMonster::Restore(class CRestore &) { return 1; }
int CBaseMonster::Save(class CSave &) { return 1; }

int TrainSpeed(int iSpeed, int iMax) { return 0; }

//Master Sword
void CBasePlayer::PainSound() {}
void CBasePlayer::TakeDamageEffect(CBaseEntity *pInflictor, CBaseEntity *pAttacker, float flDamage, int bitsDamageType) {}
int CBasePlayer::IRelationship(CBaseEntity *pTarget) { return 0; }

int CBasePlayerWeapon::Restore(class CRestore &) { return 1; }
int CBasePlayerWeapon::Save(class CSave &) { return 1; }

void CBasePlayer::StruckSound(CBaseEntity *pInflicter, CBaseEntity *pAttacker, float flDamage, TraceResult *ptr, int bitsDamageType) {}
void CBasePlayer::DeathSound(void) {}
int CBasePlayer::TakeHealth(float flHealth, int bitsDamageType) { return 0; }
int CBasePlayer::TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) { return 0; }
void CBasePlayer::PackDeadPlayerItems(void) {}
void CBasePlayer::WaterMove() {}
bool CBasePlayer::IsOnLadder(void) { return false; }
void CBasePlayer::PlayerDeathThink(void) {}
void CBasePlayer::StartDeathCam(void) {}
void CBasePlayer::StartObserver(Vector vecPosition, Vector vecViewAngle) {}
void CBasePlayer::PlayerUse(void) {}
void CBasePlayer::Jump() {}
void CBasePlayer::Duck() {}
int CBasePlayer::Classify(void) { return 0; }
//void CBasePlayer :: PlayStepSound(int step, float fvol) { }
//void CBasePlayer :: UpdateStepSound( void ) { }
void CBasePlayer::PreThink(void) {}
void CBasePlayer::CheckTimeBasedDamage() {}
void CBasePlayer ::UpdateGeigerCounter(void) {}
void CBasePlayer ::UpdatePlayerSound(void) {}
void CBasePlayer::PostThink() {}
void CBasePlayer ::Precache(void) {}
int CBasePlayer::Save(CSave &save) { return 0; }
void CBasePlayer::RenewItems(void) {}
int CBasePlayer::Restore(CRestore &restore) { return 0; }
void CBasePlayer::SelectNextItem(int iItem) {}
bool CBasePlayer::HasWeapons(void) { return false; }
void CBasePlayer::SelectPrevItem(int iItem) {}
CBaseEntity *FindEntityForward(CBaseEntity *pMe) { return NULL; }
//bool CBasePlayer :: FlashlightIsOn( void ) { return false; }
//void CBasePlayer :: FlashlightTurnOn( void ) { }
//void CBasePlayer :: FlashlightTurnOff( void ) { }
//void CBasePlayer :: ForceClientDllUpdate( void ) { }
void CBasePlayer::ImpulseCommands() {}
void CBasePlayer::CheatImpulseCommands(int iImpulse) {}
void CBasePlayer::ItemPreFrame() {}
void CBasePlayer::ItemPostFrame() {}
int CBasePlayer::AmmoInventory(int iAmmoIndex) { return -1; }
//Master Sword
void CBasePlayer ::UpdateClientData(void) {}
bool CBasePlayer ::FBecomeProne(void) { return true; }
void CBasePlayer ::BarnacleVictimBitten(entvars_t *pevBarnacle) {}
void CBasePlayer ::BarnacleVictimReleased(void) {}
int CBasePlayer ::Illumination(void) { return 0; }
void CBasePlayer ::EnableControl(bool fControl) {}
Vector CBasePlayer ::GetAutoaimVector(float flDelta) { return g_vecZero; }
//Vector CBasePlayer :: AutoaimDeflection( Vector &vecSrc, float flDist, float flDelta  ) { return g_vecZero; }
void CBasePlayer ::ResetAutoaim() {}
void CBasePlayer ::SetCustomDecalFrames(int nFrames) {}
int CBasePlayer ::GetCustomDecalFrames(void) { return -1; }
Vector CBasePlayer ::GetGunPosition(void) { return g_vecZero; }
//Master Sword
void CBasePlayer::Trade() {}
bool CBasePlayer::AcceptOffer() { return false; }
tradeinfo_t *CBasePlayer::TradeItem(tradeinfo_t *ptiTradeInfo) { return NULL; }
bool CBasePlayer::SkinMonster(CMSMonster *pDeadMonster) { return false; }
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
