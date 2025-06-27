#ifndef ASBINDINGS_H
#define ASBINDINGS_H

//==========================================================================
// AngelScript Integration Layer
// 
// Master registration and coordination for all AngelScript bindings
//==========================================================================

class asIScriptEngine;

// Forward declarations for component systems
class ASModuleSystem;
//==========================================================================
// ASBindings - Master Integration Layer
//==========================================================================
class ASBindings
{
public:
    // Master registration function for all bindings
    static bool RegisterAll(asIScriptEngine* pEngine);
    
    // Registration functions for each category
    static bool RegisterCoreTypes(asIScriptEngine* pEngine);
    static bool RegisterBuiltinFunctions(asIScriptEngine* pEngine);
    static bool RegisterScriptClasses(asIScriptEngine* pEngine);
    static bool RegisterCoroutineFunctions(asIScriptEngine* pEngine);
    static bool RegisterMemoryOptimization(asIScriptEngine* pEngine);
    static bool RegisterModuleSystem(asIScriptEngine* pEngine);
    
    // Validation and diagnostics
    static bool ValidateRegistrations(asIScriptEngine* pEngine);
    static void LogRegistrationInfo(asIScriptEngine* pEngine);
    
private:
    // Prevent instantiation
    ASBindings() = delete;
    ~ASBindings() = delete;
    ASBindings(const ASBindings&) = delete;
    ASBindings& operator=(const ASBindings&) = delete;
};

#endif // ASBINDINGS_H