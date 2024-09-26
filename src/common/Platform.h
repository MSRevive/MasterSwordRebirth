#ifndef COMMON_PLATFORM_H
#define COMMON_PLATFORM_H

// Allow "DEBUG" in addition to default "_DEBUG"
#ifdef _DEBUG
#define DEBUG 1
#endif

// Silence certain warnings
#pragma warning(disable : 4244)	 // int or float down-conversion
#pragma warning(disable : 4305)	 // int or float data truncation
#pragma warning(disable : 4201)	 // nameless struct/union
#pragma warning(disable : 4514)	 // unreferenced inline function removed
#pragma warning(disable : 4100)	 // unreferenced formal parameter
#pragma warning(disable : 26495) // Variable is uninitialized
#pragma warning(disable : 26451) // Arithmetic overflow
#pragma warning(disable : 26812) // The enum type is unscoped

// Misc C-runtime library headers
#include <cctype>
#include <climits>
#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using byte = unsigned char;
using string_t = unsigned int;
using func_t = int;
using vec_t = float;
using word = unsigned short;

#define ARRAYSIZE(p) (sizeof(p) / sizeof(p[0]))
#define V_min(a, b) (((a) < (b)) ? (a) : (b))
#define V_max(a, b) (((a) > (b)) ? (a) : (b))

#endif