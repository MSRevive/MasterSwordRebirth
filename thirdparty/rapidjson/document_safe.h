//=========       Copyright © Bernt Andreas Eide!       ============//
//
// Purpose: POSIX friendly include
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

#include "document.h"
#include "stringbuffer.h"
#include "writer.h"
#include "filewritestream.h"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a ,b) (((a) > (b)) ? (a) : (b))

#endif // JSON_DOC_SAFE_H