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
	static msstring_ref GetGameDir();
	static msstring_ref GetString(int string);
	static int AllocString(msstring_ref String);
	static float CVAR_GetFloat(msstring_ref Cvar);
	static msstring_ref CVAR_GetString(msstring_ref Cvar);
	static void CVAR_SetFloat(msstring_ref Cvar, float Value);
	static void CVAR_SetString(msstring_ref Cvar, msstring_ref Value);
#ifdef VECTOR_H
	static void MakeVectors(const Vector &vecAngles, float *p_vForward, float *p_vRight = NULL, float *p_vUp = NULL);
#endif
#ifdef CVARDEF_H
#ifndef VALVE_DLL
	static cvar_s *CVAR_Create(msstring_ref Cvar, msstring_ref Value, const int Flags);
#endif
#ifdef VALVE_DLL
	static void CVAR_Create(cvar_t &Cvar);
#endif
#endif
#ifdef VECTOR_H
	static void Shared_TraceLine(const Vector &vecStart, const Vector &vecEnd, int ignore_monsters, sharedtrace_t &Tr, int CLFlags, int CLIgnoreFlags, edict_t *SVIgnoreEnt = NULL);
	static int Shared_PointContents(const Vector &Origin);
	static float Shared_GetWaterHeight(const Vector &Origin, float minz, float maxz);											 //Find height between a minx and maxs
	static void Shared_PlaySound3D(msstring_ref Sound, float Volume, const Vector &Origin, float Attn);							 //Play sound independent of an entity
	static void Shared_PlaySound3D2(msstring_ref Sound, float Volume, const Vector &Origin, float Attn, int Chan, float sPitch); //MAR2012 Thothie - Same diff + Channel Control
#endif
	static float AngleDiff(float destAngle, float srcAngle);
};

//string_i - In debug mode, a normal string.  In release mode its created within the engine and released each level
#ifndef CONST_H
typedef int string_t;
#endif
/*//#ifdef _DEBUG
	//	#define STRINGI_SETSTR_STR( a )  msstring::operator = ( a )
	//	#define STRINGI_SETSTR_INT( a )  msstring::operator = ( EngineFunc::GetString( a ) )
	//	#define STRINGI_BASECLASS struct string_i  : msstring
	//#else
		#define STRINGI_SETSTR_STR( a )
		#define STRINGI_SETSTR_INT( a )
		#define STRINGI_BASECLASS struct string_i
	//#endif

		STRINGI_BASECLASS
		{
			string_i( )								{ m_string = 0; }

			string_i( msstring_ref a )				{ operator = ( a ); }
			string_i( string_t a )					{ operator = ( a ); }
			string_t operator = ( string_t a )		{ m_string = a; STRINGI_SETSTR_INT( m_string ); return m_string;  }
			operator string_t ( )					{ return m_string; }
			string_i operator = ( msstring_ref a )	{ m_string = EngineFunc::AllocString( a ); STRINGI_SETSTR_STR( a ); return m_string; }
			operator msstring_ref ( )				{ return EngineFunc::GetString( m_string ); }
			bool operator == ( msstring_ref a )	const;
			inline bool operator != ( msstring_ref a )	const { return !operator == ( a ); }

			operator bool ( )						{ return m_string ? true : false; }
			operator ! ( )							{ return !operator bool( ); }
			string_t m_string;
		};*/

//#define string_i msstring
struct string_i
{
	string_i() {}

	string_i(msstring_ref a) { operator=(a); }
	string_i(const msstring &a) { operator=(a); }
	msstring_ref operator=(msstring_ref a)
	{
		m_string = a;
		return m_string;
	}
	msstring_ref operator=(const msstring &a)
	{
		m_string = a;
		return m_string;
	}
	operator msstring_ref() { return m_string; }
	operator const msstring &() { return m_string; }
	bool operator==(msstring_ref a) const { return m_string == a; }
	inline bool operator!=(msstring_ref a) const { return !operator==(a); }

	operator bool() { return m_string.len() ? true : false; }
	bool operator!() { return !operator bool(); }

	msstring_ref c_str() { return m_string.c_str(); }
	size_t len() { return m_string.len(); }

	msstring m_string;
};

#ifdef NOT_HLDLL
#define msnew new
#elif DEV_BUILD
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
