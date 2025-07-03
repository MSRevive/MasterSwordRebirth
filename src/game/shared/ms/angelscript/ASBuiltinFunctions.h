//==========================================================================
// ASBuiltinFunctions.h - Comprehensive Implementation
//
// Built-in function implementations for AngelScript integration
// Converts legacy $-functions to AngelScript global functions
//==========================================================================

#ifndef AS_BUILTIN_FUNCTIONS_H
#define AS_BUILTIN_FUNCTIONS_H

// Forward declarations
class asIScriptEngine;

//==========================================================================
// Built-in Functions Registration - Comprehensive Implementation
//==========================================================================
namespace ASBuiltinFunctions
{
    // Main registration function
    void RegisterAll(asIScriptEngine* pEngine);
    
    // Individual registration functions
    void RegisterStringFunctions(asIScriptEngine* pEngine);
    void RegisterMathFunctions(asIScriptEngine* pEngine);
    void RegisterVectorUtilityFunctions(asIScriptEngine* pEngine);
    void RegisterGameFunctions(asIScriptEngine* pEngine);
}

#endif // AS_BUILTIN_FUNCTIONS_H