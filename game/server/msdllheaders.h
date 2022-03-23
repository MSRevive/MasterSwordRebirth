#ifndef _WIN32
typedef unsigned int size_t;
#endif

#include "buildcontrol.h"
#include "hl/extdll.h"
#include "hl/util.h"
#include "iscript.h"	//MS scripted interface
#include "sharedutil.h" //MS generic utility functions
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