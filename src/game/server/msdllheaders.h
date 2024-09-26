#include "buildcontrol.h"
#include "hl/extdll.h"
#include "hl/util.h"
#include "iscript.h"	//MS scripted interface
#include "sharedutil.h" //MS generic utility functions
#include "cbase.h"

#ifndef _WIN32
#define MINCHAR     0x80        
#define MAXCHAR     0x7f        
#define MINSHORT    0x8000      
#define MAXSHORT    0x7fff      
#define MINLONG     0x80000000  
#define MAXLONG     0x7fffffff  
#define MAXBYTE     0xff        
#define MAXWORD     0xffff      
#define MAXDWORD    0xffffffff  
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