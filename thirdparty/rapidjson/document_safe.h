//=========       Copyright © Bernt Andreas Eide!       ============//
//
// Purpose: Safe document include, POSIX systems will break when including the raw document.h
// because cbase.h has different macros which collide with macros used in the RAPIDJSON lib.
//
//==================================================================//

#ifndef JSON_DOC_SAFE_H
#define JSON_DOC_SAFE_H

#ifdef _WIN32
#pragma once
#endif

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include "document.h" // <- RapidJSON

// RE-include necessary stuff:

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include "minmax.h" // Revert

#endif // JSON_DOC_SAFE_H