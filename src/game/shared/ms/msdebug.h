//Master Sword debug
#ifndef MSDEBUG_H
#define MSDEBUG_H

#include "sharedutil.h"
#ifndef NULL
#define NULL 0
#endif

#ifdef VECTOR_H
struct sharedtrace_t
{
	bool Allsolid;		  // if true, plane is not valid
	bool Startsolid;	  // if true, the initial point was in a solid area
	bool Inopen, Inwater; // End point is in empty space or in water
	float Fraction;		  // time completed, 1.0 = didn't hit anything
	Vector EndPos;		  // final position
	Vector PlaneNormal;	  // surface normal at impact
	float PlaneDist;
	int HitEnt;			  // entity at impact
	Vector DeltaVelocity; // Change in player's velocity caused by impact. (client only)
	int Hitgroup;
};
#define PM_NORMAL 0x00000000
#define PM_STUDIO_IGNORE 0x00000001 // Skip studio models
#define PM_STUDIO_BOX 0x00000002	// Use boxes for non-complex studio models (even in traceline)
#define PM_GLASS_IGNORE 0x00000004	// Ignore entities with non-normal rendermode
#define PM_WORLD_ONLY 0x00000008	// Only trace against the world
#endif

//Engine functions shared between the client and server
//Each dll has a different implementation mapping to the proper engine call
class EngineFunc
{
public:
	static const char* GetGameDir();
	static const char* GetString(int string);
	static int AllocString(const char* String);
	static float CVAR_GetFloat(const char* Cvar);
	static const char* CVAR_GetString(const char* Cvar);
	static void CVAR_SetFloat(const char* Cvar, float Value);
	static void CVAR_SetString(const char* Cvar, const char* Value);
#ifdef VECTOR_H
	static void MakeVectors(const Vector &vecAngles, float *p_vForward, float *p_vRight = NULL, float *p_vUp = NULL);
#endif
#ifdef CVARDEF_H
#ifndef VALVE_DLL
	static cvar_s *CVAR_Create(const char* Cvar, const char* Value, const int Flags);
#endif
#ifdef VALVE_DLL
	static void CVAR_Create(cvar_t &Cvar);
#endif
#endif
#ifdef VECTOR_H
	static void Shared_TraceLine(const Vector &vecStart, const Vector &vecEnd, int ignore_monsters, sharedtrace_t &Tr, int CLFlags, int CLIgnoreFlags, edict_t *SVIgnoreEnt = NULL);
	static int Shared_PointContents(const Vector &Origin);
	static float Shared_GetWaterHeight(const Vector &Origin, float minz, float maxz);											 //Find height between a minx and maxs
	static void Shared_PlaySound3D(const char* Sound, float Volume, const Vector &Origin, float Attn);							 //Play sound independent of an entity
	static void Shared_PlaySound3D2(const char* Sound, float Volume, const Vector &Origin, float Attn, int Chan, float sPitch); //MAR2012 Thothie - Same diff + Channel Control
#endif
	static float AngleDiff(float destAngle, float srcAngle);
};

#ifdef DEV_BUILD
void *operator new(size_t size, const char *pszSourceFile, int LineNum);
void operator delete(void *ptr, const char *pszSourceFile, int LineNum);
#define msnew new (__FILE__, __LINE__)
#else
#define msnew new
#endif

struct memalloc_t
{
	msstring SourceFile;
	int LineNum;
	int Index;
	void *pAddr;
	bool Used;
	inline memalloc_t &operator=(const memalloc_t &mem)
	{
		SourceFile = mem.SourceFile;
		LineNum = mem.LineNum;
		pAddr = mem.pAddr;
		return *this;
	}
};

#endif
