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
// Handle structs - proper value types for AngelScript
//==========================================================================
struct EntityHandle
{
    int value;
    
    EntityHandle() : value(0) {}
    EntityHandle(int v) : value(v) {}
    EntityHandle(const EntityHandle& other) : value(other.value) {}
    
    EntityHandle& operator=(const EntityHandle& other) {
        value = other.value;
        return *this;
    }
    
    bool operator==(const EntityHandle& other) const {
        return value == other.value;
    }
};

struct PlayerHandle
{
    int value;
    
    PlayerHandle() : value(0) {}
    PlayerHandle(int v) : value(v) {}
    PlayerHandle(const PlayerHandle& other) : value(other.value) {}
    
    PlayerHandle& operator=(const PlayerHandle& other) {
        value = other.value;
        return *this;
    }
    
    bool operator==(const PlayerHandle& other) const {
        return value == other.value;
    }
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
    void RegisterEntityHandle(asIScriptEngine* pEngine);
    void RegisterPlayerHandle(asIScriptEngine* pEngine);
    void RegisterEntityHandleMethods(asIScriptEngine* pEngine);  // Call after entity types are registered
    void RegisterString(asIScriptEngine* pEngine);
    void RegisterMathFunctions(asIScriptEngine* pEngine);
}

#endif // AS_CORE_TYPES_H