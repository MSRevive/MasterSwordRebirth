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
    void RegisterEntityHandle(asIScriptEngine* pEngine);
    void RegisterString(asIScriptEngine* pEngine);
    void RegisterMathFunctions(asIScriptEngine* pEngine);
}

#endif // AS_CORE_TYPES_H