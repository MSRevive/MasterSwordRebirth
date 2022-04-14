/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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
#ifndef ENGINECALLBACK_H
#define ENGINECALLBACK_H
#pragma once

#include "event_flags.h"
#include <string.h>
//include "logger.h"

// Must be provided by user of this code
extern enginefuncs_t g_engfuncs;

extern unsigned int g_iNumBytesWritten;
extern int g_iUserMessageType;
#define MAX_USERMESSAGE_SIZE 192

// The actual engine callbacks
#define GETPLAYERUSERID (*g_engfuncs.pfnGetPlayerUserId)
//#define PRECACHE_MODEL	(*g_engfuncs.pfnPrecacheModel)
int PRECACHE_MODEL(const char *pszModel); //Master Sword - Keep track of all model precaching
int PRECACHE_SOUND(const char *pszSound); //Thothie MAR2012_26 - Ditto for sounds
//define PRECACHE_SOUND(sFile) logfile << "Precache_Sound(" << MSGlobals->gSoundPrecacheCount++ << "):" << sFile << "\n"; (*g_engfuncs.pfnPrecacheSound)(sFile) //Thothie Track/Manage Sound Precaches MAR2012_26
#define	WRITE_BOOL( b )	WRITE_BYTE( b ? 1 : 0 )			// MIB FEB2019_23 [LOCAL_PANEL_IMAGE]
#define PRECACHE_GENERIC (*g_engfuncs.pfnPrecacheGeneric)
#define SET_MODEL (*g_engfuncs.pfnSetModel)
#define MODEL_INDEX (*g_engfuncs.pfnModelIndex)
#define MODEL_FRAMES (*g_engfuncs.pfnModelFrames)
#define SET_SIZE (*g_engfuncs.pfnSetSize)
#define CHANGE_LEVEL (*g_engfuncs.pfnChangeLevel)
#define GET_SPAWN_PARMS (*g_engfuncs.pfnGetSpawnParms)
#define SAVE_SPAWN_PARMS (*g_engfuncs.pfnSaveSpawnParms)
#define VEC_TO_YAW (*g_engfuncs.pfnVecToYaw)
#define VEC_TO_ANGLES (*g_engfuncs.pfnVecToAngles)
#define MOVE_TO_ORIGIN (*g_engfuncs.pfnMoveToOrigin)
#define oldCHANGE_YAW (*g_engfuncs.pfnChangeYaw)
#define CHANGE_PITCH (*g_engfuncs.pfnChangePitch)
#define MAKE_VECTORS (*g_engfuncs.pfnMakeVectors)
#define CREATE_ENTITY (*g_engfuncs.pfnCreateEntity)
#define REMOVE_ENTITY (*g_engfuncs.pfnRemoveEntity)
#define CREATE_NAMED_ENTITY (*g_engfuncs.pfnCreateNamedEntity)
#define MAKE_STATIC (*g_engfuncs.pfnMakeStatic)
#define ENT_IS_ON_FLOOR (*g_engfuncs.pfnEntIsOnFloor)
#define DROP_TO_FLOOR (*g_engfuncs.pfnDropToFloor)
#define WALK_MOVE (*g_engfuncs.pfnWalkMove)
#define SET_ORIGIN (*g_engfuncs.pfnSetOrigin)
#define EMIT_SOUND_DYN2 (*g_engfuncs.pfnEmitSound)
#define BUILD_SOUND_MSG (*g_engfuncs.pfnBuildSoundMsg)
#define TRACE_LINE (*g_engfuncs.pfnTraceLine)
#define TRACE_TOSS (*g_engfuncs.pfnTraceToss)
#define TRACE_MONSTER_HULL (*g_engfuncs.pfnTraceMonsterHull)
#define TRACE_HULL (*g_engfuncs.pfnTraceHull)
#define GET_AIM_VECTOR (*g_engfuncs.pfnGetAimVector)
#define SERVER_COMMAND (*g_engfuncs.pfnServerCommand)
#define SERVER_EXECUTE (*g_engfuncs.pfnServerExecute)
#define CLIENT_COMMAND (*g_engfuncs.pfnClientCommand)
#define PARTICLE_EFFECT (*g_engfuncs.pfnParticleEffect)
#define LIGHT_STYLE (*g_engfuncs.pfnLightStyle)
#define DECAL_INDEX (*g_engfuncs.pfnDecalIndex)
#define POINT_CONTENTS (*g_engfuncs.pfnPointContents)
#define CRC32_INIT (*g_engfuncs.pfnCRC32_Init)
#define CRC32_PROCESS_BUFFER (*g_engfuncs.pfnCRC32_ProcessBuffer)
#define CRC32_PROCESS_BYTE (*g_engfuncs.pfnCRC32_ProcessByte)
#define CRC32_FINAL (*g_engfuncs.pfnCRC32_Final)
#define RANDOM_LONG (*g_engfuncs.pfnRandomLong)
#define RANDOM_FLOAT (*g_engfuncs.pfnRandomFloat)
#define GETPLAYERAUTHID (*g_engfuncs.pfnGetPlayerAuthId)

//MIB JUN2010_17 - enable total disable of debug alert messages
#define TURN_OFF_ALERT 0
#if !TURN_OFF_ALERT
#define ALERT (*g_engfuncs.pfnAlertMessage)
#define SPAWN_GLOBAL_ITEMS 1
#else
#define ALERT
#define SPAWN_GLOBAL_ITEMS 0
#endif

inline void MESSAGE_BEGIN(int msg_dest, int msg_type, const float* pOrigin = NULL, edict_t* ed = NULL)
{
	g_iNumBytesWritten = 0;
	g_iUserMessageType = msg_type;
	(*g_engfuncs.pfnMessageBegin)(msg_dest, msg_type, pOrigin, ed);
}

inline void MESSAGE_END(void)
{
	if (g_iNumBytesWritten >= MAX_USERMESSAGE_SIZE)
	{
		ALERT(at_console, "USERMSG: Wrote at least %u bytes for usermsg %i\n", g_iNumBytesWritten, g_iUserMessageType);
	}
	(*g_engfuncs.pfnMessageEnd)();
}

// Used to check if a usermsg can write X more bytes!
inline bool CHECK_CAN_WRITE(unsigned int size)
{
	if ((g_iNumBytesWritten + size) > MAX_USERMESSAGE_SIZE)
		return false;

	g_iNumBytesWritten += size;
	return true;
}

inline void WRITE_BYTE(int iValue)
{
	if (!CHECK_CAN_WRITE(1)) return;
	(*g_engfuncs.pfnWriteByte)(iValue);
}

inline void WRITE_CHAR(int iValue)
{
	if (!CHECK_CAN_WRITE(1)) return;
	(*g_engfuncs.pfnWriteChar)(iValue);
}

inline void WRITE_SHORT(int iValue)
{
	if (!CHECK_CAN_WRITE(2)) return;
	(*g_engfuncs.pfnWriteShort)(iValue);
}

inline void WRITE_ENTITY(int iValue)
{
	if (!CHECK_CAN_WRITE(4)) return;
	(*g_engfuncs.pfnWriteEntity)(iValue);
}

inline void WRITE_LONG(int iValue)
{
	if (!CHECK_CAN_WRITE(4)) return;
	(*g_engfuncs.pfnWriteLong)(iValue);
}

inline void WRITE_ANGLE(float flValue)
{
	if (!CHECK_CAN_WRITE(1)) return;
	(*g_engfuncs.pfnWriteAngle)(flValue);
}

inline void WRITE_COORD(float flValue)
{
	if (!CHECK_CAN_WRITE(2)) return;
	(*g_engfuncs.pfnWriteCoord)(flValue);
}

inline void WRITE_STRING(const char* sz)
{
	if (!sz || !CHECK_CAN_WRITE(strlen(sz))) return;
	(*g_engfuncs.pfnWriteString)(sz);
}

#ifdef VALVE_DLL
#define WRITE_STRING_MAX 180 // Upper limit for a usermessage is around 192 bytes, keep this in mind!
extern char g_pTempStringLimit[WRITE_STRING_MAX];
// Use this to ensure that we only write this many bytes, ensure null terminated string, prevent overflows.
#define WRITE_STRING_LIMIT(src, len)  memset(g_pTempStringLimit, 0, WRITE_STRING_MAX); strncpy(g_pTempStringLimit, src, WRITE_STRING_MAX); g_pTempStringLimit[min(WRITE_STRING_MAX-1, len)] = 0; WRITE_STRING(g_pTempStringLimit)
#endif

#define CVAR_REGISTER (*g_engfuncs.pfnCVarRegister)
#define CVAR_GET_FLOAT (*g_engfuncs.pfnCVarGetFloat)
#define CVAR_GET_STRING (*g_engfuncs.pfnCVarGetString)
#define CVAR_SET_FLOAT (*g_engfuncs.pfnCVarSetFloat)
#define CVAR_SET_STRING (*g_engfuncs.pfnCVarSetString)
#define CVAR_GET_POINTER (*g_engfuncs.pfnCVarGetPointer)

#define ENGINE_FPRINTF (*g_engfuncs.pfnEngineFprintf)
#define ALLOC_PRIVATE (*g_engfuncs.pfnPvAllocEntPrivateData)
inline void *GET_PRIVATE(edict_t *pent)
{
	if (pent)
		return pent->pvPrivateData;
	return NULL;
}
#define FREE_PRIVATE (*g_engfuncs.pfnFreeEntPrivateData)
//#define STRING			(*g_engfuncs.pfnSzFromIndex)
//#define ALLOC_STRING	(*g_engfuncs.pfnAllocString)
#ifdef VALVE_DLL
int ALLOC_STRING(const char *szValue); //Master Sword - Keep track of all string allocations on server
void ClearStringPool();
#else
#define ALLOC_STRING (*g_engfuncs.pfnAllocString)
#endif

#define FIND_ENTITY_BY_STRING (*g_engfuncs.pfnFindEntityByString)
#define GETENTITYILLUM (*g_engfuncs.pfnGetEntityIllum)
#define FIND_ENTITY_IN_SPHERE (*g_engfuncs.pfnFindEntityInSphere)
#define FIND_CLIENT_IN_PVS (*g_engfuncs.pfnFindClientInPVS)
#define EMIT_AMBIENT_SOUND (*g_engfuncs.pfnEmitAmbientSound)
#define GET_MODEL_PTR (*g_engfuncs.pfnGetModelPtr)
#define REG_USER_MSG (*g_engfuncs.pfnRegUserMsg)
#define GET_BONE_POSITION (*g_engfuncs.pfnGetBonePosition)
#define FUNCTION_FROM_NAME (*g_engfuncs.pfnFunctionFromName)
#define NAME_FOR_FUNCTION (*g_engfuncs.pfnNameForFunction)
#define TRACE_TEXTURE (*g_engfuncs.pfnTraceTexture)
#define CLIENT_PRINTF (*g_engfuncs.pfnClientPrintf)
#define CMD_ARGS (*g_engfuncs.pfnCmd_Args)
#define CMD_ARGC (*g_engfuncs.pfnCmd_Argc)
#define CMD_ARGV (*g_engfuncs.pfnCmd_Argv)
#define GET_ATTACHMENT (*g_engfuncs.pfnGetAttachment)
#define SET_VIEW (*g_engfuncs.pfnSetView)
#define SET_CROSSHAIRANGLE (*g_engfuncs.pfnCrosshairAngle)
#define LOAD_FILE_FOR_ME (*g_engfuncs.pfnLoadFileForMe)
#define FREE_FILE (*g_engfuncs.pfnFreeFile)
#define COMPARE_FILE_TIME (*g_engfuncs.pfnCompareFileTime)
#define GET_GAME_DIR (*g_engfuncs.pfnGetGameDir)
#define IS_MAP_VALID (*g_engfuncs.pfnIsMapValid)
#define NUMBER_OF_ENTITIES (*g_engfuncs.pfnNumberOfEntities)
#define IS_DEDICATED_SERVER (*g_engfuncs.pfnIsDedicatedServer)

#define PRECACHE_EVENT (*g_engfuncs.pfnPrecacheEvent)
#define PLAYBACK_EVENT_FULL (*g_engfuncs.pfnPlaybackEvent)

#define ENGINE_SET_PVS (*g_engfuncs.pfnSetFatPVS)
#define ENGINE_SET_PAS (*g_engfuncs.pfnSetFatPAS)

#define ENGINE_CHECK_VISIBILITY (*g_engfuncs.pfnCheckVisibility)

#define DELTA_SET (*g_engfuncs.pfnDeltaSetField)
#define DELTA_UNSET (*g_engfuncs.pfnDeltaUnsetField)
#define DELTA_ADDENCODER (*g_engfuncs.pfnDeltaAddEncoder)
#define ENGINE_CURRENT_PLAYER (*g_engfuncs.pfnGetCurrentPlayer)

#define ENGINE_CANSKIP (*g_engfuncs.pfnCanSkipPlayer)

#define DELTA_FINDFIELD (*g_engfuncs.pfnDeltaFindField)
#define DELTA_SETBYINDEX (*g_engfuncs.pfnDeltaSetFieldByIndex)
#define DELTA_UNSETBYINDEX (*g_engfuncs.pfnDeltaUnsetFieldByIndex)

#define ENGINE_GETPHYSINFO (*g_engfuncs.pfnGetPhysicsInfoString)

#define ENGINE_SETGROUPMASK (*g_engfuncs.pfnSetGroupMask)

#define ENGINE_INSTANCE_BASELINE (*g_engfuncs.pfnCreateInstancedBaseline)

#define ENGINE_FORCE_UNMODIFIED (*g_engfuncs.pfnForceUnmodified)

#define PLAYER_CNX_STATS (*g_engfuncs.pfnGetPlayerStats)

#endif //ENGINECALLBACK_H