//=========       Copyright © Bernt Andreas Eide!       ============//
//
// Purpose: POSIX friendly include
//
//==================================================================//

#ifndef STREAM_SAFE_H
#define STREAM_SAFE_H

#ifdef _WIN32
#pragma once
#endif

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include <fstream>
#include <iostream>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

#endif // STREAM_SAFE_H