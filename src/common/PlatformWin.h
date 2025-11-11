#pragma once

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#define NOWINRES
#define NOSERVICE
#define NOMCX
#define NOIME
#define NOMINMAX
#define NOMB
#define NOGDI  // Prevent GetObject macro definition that conflicts with AngelScript

#pragma push_macro("SERVER_EXECUTE")
#ifdef SERVER_EXECUTE
#undef SERVER_EXECUTE
#endif

//Fix problems with ARRAYSIZE and HSPRITE SDK definitions conflicting with Windows header definitions
//TODO: ARRAYSIZE should be replaced with std::size, which is a superior replacement
#pragma push_macro("ARRAYSIZE")
#ifdef ARRAYSIZE
#undef ARRAYSIZE
#endif

#include <Windows.h>

#pragma pop_macro("ARRAYSIZE")

// Undefine problematic Windows macros that conflict with AngelScript/asbind20
#ifdef GetObject
#undef GetObject
#endif

#endif
