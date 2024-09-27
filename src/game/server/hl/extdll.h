/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#ifndef EXTDLL_H
#define EXTDLL_H

//
// Global header file for extension DLLs
//

#include "Platform.h"

// Prevent tons of unused windows definitions
#ifdef _WIN32
#include "PlatformWin.h"
#else // _WIN32
#define FALSE 0
#define TRUE 1

typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned long ULONG;
typedef unsigned int uint;
typedef int BOOL;

#include <limits.h>
#include <stdarg.h>

#ifndef MAX_PATH
#define MAX_PATH PATH_MAX
#endif

#include "strhelper.h"
#endif //_WIN32

// Misc C-runtime library headers
#include "stdio.h"
#include "stdlib.h"
#include "math.h"

// Vector class
#include "vector.h"

// Shared engine/DLL constants
#include "const.h"
#include "progdefs.h"
#include "edict.h"

// Shared header describing protocol between engine and DLLs
#include "eiface.h"

// Shared header between the client DLL and the game DLLs
#include "cdll_dll.h"

#endif //EXTDLL_H
