#ifndef COMMON_PLATFORM_WIN_H
#define COMMON_PLATFORM_WIN_H

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#define NOWINRES
#define NOSERVICE
#define NOMCX
#define NOIME
#define NOMINMAX
#define NOMB
#define NOGDI  // Prevent GetObject macro definition that conflicts with AngelScript

#pragma push_macro("ARRAYSIZE")
#ifdef ARRAYSIZE
#undef ARRAYSIZE
#endif

#pragma push_macro("SERVER_EXECUTE")
#ifdef SERVER_EXECUTE
#undef SERVER_EXECUTE
#endif

#include <Windows.h>

// Undefine problematic Windows macros that conflict with AngelScript/asbind20
#ifdef GetObject
#undef GetObject
#endif

#endif

#endif