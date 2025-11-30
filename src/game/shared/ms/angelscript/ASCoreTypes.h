//==========================================================================
// ASCoreTypes.h - Minimal Implementation
//
// Core type bindings for AngelScript integration
// Provides fundamental types and math functions for script usage
//==========================================================================

#ifndef AS_CORE_TYPES_H
#define AS_CORE_TYPES_H

// Forward declarations
class asIScriptEngine;

// Include cmath for sqrt and other math functions
#include <cmath>

// Include vector.h for Vector class
typedef float vec_t;
#include "hl/vector.h"

// Type aliases for cleaner C++ code
// Vector is aliased to Vector3 for AngelScript
using Vector3 = Vector;

// Color struct for RGBA color values
struct Color
{
    unsigned char r, g, b, a;

    Color() : r(255), g(255), b(255), a(255) {}
    Color(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a = 255)
        : r(_r), g(_g), b(_b), a(_a) {}
};

//==========================================================================
// Core Type Registration Functions - Minimal Implementation
//==========================================================================
namespace ASCoreTypes
{
    // Main registration function - calls all others
    void RegisterAll(asIScriptEngine* pEngine);
    
    // Individual type registration - placeholders for now
    void RegisterVector3(asIScriptEngine* pEngine);
    void RegisterColor(asIScriptEngine* pEngine);
    void RegisterString(asIScriptEngine* pEngine);
    void RegisterMathFunctions(asIScriptEngine* pEngine);
}

#endif // AS_CORE_TYPES_H