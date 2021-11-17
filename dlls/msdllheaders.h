#include "../MSShared/buildcontrol.h"
#include "Half-life/extdll.h"
#include "Half-life/util.h"
#include "../MSShared/iscript.h"	//MS scripted interface
#include "../MSShared/sharedutil.h" //MS generic utility functions
#include "cbase.h"
#ifndef _WIN32
#define Sleep sleep
#endif

#pragma warning(disable : 4995) // allow use of old headers (such as string.h, iostream.h, etc.)
#pragma warning(disable : 4288) // loop control variable declared in the for-loop is used outside the for-loop scope

BOOL UTIL_IsPointWithinEntity(Vector &vPoint, CBaseEntity *pEntity);
void SpriteEffect(CBaseEntity *pEntity, int Effect, char *cSprite);
Vector GetHighBone(entvars_t *pev, int Bone);
CBaseEntity *MSInstance(edict_t *pent);
CBaseEntity *MSInstance(entvars_t *pev);
void *MSCopyClassMemory(void *pDest, void *pSource, size_t Length);
void *MSZeroClassMemory(void *pDest, size_t Length);

#ifndef VALVE_DLL
#undef DLLEXPORT
#endif