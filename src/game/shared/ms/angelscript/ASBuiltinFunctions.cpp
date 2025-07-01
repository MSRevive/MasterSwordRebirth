//==========================================================================
// ASBuiltinFunctions.cpp - Using asbind20
//
// Built-in function implementations for AngelScript integration
// Converts legacy $-functions to AngelScript global functions
//==========================================================================

#include "ASBuiltinFunctions.h"
#include <asbind20/asbind.hpp>
#include <cstdio>
#include <string>
#include <algorithm>
#include <cctype>
#include <cstdlib>  // For rand()

// Master Sword specific includes
#ifdef CLIENT_DLL
    #include "hud.h"
    #include "cl_util.h"
    #include "cl_dll.h"
    extern cl_enginefunc_t gEngfuncs;
#else
    #include "extdll.h"
    #include "util.h"
    #include "cbase.h"
    #include "player/player.h"
    extern globalvars_t *gpGlobals;
    extern enginefuncs_t g_engfuncs;
#endif
#include <cmath>
#include "../../../server/hl/vector.h"
#include <angelscript/addons/scriptarray/scriptarray.h>
#include "mslogger.h"

// Forward declare EntityHandle struct (defined in ASCoreTypes.cpp)
struct EntityHandle
{
    int value;
    EntityHandle() : value(0) {}
    EntityHandle(int v) : value(v) {}
};

// Helper function to get player by index - implemented differently on server vs client
static CBasePlayer* GetPlayerByIndexHelper(int index)
{
#ifdef CLIENT_DLL
    return nullptr; // Client doesn't have access to other players
#else
    // Server-side implementation
    if (index < 1 || index > gpGlobals->maxClients) {
        return nullptr;
    }
    
    // Use engine function to get edict
    edict_t* pEdict = g_engfuncs.pfnPEntityOfEntIndex(index);
    if (!pEdict || pEdict->free) {
        return nullptr;
    }
    
    // Get the entity from the edict
    CBaseEntity* pEntity = CBaseEntity::Instance(pEdict);
    if (!pEntity) {
        return nullptr;
    }
    
    // Check if it's a player
    if (!(pEntity->edict()->v.flags & FL_CLIENT)) {
        return nullptr;
    }
    
    return static_cast<CBasePlayer*>(pEntity);
#endif
}

//==========================================================================
// Built-in Functions Registration - Comprehensive Implementation
//==========================================================================
namespace ASBuiltinFunctions
{
    void RegisterAll(asIScriptEngine* pEngine)
        {
            if (!pEngine) {
                printf("ASBuiltinFunctions::RegisterAll: NULL engine pointer!\n");
                return;
            }
            
            printf("ASBuiltinFunctions: Registering comprehensive builtin functions...\n");
            
            // Register string functions
            RegisterStringFunctions(pEngine);
            
            // Register math utility functions  
            RegisterMathFunctions(pEngine);
            
            // Register vector creation and utility functions
            RegisterVectorUtilityFunctions(pEngine);
            
            // Register game system functions
            RegisterGameFunctions(pEngine);
            
            printf("ASBuiltinFunctions: Registration complete\n");
        }
    
    //==========================================================================
    // String Manipulation Functions
    //==========================================================================
    
    // C++ wrapper functions for AngelScript string operations
    std::string AS_Left(const std::string& str, int count)
    {
        if (count <= 0 || str.empty()) return "";
        if (count >= (int)str.length()) return str;
        return str.substr(0, count);
    }
    
    std::string AS_Right(const std::string& str, int count)
    {
        if (count <= 0 || str.empty()) return "";
        if (count >= (int)str.length()) return str;
        return str.substr(str.length() - count);
    }
    
    std::string AS_Mid(const std::string& str, int start, int count)
    {
        if (start < 0 || count <= 0 || str.empty() || start >= (int)str.length()) return "";
        if (start + count > (int)str.length()) count = str.length() - start;
        return str.substr(start, count);
    }
    
    int AS_Length(const std::string& str)
    {
        return (int)str.length();
    }
    
    std::string AS_ToUpper(const std::string& str)
    {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    }
    
    std::string AS_ToLower(const std::string& str)
    {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }
    
    std::string AS_Replace(const std::string& str, const std::string& find, const std::string& replace)
    {
        if (find.empty()) return str;
        
        std::string result = str;
        size_t pos = 0;
        while ((pos = result.find(find, pos)) != std::string::npos) {
            result.replace(pos, find.length(), replace);
            pos += replace.length();
        }
        return result;
    }
    
    void RegisterStringFunctions(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        printf("ASBuiltinFunctions: Registering string functions...\n");
        
        // Register string manipulation functions
        pEngine->RegisterGlobalFunction("string Left(const string &in, int)", asFUNCTION(AS_Left), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("string Right(const string &in, int)", asFUNCTION(AS_Right), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("string Mid(const string &in, int, int)", asFUNCTION(AS_Mid), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("int Length(const string &in)", asFUNCTION(AS_Length), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("string ToUpper(const string &in)", asFUNCTION(AS_ToUpper), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("string ToLower(const string &in)", asFUNCTION(AS_ToLower), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("string Replace(const string &in, const string &in, const string &in)", asFUNCTION(AS_Replace), asCALL_CDECL);
        
        printf("ASBuiltinFunctions: String functions registered\n");
    }
    
    //==========================================================================
    // Math Utility Functions
    //==========================================================================
    
    // C++ wrapper functions for AngelScript math operations
    float AS_Sin(float angle)
    {
        return sinf(angle);
    }
    
    float AS_Cos(float angle)
    {
        return cosf(angle);
    }
    
    float AS_Sqrt(float value)
    {
        return value >= 0.0f ? sqrtf(value) : 0.0f;
    }
    
    float AS_Abs(float value)
    {
        return fabsf(value);
    }
    
    int AS_AbsInt(int value)
    {
        return abs(value);
    }
    
    float AS_Min(float a, float b)
    {
        return (a < b) ? a : b;
    }
    
    float AS_Max(float a, float b)
    {
        return (a > b) ? a : b;
    }
    
    int AS_MinInt(int a, int b)
    {
        return (a < b) ? a : b;
    }
    
    int AS_MaxInt(int a, int b)
    {
        return (a > b) ? a : b;
    }
    
    void RegisterMathFunctions(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        printf("ASBuiltinFunctions: Registering additional math functions...\n");
        
        // Register only additional math functions that ASCoreTypes doesn't provide
        // Note: Basic functions (sin, cos, sqrt, abs, min, max) are registered in ASCoreTypes
        pEngine->RegisterGlobalFunction("int abs(int)", asFUNCTION(AS_AbsInt), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("int min(int, int)", asFUNCTION(AS_MinInt), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("int max(int, int)", asFUNCTION(AS_MaxInt), asCALL_CDECL);
        
        printf("ASBuiltinFunctions: Additional math functions registered\n");
    }
    
    //==========================================================================
    // Vector Utility Functions
    //==========================================================================
    
    // C++ wrapper functions for Vector3 creation and manipulation
    Vector AS_CreateVector(float x, float y, float z)
    {
        return Vector(x, y, z);
    }
    
    float AS_GetVectorX(const Vector& vec)
    {
        return vec.x;
    }
    
    float AS_GetVectorY(const Vector& vec)
    {
        return vec.y;
    }
    
    float AS_GetVectorZ(const Vector& vec)
    {
        return vec.z;
    }
    
    float AS_Distance(const Vector& vec1, const Vector& vec2)
    {
        Vector diff = vec1 - vec2;
        return diff.Length();
    }
    
    float AS_DotProduct(const Vector& vec1, const Vector& vec2)
    {
        return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z;
    }
    
    Vector AS_CrossProduct(const Vector& vec1, const Vector& vec2)
    {
        return Vector(
            vec1.y * vec2.z - vec1.z * vec2.y,
            vec1.z * vec2.x - vec1.x * vec2.z,
            vec1.x * vec2.y - vec1.y * vec2.x
        );
    }
    
    void RegisterVectorUtilityFunctions(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        printf("ASBuiltinFunctions: Registering vector utility functions...\n");
        
        // Register vector creation and manipulation functions with asbind20
        asbind20::global(pEngine)
            .function("Vector3 CreateVector(float, float, float)", &AS_CreateVector)
            .function("float GetVectorX(const Vector3 &in)", &AS_GetVectorX)
            .function("float GetVectorY(const Vector3 &in)", &AS_GetVectorY)
            .function("float GetVectorZ(const Vector3 &in)", &AS_GetVectorZ)
            .function("float Distance(const Vector3 &in, const Vector3 &in)", &AS_Distance)
            .function("float DotProduct(const Vector3 &in, const Vector3 &in)", &AS_DotProduct);
        // Note: CrossProduct is registered in ASCoreTypes, not here
        
        printf("ASBuiltinFunctions: Vector utility functions registered\n");
    }
    
    //==========================================================================
    // Game System Functions
    //==========================================================================
    
    // Note: AS_GetCvar and AS_GetGameTime are now implemented in ASEntityBindings.cpp
    // to ensure proper engine integration and avoid duplication
    
    // Random float between min and max
    float AS_Random(float min, float max)
    {
#ifdef CLIENT_DLL
        // Client-side random
        return min + (max - min) * ((float)rand() / RAND_MAX);
#else
        // Server-side uses engine's RANDOM_FLOAT
        return RANDOM_FLOAT(min, max);
#endif
    }
    
    // Random integer between min and max (inclusive)
    int AS_RandomInt(int min, int max)
    {
#ifdef CLIENT_DLL
        // Client-side random
        return min + (rand() % (max - min + 1));
#else
        // Server-side uses engine's RANDOM_LONG
        return RANDOM_LONG(min, max);
#endif
    }
    
    // Write message to game log/console
    void AS_LogMessage(const std::string& msg)
    {
#ifdef CLIENT_DLL
        gEngfuncs.Con_Printf("%s\n", msg.c_str());
#else
        ALERT(at_console, "%s\n", msg.c_str());
#endif
    }
    
    // Write developer console message
    void AS_DeveloperMessage(int level, const std::string& msg)
    {
#ifdef CLIENT_DLL
        float devValue = gEngfuncs.pfnGetCvarFloat("developer");
        if (devValue >= level) {
            gEngfuncs.Con_DPrintf("%s\n", msg.c_str());
        }
#else
        float devValue = CVAR_GET_FLOAT("developer");
        if (devValue >= level) {
            ALERT(at_console, "%s\n", msg.c_str());
        }
#endif
    }
    
    // Find entity by targetname
    EntityHandle AS_FindEntityByName(const std::string& name)
    {
        MS_ANGEL_DEBUG("FindEntityByName: Searching for '%s'", name.c_str());
        // TODO: Implement proper entity finding
        // For now, return an invalid handle
        return EntityHandle(0);
    }
    
    // Get player by index (1-based)
    CBasePlayer* AS_GetPlayerByIndex(int index)
    {
        #ifdef CLIENT_DLL
            // Client-side: Limited player access
            MS_ANGEL_DEBUG("GetPlayerByIndex called on client (index: %d)", index);
            return nullptr;
        #else
            // Server-side: Real player lookup
            if (index < 1 || index > gpGlobals->maxClients) {
                MS_ANGEL_DEBUG("GetPlayerByIndex: Invalid index %d (max: %d)", index, gpGlobals->maxClients);
                return nullptr;
            }
            
            CBasePlayer* pPlayer = GetPlayerByIndexHelper(index);
            if (pPlayer && pPlayer->edict() && !pPlayer->edict()->free) {
                MS_ANGEL_DEBUG("GetPlayerByIndex: Found player %d (%s)", index, STRING(pPlayer->edict()->v.netname));
                return pPlayer;
            }
            
            MS_ANGEL_DEBUG("GetPlayerByIndex: No valid player at index %d", index);
            return nullptr;
        #endif
    }
    
    // Get number of connected players
    int AS_GetPlayerCount()
    {
        #ifdef CLIENT_DLL
            // Client-side: Return 1 (self)
            return 1;
        #else
            // Server-side: Count connected players
            int playerCount = 0;
            for (int i = 1; i <= gpGlobals->maxClients; i++) {
                CBasePlayer* pPlayer = GetPlayerByIndexHelper(i);
                if (pPlayer && pPlayer->edict() && !pPlayer->edict()->free) {
                    // Additional check: ensure player is fully connected
                    if (pPlayer->edict()->v.flags & FL_CLIENT) {
                        playerCount++;
                    }
                }
            }
            
            MS_ANGEL_DEBUG("GetPlayerCount: %d connected players", playerCount);
            return playerCount;
        #endif
    }
    
    // Get all connected players as an array
    CScriptArray* AS_GetAllPlayers(asIScriptEngine* engine)
    {
        // Get the array type for CBasePlayer@[]
        asITypeInfo* arrayType = engine->GetTypeInfoByDecl("array<CBasePlayer@>");
        if (!arrayType) {
            MS_ANGEL_ERROR("Failed to get array<CBasePlayer@> type for GetAllPlayers");
            return nullptr;
        }
        
        // Create new array
        CScriptArray* playerArray = CScriptArray::Create(arrayType);
        
        #ifndef CLIENT_DLL
            // Server-side: Populate with real players
            for (int i = 1; i <= gpGlobals->maxClients; i++) {
                CBasePlayer* pPlayer = GetPlayerByIndexHelper(i);
                if (pPlayer && pPlayer->edict() && !pPlayer->edict()->free) {
                    if (pPlayer->edict()->v.flags & FL_CLIENT) {
                        // Add player to array
                        playerArray->Resize(playerArray->GetSize() + 1);
                        *(CBasePlayer**)playerArray->At(playerArray->GetSize() - 1) = pPlayer;
                    }
                }
            }
            
            MS_ANGEL_INFO("GetAllPlayers: Returning %d players", playerArray->GetSize());
        #else
            // Client-side: Empty array
            MS_ANGEL_DEBUG("GetAllPlayers called on client side");
        #endif
        
        return playerArray;
    }

    // Check if entity reference is valid
    bool AS_IsValidEntity(EntityHandle handle)
    {
        MS_ANGEL_DEBUG("IsValidEntity: Checking handle %d", handle.value);
        // TODO: Implement proper entity validation
        return handle.value != 0;
    }
    
    // Create angle vector
    Vector AS_CreateAngles(float pitch, float yaw, float roll)
    {
        return Vector(pitch, yaw, roll);
    }
    
    // Get pitch component from angles
    float AS_GetAnglePitch(const Vector& angles)
    {
        return angles.x;
    }
    
    // Get yaw component from angles
    float AS_GetAngleYaw(const Vector& angles)
    {
        return angles.y;
    }
    
    // Get roll component from angles
    float AS_GetAngleRoll(const Vector& angles)
    {
        return angles.z;
    }
    
    void RegisterGameFunctions(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        printf("ASBuiltinFunctions: Registering game system functions...\n");
        
        // Note: GetCvar and GetGameTime are now registered in ASEntityBindings.cpp
        // to avoid duplicate registrations and ensure proper engine integration
        
        // Register game functions with asbind20 - simplified to isolate assertion issue
        asbind20::global(pEngine)
            // Random functions
            .function("float Random(float, float)", &AS_Random)
            .function("int RandomInt(int, int)", &AS_RandomInt)
            // Logging functions
            .function("void LogMessage(const string &in)", &AS_LogMessage)
            .function("void DeveloperMessage(int, const string &in)", &AS_DeveloperMessage)
            // Basic functions only for now
            .function("int GetPlayerCount()", &AS_GetPlayerCount)
            // Angle manipulation functions
            .function("Vector3 CreateAngles(float, float, float)", &AS_CreateAngles)
            .function("float GetAnglePitch(const Vector3 &in)", &AS_GetAnglePitch)
            .function("float GetAngleYaw(const Vector3 &in)", &AS_GetAngleYaw)
            .function("float GetAngleRoll(const Vector3 &in)", &AS_GetAngleRoll);
        
        // Register GetAllPlayers function separately since it needs engine parameter
        pEngine->RegisterGlobalFunction("array<CBasePlayer@>@ GetAllPlayers()", 
            asFUNCTION(AS_GetAllPlayers), asCALL_CDECL);
        
        printf("ASBuiltinFunctions: Game system functions registered\n");
    }
}