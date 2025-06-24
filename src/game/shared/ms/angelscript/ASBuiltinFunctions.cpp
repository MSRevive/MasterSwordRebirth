//==========================================================================
// ASBuiltinFunctions.cpp - Comprehensive Implementation
//
// Built-in function implementations for AngelScript integration
// Converts legacy $-functions to AngelScript global functions
//==========================================================================

#include "ASBuiltinFunctions.h"
#include <angelscript.h>
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
    extern globalvars_t *gpGlobals;
    extern enginefuncs_t g_engfuncs;
#endif
#include <cmath>
#include "../../../server/hl/vector.h"

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
        
        printf("ASBuiltinFunctions: Registering math functions...\n");
        
        // Register math utility functions
        pEngine->RegisterGlobalFunction("float sin(float)", asFUNCTION(AS_Sin), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("float cos(float)", asFUNCTION(AS_Cos), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("float sqrt(float)", asFUNCTION(AS_Sqrt), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("float abs(float)", asFUNCTION(AS_Abs), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("int abs(int)", asFUNCTION(AS_AbsInt), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("float min(float, float)", asFUNCTION(AS_Min), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("float max(float, float)", asFUNCTION(AS_Max), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("int min(int, int)", asFUNCTION(AS_MinInt), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("int max(int, int)", asFUNCTION(AS_MaxInt), asCALL_CDECL);
        
        printf("ASBuiltinFunctions: Math functions registered\n");
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
        
        // Register vector creation and manipulation functions
        pEngine->RegisterGlobalFunction("Vector3 CreateVector(float, float, float)", asFUNCTION(AS_CreateVector), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("float GetVectorX(const Vector3 &in)", asFUNCTION(AS_GetVectorX), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("float GetVectorY(const Vector3 &in)", asFUNCTION(AS_GetVectorY), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("float GetVectorZ(const Vector3 &in)", asFUNCTION(AS_GetVectorZ), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("float Distance(const Vector3 &in, const Vector3 &in)", asFUNCTION(AS_Distance), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("float DotProduct(const Vector3 &in, const Vector3 &in)", asFUNCTION(AS_DotProduct), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("Vector3 CrossProduct(const Vector3 &in, const Vector3 &in)", asFUNCTION(AS_CrossProduct), asCALL_CDECL);
        
        printf("ASBuiltinFunctions: Vector utility functions registered\n");
    }
    
    //==========================================================================
    // Game System Functions
    //==========================================================================
    
    // Get console variable value
    std::string AS_GetCvar(const std::string& name)
    {
#ifdef CLIENT_DLL
        const char* value = CVAR_GET_STRING(name.c_str());
        return value ? std::string(value) : "";
#else
        const char* value = CVAR_GET_STRING(name.c_str());
        return value ? std::string(value) : "";
#endif
    }
    
    // Get current game time
    float AS_GetGameTime()
    {
#ifdef CLIENT_DLL
        // On client, use the engine's client time
        return gEngfuncs.GetClientTime();
#else
        // On server, use gpGlobals->time
        return gpGlobals ? gpGlobals->time : 0.0f;
#endif
    }
    
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
    void* AS_FindEntityByName(const std::string& name)
    {
        // TODO: Implement proper entity finding
        // For now, return nullptr (not implemented)
        return nullptr;
    }
    
    // Get player by index (1-based)
    void* AS_GetPlayerByIndex(int index)
    {
        // TODO: Implement proper player by index lookup
        // For now, return nullptr (not implemented)
        return nullptr;
    }
    
    // Get number of connected players
    int AS_GetPlayerCount()
    {
        // TODO: Implement proper player counting
        // For now, return a placeholder value
        return 1;
    }
    
    // Check if entity reference is valid
    bool AS_IsValidEntity(void* entity)
    {
        // TODO: Implement proper entity validation
        // For now, just check for non-null pointer
        return (entity != nullptr);
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
        
        // Console variable access
        pEngine->RegisterGlobalFunction("string GetCvar(const string &in)", asFUNCTION(AS_GetCvar), asCALL_CDECL);
        
        // Time functions
        pEngine->RegisterGlobalFunction("float GetGameTime()", asFUNCTION(AS_GetGameTime), asCALL_CDECL);
        
        // Random functions
        pEngine->RegisterGlobalFunction("float Random(float, float)", asFUNCTION(AS_Random), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("int RandomInt(int, int)", asFUNCTION(AS_RandomInt), asCALL_CDECL);
        
        // Logging functions
        pEngine->RegisterGlobalFunction("void LogMessage(const string &in)", asFUNCTION(AS_LogMessage), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("void DeveloperMessage(int, const string &in)", asFUNCTION(AS_DeveloperMessage), asCALL_CDECL);
        
        // Entity access functions
        pEngine->RegisterGlobalFunction("EntityHandle@ FindEntityByName(const string &in)", asFUNCTION(AS_FindEntityByName), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("EntityHandle@ GetPlayerByIndex(int)", asFUNCTION(AS_GetPlayerByIndex), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("int GetPlayerCount()", asFUNCTION(AS_GetPlayerCount), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("bool IsValidEntity(EntityHandle@)", asFUNCTION(AS_IsValidEntity), asCALL_CDECL);
        
        // Angle manipulation functions
        pEngine->RegisterGlobalFunction("Vector3 CreateAngles(float, float, float)", asFUNCTION(AS_CreateAngles), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("float GetAnglePitch(const Vector3 &in)", asFUNCTION(AS_GetAnglePitch), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("float GetAngleYaw(const Vector3 &in)", asFUNCTION(AS_GetAngleYaw), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("float GetAngleRoll(const Vector3 &in)", asFUNCTION(AS_GetAngleRoll), asCALL_CDECL);
        
        printf("ASBuiltinFunctions: Game system functions registered\n");
    }
}