#pragma once

// Allow "DEBUG" in addition to default "_DEBUG"
#ifdef _DEBUG
#define DEBUG 1
#endif

// Silence certain warnings
#pragma warning(disable : 4244)	 // int or float down-conversion
#pragma warning(disable : 4305)	 // int or float data truncation
#pragma warning(disable : 4514)	 // unreferenced inline function removed
#pragma warning(disable : 4100)	 // unreferenced formal parameter
#pragma warning(disable : 4201) // nameless struct/union
#pragma warning(disable : 4389) // signed/unsigned mismatch
#pragma warning(disable : 26495) // Variable is uninitialized
#pragma warning(disable : 26451) // Arithmetic overflow
#pragma warning(disable : 26812) // The enum type is unscoped

#include "steam/steamtypes.h" // DAL

// Misc C-runtime library headers
#include <cassert>
#include <cctype>
#include <climits>
#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef WIN32
// Prevent tons of unused windows definitions
#define WIN32_LEAN_AND_MEAN

// Disable all Windows 10 and older APIs otherwise pulled in by Windows.h
#define NOGDICAPMASKS
//#define NOVIRTUALKEYCODES needed for vgui_containerlist
#define NOWINMESSAGES
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCTLMGR
#define NODRAWTEXT
//#define NOGDI required for now
#define NOKERNEL
#define NONLS
//#define NOMB required for now
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX

// Disable additional stuff not covered by the Windows.h list
#define NOWINRES
#define NOIME

// Fix problems with HSPRITE SDK definitions conflicting with Windows header definitions
#define HSPRITE WINDOWS_HSPRITE

#include <windows.h>

#undef HSPRITE
#else
// Add core Linux headers here if you need them
#endif

#ifndef _WIN32
extern "C" char *strlwr(char *str);
#endif

#ifdef POSIX
char* strupr(char* start)
{
	unsigned char *str = (unsigned char*)start;
	while( *str )
	{
		if ( (unsigned char)(*str - 'a') <= ('z' - 'a') )
			*str -= 'a' - 'A';
		else if ( (unsigned char)*str >= 0x80 ) // non-ascii, fall back to CRT
			*str = toupper( *str );
		str++;
	}
	return start;
}

char* strlower(char* start)
{
	unsigned char* str = (unsigned char*)start;
	while (*str)
	{
		if ((unsigned char)(*str - 'A') <= ('Z' - 'A'))
			*str += 'a' - 'A';
		else if ((unsigned char)*str >= 0x80) // non-ascii, fall back to CRT
			*str = tolower(*str);
		str++;
	}
	return start;
}

#define _strupr strupr
#define _strlwr strlower
#define _snprintf snprintf
#define _vsnprintf(a, b, c, d) vsnprintf(a, b, c, d)
#define strnicmp strncasecmp
#define _strnicmp strncasecmp
#define stricmp strcasecmp
#endif

#ifndef MAX_PATH
#define MAX_PATH PATH_MAX
#endif

typedef unsigned char byte;
typedef int qboolean;
typedef int func_t;   //
typedef int string_t; // from engine's pr_comp.h;
typedef float vec_t;  // needed before including progdefs.h

#define clrmem(a) memset(&a, 0, sizeof(a));

#define V_min(a, b) (((a) < (b)) ? (a) : (b))
#define V_max(a, b) (((a) > (b)) ? (a) : (b))
