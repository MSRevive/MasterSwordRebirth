//==========================================================================
// ASBuiltinFunctions.h - Comprehensive Implementation
//
// Built-in function implementations for AngelScript integration
// Converts legacy $-functions to AngelScript global functions
//==========================================================================

#ifndef AS_BUILTIN_FUNCTIONS_H
#define AS_BUILTIN_FUNCTIONS_H

#include <string>
#include <angelscript.h>
#include <angelscript/addons/scriptarray/scriptarray.h>

// Forward declarations
class asIScriptEngine;
class asIScriptFunction;
class CScriptArray;
class Vector; // Master Sword's Vector class

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
    
    //==========================================================================
    // Core Utility Functions - Available to GameMaster Scripts
    //==========================================================================
    
    // Time & Game State Functions
    // float GetGameTime() - Available in ASEngineBindings
    // string GetMapName() - Available in ASEngineBindings  
    // string GetCvar(string) - Available in ASEngineBindings
    std::string AS_GetTimestamp();
    void AS_LogMessage(const std::string& message);
    void AS_DeveloperMessage(int level, const std::string& message);
    
    // String Utility Functions  
    class CScriptArray* AS_Split(const std::string& input, const std::string& delim, asIScriptEngine* engine);
    class CScriptArray* AS_SplitWrapper(const std::string& input, const std::string& delim);
    std::string AS_FormatInt(int value);
    std::string AS_FormatFloat(float value, const std::string& options, int width, int precision);
    
    // Math Functions (complement scriptmath addon)
    float AS_Min(float a, float b);
    float AS_Max(float a, float b);
    int AS_MinInt(int a, int b);
    int AS_MaxInt(int a, int b);
    int AS_AbsInt(int value);
    // float ceil(float) - Available in scriptmath addon
    int AS_RandomInt(int min, int max);
    float AS_Random(float min, float max);
    
    // Vector Functions
    // Vector CreateVector(float, float, float) - Available as Vector constructor
    // Vector CreateAngles(float, float, float) - Available below
    Vector AS_CreateVector(float x, float y, float z);
    Vector AS_CreateAngles(float pitch, float yaw, float roll);
    
    //==========================================================================
    // GameMaster Communication Functions
    //==========================================================================
    
    // Player Communication
    void AS_SendPlayerMessage(const std::string& playerID, const std::string& message);
    void AS_SendConsoleMessage(const std::string& playerID, const std::string& message);
    void AS_SendInfoMessageToAll(const std::string& title, const std::string& message);
    void AS_SendMessageToAllPlayers(const std::string& color, const std::string& message);
    
    // External System Integration
    void AS_CallPlayerExternal(const std::string& playerID, const std::string& function, CScriptArray* args);
    void AS_CallGameMasterExternal(const std::string& function, CScriptArray* args);
    
    // Event System Functions
    void AS_RegisterEngineEvent(const std::string& eventName, asIScriptFunction* callback);
    void AS_LogEngineEventHandlers();
    void TriggerEngineEvent(const std::string& eventName, CScriptArray* args = nullptr);
    void TriggerCustomEngineEvent(const std::string& eventName, CScriptArray* args = nullptr);
    
    // Advanced System Functions
    void AS_InitializeAdvancedTriggerSystem();
    void AS_InitializeHPSequenceTrigger();
    void AS_InitializeEntitySpawner();
    void AS_InitializeEntityCommunications();
    void AS_ShutdownAdvancedTriggerSystem();
    void AS_ShutdownHPSequenceTrigger();
    void AS_ShutdownEntitySpawner();
    void AS_ShutdownEntityCommunications();
    
    // Advanced System Status Functions
    bool AS_IsAdvancedTriggerSystemActive();
    bool AS_IsHPSequenceSystemActive();
    bool AS_IsEntitySpawnerActive();
    bool AS_IsEntityCommSystemActive();
    int AS_GetActiveTriggersCount();
    int AS_GetActiveSequencesCount();
    
    // GameMaster Registration Function
    void RegisterGameMasterFunctions(asIScriptEngine* pEngine);
}

#endif // AS_BUILTIN_FUNCTIONS_H