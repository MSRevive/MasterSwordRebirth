//==========================================================================
// ASBuiltinFunctions.cpp - Using asbind20
//
// Built-in function implementations for AngelScript integration
// Converts legacy $-functions to AngelScript global functions
//==========================================================================

#include "ASBuiltinFunctions.h"
#include "ASEngineInterface.h"
#include <asbind20/asbind.hpp>
#include <angelscript.h>  // For asGetActiveContext
#include <angelscript/addons/scriptarray/scriptarray.h>
#include <cstdio>
#include <string>
#include <algorithm>
#include <cctype>
#include <cstdlib>  // For rand()
#include <ctime>    // For time functions

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
    // Note: gmsgSayText is handled via ASEngineProvider pattern
#endif
#include <cmath>
#include <map>
#include "../../../server/hl/vector.h"
#include "mslogger.h"
#include "ASEngineEventManager.h"

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
            
            // Register GameMaster communication and event functions
            RegisterGameMasterFunctions(pEngine);
            
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
        
        // Register math functions with asbind20
        // Note: Basic float functions (sin, cos, sqrt, abs, ceil, min, max) are registered in ASCoreTypes.cpp
        // We add integer versions and additional utility functions
        asbind20::global(pEngine)
            // Integer math functions to complement float versions from ASCoreTypes
            .function("int abs(int)", &AS_AbsInt)
            .function("int min(int, int)", &AS_MinInt)
            .function("int max(int, int)", &AS_MaxInt);
        
        printf("ASBuiltinFunctions: Additional math functions registered\n");
    }
    
    //==========================================================================
    // Vector Utility Functions
    //==========================================================================
    
    // C++ wrapper functions for Vector creation and manipulation
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
    
    // Get formatted timestamp string
    std::string AS_GetTimestamp()
    {
        time_t rawtime;
        struct tm* timeinfo;
        char buffer[80];
        
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        return std::string(buffer);
    }
    
    // Split string by delimiter and return array
    CScriptArray* AS_Split(const std::string& input, const std::string& delim, asIScriptEngine* engine)
    {
        // Get array type for string[]
        asITypeInfo* arrayType = engine->GetTypeInfoByDecl("array<string>");
        if (!arrayType) {
            MS_ANGEL_ERROR("Failed to get array<string> type for split function");
            return nullptr;
        }
        
        // Create new array
        CScriptArray* resultArray = CScriptArray::Create(arrayType, (asUINT)0);
        
        if (input.empty()) {
            return resultArray; // Return empty array
        }
        
        if (delim.empty()) {
            // If delimiter is empty, split into individual characters
            for (size_t i = 0; i < input.length(); i++) {
                std::string charStr = input.substr(i, 1);
                resultArray->Resize(resultArray->GetSize() + 1);
                *(std::string*)resultArray->At(resultArray->GetSize() - 1) = charStr;
            }
            return resultArray;
        }
        
        // Split by delimiter
        size_t start = 0;
        size_t pos = input.find(delim);
        
        while (pos != std::string::npos) {
            std::string token = input.substr(start, pos - start);
            resultArray->Resize(resultArray->GetSize() + 1);
            *(std::string*)resultArray->At(resultArray->GetSize() - 1) = token;
            
            start = pos + delim.length();
            pos = input.find(delim, start);
        }
        
        // Add the last token
        std::string lastToken = input.substr(start);
        resultArray->Resize(resultArray->GetSize() + 1);
        *(std::string*)resultArray->At(resultArray->GetSize() - 1) = lastToken;
        
        return resultArray;
    }
    
    // Format integer to string
    std::string AS_FormatInt(int value)
    {
        char buffer[32];
        sprintf(buffer, "%d", value);
        return std::string(buffer);
    }
    
    // Format float to string with options
    std::string AS_FormatFloat(float value, const std::string& options, int width, int precision)
    {
        char buffer[128];
        
        // Parse options for formatting style
        bool scientific = (options.find('e') != std::string::npos || options.find('E') != std::string::npos);
        bool fixed = (options.find('f') != std::string::npos || options.find('F') != std::string::npos);
        bool general = (options.find('g') != std::string::npos || options.find('G') != std::string::npos);
        
        // Default to general format if none specified
        if (!scientific && !fixed && !general) {
            general = true;
        }
        
        // Clamp precision and width to reasonable ranges
        precision = (precision < 0) ? 6 : ((precision > 20) ? 20 : precision);
        width = (width < 0) ? 0 : ((width > 100) ? 100 : width);
        
        // Create format string
        char formatStr[32];
        if (scientific) {
            if (width > 0) {
                sprintf(formatStr, "%%%d.%de", width, precision);
            } else {
                sprintf(formatStr, "%%.%de", precision);
            }
        } else if (fixed) {
            if (width > 0) {
                sprintf(formatStr, "%%%d.%df", width, precision);
            } else {
                sprintf(formatStr, "%%.%df", precision);
            }
        } else { // general
            if (width > 0) {
                sprintf(formatStr, "%%%d.%dg", width, precision);
            } else {
                sprintf(formatStr, "%%.%dg", precision);
            }
        }
        
        sprintf(buffer, formatStr, value);
        return std::string(buffer);
    }
    
    // Wrapper function for split that automatically gets engine
    CScriptArray* AS_SplitWrapper(const std::string& input, const std::string& delim)
    {
        // Get current engine from context - this is a common pattern in AngelScript
        asIScriptContext* ctx = asGetActiveContext();
        if (!ctx) {
            MS_ANGEL_ERROR("AS_SplitWrapper: No active script context");
            return nullptr;
        }
        
        asIScriptEngine* engine = ctx->GetEngine();
        if (!engine) {
            MS_ANGEL_ERROR("AS_SplitWrapper: Failed to get engine from context");
            return nullptr;
        }
        
        return AS_Split(input, delim, engine);
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
        // Use multiple output methods to ensure visibility
        ALERT(at_console, "[AS] %s\n", msg.c_str());
        
        // And log to file using MSLogger
        MS_ANGEL_INFO("[AS] %s", msg.c_str());
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
        CScriptArray* playerArray = CScriptArray::Create(arrayType, (asUINT)0);
        
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
        
        // Register game functions with asbind20 - comprehensive implementation
        // Note: GetTimestamp, GetPlayerCount, GetAllPlayers are registered in ASEntityBindings.cpp
        // Note: formatInt is provided by scriptstdstring addon with enhanced functionality
        asbind20::global(pEngine)
            // String utility functions
            .function("string formatFloat(float, const string &in, int, int)", &AS_FormatFloat)
            // Random functions
            .function("float Random(float, float)", &AS_Random)
            .function("int RandomInt(int, int)", &AS_RandomInt)
            // Logging functions
            .function("void LogMessage(const string &in)", &AS_LogMessage)
            .function("void DeveloperMessage(int, const string &in)", &AS_DeveloperMessage)
            // Angle manipulation functions
            .function("Vector3 CreateAngles(float, float, float)", &AS_CreateAngles)
            .function("float GetAnglePitch(const Vector3 &in)", &AS_GetAnglePitch)
            .function("float GetAngleYaw(const Vector3 &in)", &AS_GetAngleYaw)
            .function("float GetAngleRoll(const Vector3 &in)", &AS_GetAngleRoll);
        
        // Note: GetAllPlayers is registered in ASEntityBindings.cpp to avoid duplication
        
        pEngine->RegisterGlobalFunction("array<string>@ split(const string &in, const string &in)", 
            asFUNCTION(AS_SplitWrapper), asCALL_CDECL);
        
        printf("ASBuiltinFunctions: Game system functions registered\n");
    }
    
    //==========================================================================
    // GameMaster Communication Functions Implementation
    //==========================================================================
    
    // Find player by Steam ID helper
    CBasePlayer* FindPlayerBySteamID(const std::string& steamID)
    {
        #ifdef CLIENT_DLL
            return nullptr; // Client doesn't have access to other players
        #else
            if (steamID.empty()) {
                return nullptr;
            }
            
            for (int i = 1; i <= gpGlobals->maxClients; i++) {
                CBasePlayer* pPlayer = GetPlayerByIndexHelper(i);
                if (pPlayer && pPlayer->edict() && !pPlayer->edict()->free && (pPlayer->edict()->v.flags & FL_CLIENT)) {
                    // Get player's Steam ID through engine
                    std::string playerSteamID = ASEngineProvider::GetPlayerAuthId((void*)pPlayer);
                    if (playerSteamID == steamID) {
                        return pPlayer;
                    }
                }
            }
            return nullptr;
        #endif
    }
    
    // Send chat message to specific player
    void AS_SendPlayerMessage(const std::string& playerID, const std::string& message)
    {
        if (playerID.empty() || message.empty()) {
            MS_ANGEL_ERROR("SendPlayerMessage: Empty playerID or message");
            return;
        }
        
        // Security check: Prevent message injection
        if (message.find('\n') != std::string::npos || message.find('\r') != std::string::npos) {
            MS_ANGEL_ERROR("SendPlayerMessage: Message contains invalid characters (newlines)");
            return;
        }
        
        // Length check: Prevent overly long messages
        if (message.length() > 256) {
            MS_ANGEL_ERROR("SendPlayerMessage: Message too long (%d characters, max 256)", (int)message.length());
            return;
        }
        
        // Use ASEngineProvider for cross-platform compatibility
        ASEngineProvider::SendPlayerMessage(playerID, message);
        MS_ANGEL_DEBUG("SendPlayerMessage: Sent message to %s: %s", playerID.c_str(), message.c_str());
    }
    
    // Send console message to specific player
    void AS_SendConsoleMessage(const std::string& playerID, const std::string& message)
    {
        if (playerID.empty() || message.empty()) {
            MS_ANGEL_ERROR("SendConsoleMessage: Empty playerID or message");
            return;
        }
        
        // Security check: Prevent command injection
        if (message.find(';') != std::string::npos || message.find('\n') != std::string::npos || 
            message.find('\r') != std::string::npos || message.find('"') != std::string::npos) {
            MS_ANGEL_ERROR("SendConsoleMessage: Message contains potentially dangerous characters");
            return;
        }
        
        // Length check
        if (message.length() > 512) {
            MS_ANGEL_ERROR("SendConsoleMessage: Console message too long (%d characters, max 512)", (int)message.length());
            return;
        }
        
        #ifdef CLIENT_DLL
            MS_ANGEL_DEBUG("SendConsoleMessage: Client attempted console message to %s: %s", 
                          playerID.c_str(), message.c_str());
        #else
            CBasePlayer* pPlayer = FindPlayerBySteamID(playerID);
            if (!pPlayer) {
                MS_ANGEL_ERROR("SendConsoleMessage: Could not find player with ID: %s", playerID.c_str());
                return;
            }
            
            // Check if player is connected
            if (!ASEngineProvider::IsPlayerConnected((void*)pPlayer)) {
                MS_ANGEL_ERROR("SendConsoleMessage: Player %s is not connected", playerID.c_str());
                return;
            }
            
            // Use safe echo command - escape any remaining quotes
            std::string safeMessage = message;
            // Replace any remaining quotes with escaped versions
            size_t pos = 0;
            while ((pos = safeMessage.find('"', pos)) != std::string::npos) {
                safeMessage.replace(pos, 1, "'");
                pos += 1;
            }
            
            CLIENT_COMMAND(pPlayer->edict(), UTIL_VarArgs("echo \"%s\"\n", safeMessage.c_str()));
            
            MS_ANGEL_DEBUG("SendConsoleMessage: Sent console message to %s: %s", playerID.c_str(), safeMessage.c_str());
        #endif
    }
    
    // Send info popup to all players
    void AS_SendInfoMessageToAll(const std::string& title, const std::string& message)
    {
        if (title.empty() || message.empty()) {
            MS_ANGEL_ERROR("SendInfoMessageToAll: Empty title or message");
            return;
        }
        
        #ifdef CLIENT_DLL
            MS_ANGEL_DEBUG("SendInfoMessageToAll: Client attempted to send info message: %s - %s", 
                          title.c_str(), message.c_str());
        #else
            // Send info message to all connected players
            for (int i = 1; i <= gpGlobals->maxClients; i++) {
                CBasePlayer* pPlayer = GetPlayerByIndexHelper(i);
                if (pPlayer && pPlayer->edict() && !pPlayer->edict()->free && (pPlayer->edict()->v.flags & FL_CLIENT)) {
                    // Send using engine provider
                    std::string fullMessage = title + ": " + message;
                    ASEngineProvider::SendInfoMsg((void*)pPlayer, fullMessage);
                }
            }
            
            MS_ANGEL_DEBUG("SendInfoMessageToAll: Sent info message to all players: %s - %s", 
                          title.c_str(), message.c_str());
        #endif
    }
    
    // Send colored chat message to all players
    void AS_SendMessageToAllPlayers(const std::string& color, const std::string& message)
    {
        if (message.empty()) {
            MS_ANGEL_ERROR("SendMessageToAllPlayers: Empty message");
            return;
        }
        
        ASEngineProvider::SendMessageToAllPlayers(color, message);
        MS_ANGEL_DEBUG("SendMessageToAllPlayers: Sent colored message to all: [%s] %s", 
                          color.c_str(), message.c_str());
    }
    
    //==========================================================================
    // External System Integration Functions
    //==========================================================================
    
    // Call function on player's script
    void AS_CallPlayerExternal(const std::string& playerID, const std::string& function, CScriptArray* args)
    {
        if (playerID.empty() || function.empty()) {
            MS_ANGEL_ERROR("CallPlayerExternal: Empty playerID or function name");
            return;
        }
        
        #ifdef CLIENT_DLL
            MS_ANGEL_DEBUG("CallPlayerExternal: Client attempted to call %s on player %s", 
                          function.c_str(), playerID.c_str());
        #else
            CBasePlayer* pPlayer = FindPlayerBySteamID(playerID);
            if (!pPlayer) {
                MS_ANGEL_ERROR("CallPlayerExternal: Could not find player with ID: %s", playerID.c_str());
                return;
            }
            
            // Convert args array to vector of strings for easier handling
            std::vector<std::string> argList;
            if (args && args->GetSize() > 0) {
                for (asUINT i = 0; i < args->GetSize(); i++) {
                    std::string* argStr = (std::string*)args->At(i);
                    if (argStr) {
                        argList.push_back(*argStr);
                    } else {
                        argList.push_back("");
                    }
                }
            }
            
            MS_ANGEL_DEBUG("CallPlayerExternal: Calling function '%s' on player %s with %d arguments", 
                          function.c_str(), playerID.c_str(), (int)argList.size());
            
            // Integration with Master Sword script system through CLIENT_COMMAND
            // This allows calling player-side script functions via console commands
            if (!argList.empty()) {
                std::string commandStr = "as_player_call \"" + function + "\"";
                for (const auto& arg : argList) {
                    commandStr += " \"" + arg + "\"";
                }
                CLIENT_COMMAND(pPlayer->edict(), UTIL_VarArgs("%s\n", commandStr.c_str()));
            } else {
                CLIENT_COMMAND(pPlayer->edict(), UTIL_VarArgs("as_player_call \"%s\"\n", function.c_str()));
            }
            
            MS_ANGEL_INFO("CallPlayerExternal: Sent script call command to player %s", playerID.c_str());
        #endif
    }
    
    // Call GameMaster external function
    void AS_CallGameMasterExternal(const std::string& function, CScriptArray* args)
    {
        if (function.empty()) {
            MS_ANGEL_ERROR("CallGameMasterExternal: Empty function name");
            return;
        }
        
        #ifdef CLIENT_DLL
            MS_ANGEL_DEBUG("CallGameMasterExternal: Client attempted to call %s", function.c_str());
        #else
            // Convert args array to vector of strings
            std::vector<std::string> argList;
            if (args && args->GetSize() > 0) {
                for (asUINT i = 0; i < args->GetSize(); i++) {
                    std::string* argStr = (std::string*)args->At(i);
                    if (argStr) {
                        argList.push_back(*argStr);
                    } else {
                        argList.push_back("");
                    }
                }
            }
            
            MS_ANGEL_DEBUG("CallGameMasterExternal: Calling GameMaster function '%s' with %d arguments", 
                          function.c_str(), (int)argList.size());
            
            // Integration with Master Sword script system via server command
            // This allows calling server-side GameMaster script functions
            std::string commandStr = "as_gm_call \"" + function + "\"";
            for (const auto& arg : argList) {
                commandStr += " \"" + arg + "\"";
            }
            
            // Execute the command on the server
            SERVER_COMMAND(UTIL_VarArgs("%s\n", commandStr.c_str()));
            
            MS_ANGEL_INFO("CallGameMasterExternal: Executed GameMaster script call: %s", function.c_str());
        #endif
    }
    
    //==========================================================================
    // Event System Functions
    //==========================================================================
    
    // Global event handler storage (fallback for custom events)
    static std::map<std::string, std::vector<asIScriptFunction*>> g_CustomEventHandlers;
    
    // Convert string event name to EngineEventType enum
    EngineEventType StringToEngineEventType(const std::string& eventName)
    {
        if (eventName == "PLAYER_CONNECT") return EngineEventType::PLAYER_CONNECT;
        if (eventName == "PLAYER_DISCONNECT") return EngineEventType::PLAYER_DISCONNECT;
        if (eventName == "MONSTER_KILLED") return EngineEventType::MONSTER_KILLED;
        if (eventName == "TREASURE_SPAWNED") return EngineEventType::TREASURE_SPAWNED;
        if (eventName == "PLAYER_SPAWNED") return EngineEventType::PLAYER_SPAWNED;
        
        // Return a default that indicates custom event
        return EngineEventType::PLAYER_CONNECT; // Will be handled as custom event
    }
    
    // Check if event name is a built-in engine event
    bool IsBuiltinEngineEvent(const std::string& eventName)
    {
        return (eventName == "PLAYER_CONNECT" || 
                eventName == "PLAYER_DISCONNECT" ||
                eventName == "MONSTER_KILLED" ||
                eventName == "TREASURE_SPAWNED" ||
                eventName == "PLAYER_SPAWNED");
    }
    
    // Register event handler callback
    void AS_RegisterEngineEvent(const std::string& eventName, asIScriptFunction* callback)
    {
        if (eventName.empty() || !callback) {
            MS_ANGEL_ERROR("RegisterEngineEvent: Empty event name or null callback");
            return;
        }
        
        // Check if this is a built-in engine event that should use ASEngineEventManager
        if (IsBuiltinEngineEvent(eventName)) {
            #ifndef CLIENT_DLL
                ASEngineEventManager* eventMgr = ASEngineEventManager::Instance();
                if (eventMgr) {
                    EngineEventType eventType = StringToEngineEventType(eventName);
                    const char* moduleName = callback->GetModuleName();
                    bool success = eventMgr->RegisterEventHandler(eventType, callback, moduleName);
                    
                    if (success) {
                        MS_ANGEL_INFO("RegisterEngineEvent: Registered built-in event handler for '%s'", eventName.c_str());
                    } else {
                        MS_ANGEL_ERROR("RegisterEngineEvent: Failed to register built-in event handler for '%s'", eventName.c_str());
                    }
                    return;
                } else {
                    MS_ANGEL_ERROR("RegisterEngineEvent: ASEngineEventManager not available");
                }
            #else
                MS_ANGEL_DEBUG("RegisterEngineEvent: Client-side registration for built-in event '%s'", eventName.c_str());
            #endif
        }
        
        // Handle custom events or fallback for built-in events
        // Add reference to prevent garbage collection
        callback->AddRef();
        
        // Store the callback in our custom event handlers map
        g_CustomEventHandlers[eventName].push_back(callback);
        
        MS_ANGEL_DEBUG("RegisterEngineEvent: Registered custom event handler for '%s' (total handlers: %d)", 
                      eventName.c_str(), (int)g_CustomEventHandlers[eventName].size());
    }
    
    // Debug function to list all registered event handlers
    void AS_LogEngineEventHandlers()
    {
        MS_ANGEL_INFO("=== Registered Engine Event Handlers ===");
        
        // Log built-in engine event handlers via ASEngineEventManager
        #ifndef CLIENT_DLL
            ASEngineEventManager* eventMgr = ASEngineEventManager::Instance();
            if (eventMgr) {
                MS_ANGEL_INFO("Built-in Engine Events:");
                eventMgr->LogEventHandlers();
            }
        #endif
        
        // Log custom event handlers
        if (g_CustomEventHandlers.empty()) {
            MS_ANGEL_INFO("No custom event handlers registered");
        } else {
            MS_ANGEL_INFO("Custom Events:");
            for (const auto& pair : g_CustomEventHandlers) {
                MS_ANGEL_INFO("Event '%s': %d handlers", pair.first.c_str(), (int)pair.second.size());
                for (size_t i = 0; i < pair.second.size(); i++) {
                    asIScriptFunction* func = pair.second[i];
                    if (func) {
                        const char* funcName = func->GetName();
                        const char* moduleName = func->GetModuleName();
                        MS_ANGEL_INFO("  Handler %d: %s::%s", (int)i, moduleName ? moduleName : "Unknown", funcName ? funcName : "Unknown");
                    }
                }
            }
        }
        MS_ANGEL_INFO("=========================================");
    }
    
    // Function to trigger a custom event (for internal use)
    void TriggerCustomEngineEvent(const std::string& eventName, CScriptArray* args)
    {
        auto it = g_CustomEventHandlers.find(eventName);
        if (it == g_CustomEventHandlers.end() || it->second.empty()) {
            return; // No handlers for this event
        }
        
        MS_ANGEL_DEBUG("TriggerCustomEngineEvent: Triggering custom event '%s' with %d handlers", 
                      eventName.c_str(), (int)it->second.size());
        
        // Call all registered handlers
        for (asIScriptFunction* handler : it->second) {
            if (handler) {
                // Get the current context or create a new one
                asIScriptContext* ctx = asGetActiveContext();
                bool needsCleanup = false;
                
                if (!ctx) {
                    // Create new context if none active
                    asIScriptEngine* engine = handler->GetEngine();
                    if (engine) {
                        ctx = engine->CreateContext();
                        needsCleanup = true;
                    }
                }
                
                if (ctx) {
                    // Prepare and execute the handler
                    int result = ctx->Prepare(handler);
                    if (result < 0) {
                        MS_ANGEL_ERROR("TriggerCustomEngineEvent: Failed to prepare context for event '%s'", eventName.c_str());
                        if (needsCleanup) {
                            ctx->Release();
                        }
                        continue;
                    }
                    
                    // Pass arguments if provided and function expects them
                    if (args && args->GetSize() > 0 && handler->GetParamCount() > 0) {
                        ctx->SetArgObject(0, args);
                    }
                    
                    result = ctx->Execute();
                    if (result != asEXECUTION_FINISHED) {
                        if (result == asEXECUTION_EXCEPTION) {
                            MS_ANGEL_ERROR("TriggerCustomEngineEvent: Exception in handler for event '%s': %s", 
                                         eventName.c_str(), ctx->GetExceptionString());
                        } else {
                            MS_ANGEL_ERROR("TriggerCustomEngineEvent: Handler execution failed for event '%s' (result: %d)", 
                                         eventName.c_str(), result);
                        }
                    }
                    
                    if (needsCleanup) {
                        ctx->Release();
                    }
                }
            }
        }
    }
    
    // Public function to trigger any event (built-in or custom)
    void TriggerEngineEvent(const std::string& eventName, CScriptArray* args)
    {
        if (IsBuiltinEngineEvent(eventName)) {
            // Built-in events are handled by ASEngineEventManager when fired from C++ code
            // This function is mainly for custom events or testing
            MS_ANGEL_DEBUG("TriggerEngineEvent: Built-in event '%s' should be triggered via C++ engine code", eventName.c_str());
        }
        
        // Always check for custom handlers
        TriggerCustomEngineEvent(eventName, args);
    }
    
    //==========================================================================
    // Advanced System Stubs
    //==========================================================================
    
    // Advanced system state tracking
    struct AdvancedSystemState {
        bool triggerSystemActive;
        bool hpSequenceSystemActive;
        bool entitySpawnerActive;
        bool entityCommSystemActive;
        std::vector<std::string> activeTriggers;
        std::vector<std::string> activeSequences;
        
        AdvancedSystemState() : triggerSystemActive(false), hpSequenceSystemActive(false),
                               entitySpawnerActive(false), entityCommSystemActive(false) {}
    };
    
    static AdvancedSystemState g_AdvancedSystems;
    
    void AS_InitializeAdvancedTriggerSystem()
    {
        if (g_AdvancedSystems.triggerSystemActive) {
            MS_ANGEL_INFO("InitializeAdvancedTriggerSystem: System already active");
            return;
        }
        
        #ifndef CLIENT_DLL
            // Server-side initialization
            g_AdvancedSystems.triggerSystemActive = true;
            g_AdvancedSystems.activeTriggers.clear();
            
            // Register trigger-related console commands
            SERVER_COMMAND("as_trigger_debug 1\n");
            
            // Initialize trigger monitoring
            TriggerCustomEngineEvent("TRIGGER_SYSTEM_INIT", nullptr);
            
            MS_ANGEL_INFO("InitializeAdvancedTriggerSystem: Advanced trigger system initialized successfully");
        #else
            MS_ANGEL_DEBUG("InitializeAdvancedTriggerSystem: Client-side initialization");
        #endif
    }
    
    void AS_InitializeHPSequenceTrigger()
    {
        if (g_AdvancedSystems.hpSequenceSystemActive) {
            MS_ANGEL_INFO("InitializeHPSequenceTrigger: HP sequence system already active");
            return;
        }
        
        #ifndef CLIENT_DLL
            // Server-side initialization
            g_AdvancedSystems.hpSequenceSystemActive = true;
            g_AdvancedSystems.activeSequences.clear();
            
            // Initialize HP monitoring for all players
            for (int i = 1; i <= gpGlobals->maxClients; i++) {
                CBasePlayer* pPlayer = GetPlayerByIndexHelper(i);
                if (pPlayer && pPlayer->edict() && !pPlayer->edict()->free && (pPlayer->edict()->v.flags & FL_CLIENT)) {
                    // Set up HP monitoring for this player
                    // This would integrate with Master Sword's health system
                    std::string playerID = ASEngineProvider::GetPlayerAuthId((void*)pPlayer);
                    if (!playerID.empty()) {
                        g_AdvancedSystems.activeSequences.push_back("HP_MONITOR_" + playerID);
                    }
                }
            }
            
            // Trigger system initialization event
            TriggerCustomEngineEvent("HP_SEQUENCE_SYSTEM_INIT", nullptr);
            
            MS_ANGEL_INFO("InitializeHPSequenceTrigger: HP sequence trigger system initialized with %d player monitors", 
                         (int)g_AdvancedSystems.activeSequences.size());
        #else
            MS_ANGEL_DEBUG("InitializeHPSequenceTrigger: Client-side initialization");
        #endif
    }
    
    void AS_InitializeEntitySpawner()
    {
        if (g_AdvancedSystems.entitySpawnerActive) {
            MS_ANGEL_INFO("InitializeEntitySpawner: Entity spawner already active");
            return;
        }
        
        #ifndef CLIENT_DLL
            // Server-side initialization
            g_AdvancedSystems.entitySpawnerActive = true;
            
            // Initialize entity spawning system
            // This would integrate with Master Sword's entity creation system
            SERVER_COMMAND("as_spawner_debug 1\n");
            
            // Set up spawner monitoring
            TriggerCustomEngineEvent("ENTITY_SPAWNER_INIT", nullptr);
            
            MS_ANGEL_INFO("InitializeEntitySpawner: Entity spawner system initialized successfully");
        #else
            MS_ANGEL_DEBUG("InitializeEntitySpawner: Client-side initialization");
        #endif
    }
    
    void AS_InitializeEntityCommunications()
    {
        if (g_AdvancedSystems.entityCommSystemActive) {
            MS_ANGEL_INFO("InitializeEntityCommunications: Entity communications already active");
            return;
        }
        
        #ifndef CLIENT_DLL
            // Server-side initialization
            g_AdvancedSystems.entityCommSystemActive = true;
            
            // Initialize entity-to-entity communication system
            // This would allow entities to send messages to each other
            SERVER_COMMAND("as_entity_comm 1\n");
            
            // Set up communication channels
            TriggerCustomEngineEvent("ENTITY_COMM_SYSTEM_INIT", nullptr);
            
            MS_ANGEL_INFO("InitializeEntityCommunications: Entity communications system initialized successfully");
        #else
            MS_ANGEL_DEBUG("InitializeEntityCommunications: Client-side initialization");
        #endif
    }
    
    void AS_ShutdownAdvancedTriggerSystem()
    {
        if (!g_AdvancedSystems.triggerSystemActive) {
            MS_ANGEL_INFO("ShutdownAdvancedTriggerSystem: System not active");
            return;
        }
        
        #ifndef CLIENT_DLL
            // Server-side shutdown
            TriggerCustomEngineEvent("TRIGGER_SYSTEM_SHUTDOWN", nullptr);
            
            // Disable trigger monitoring
            SERVER_COMMAND("as_trigger_debug 0\n");
            
            // Clear active triggers
            g_AdvancedSystems.activeTriggers.clear();
            g_AdvancedSystems.triggerSystemActive = false;
            
            MS_ANGEL_INFO("ShutdownAdvancedTriggerSystem: Advanced trigger system shutdown successfully");
        #else
            MS_ANGEL_DEBUG("ShutdownAdvancedTriggerSystem: Client-side shutdown");
        #endif
    }
    
    void AS_ShutdownHPSequenceTrigger()
    {
        if (!g_AdvancedSystems.hpSequenceSystemActive) {
            MS_ANGEL_INFO("ShutdownHPSequenceTrigger: HP sequence system not active");
            return;
        }
        
        #ifndef CLIENT_DLL
            // Server-side shutdown
            TriggerCustomEngineEvent("HP_SEQUENCE_SYSTEM_SHUTDOWN", nullptr);
            
            // Clear HP monitoring sequences
            g_AdvancedSystems.activeSequences.clear();
            g_AdvancedSystems.hpSequenceSystemActive = false;
            
            MS_ANGEL_INFO("ShutdownHPSequenceTrigger: HP sequence trigger system shutdown successfully");
        #else
            MS_ANGEL_DEBUG("ShutdownHPSequenceTrigger: Client-side shutdown");
        #endif
    }
    
    void AS_ShutdownEntitySpawner()
    {
        if (!g_AdvancedSystems.entitySpawnerActive) {
            MS_ANGEL_INFO("ShutdownEntitySpawner: Entity spawner not active");
            return;
        }
        
        #ifndef CLIENT_DLL
            // Server-side shutdown
            TriggerCustomEngineEvent("ENTITY_SPAWNER_SHUTDOWN", nullptr);
            
            // Disable spawner monitoring
            SERVER_COMMAND("as_spawner_debug 0\n");
            
            g_AdvancedSystems.entitySpawnerActive = false;
            
            MS_ANGEL_INFO("ShutdownEntitySpawner: Entity spawner system shutdown successfully");
        #else
            MS_ANGEL_DEBUG("ShutdownEntitySpawner: Client-side shutdown");
        #endif
    }
    
    void AS_ShutdownEntityCommunications()
    {
        if (!g_AdvancedSystems.entityCommSystemActive) {
            MS_ANGEL_INFO("ShutdownEntityCommunications: Entity communications not active");
            return;
        }
        
        #ifndef CLIENT_DLL
            // Server-side shutdown
            TriggerCustomEngineEvent("ENTITY_COMM_SYSTEM_SHUTDOWN", nullptr);
            
            // Disable communication system
            SERVER_COMMAND("as_entity_comm 0\n");
            
            g_AdvancedSystems.entityCommSystemActive = false;
            
            MS_ANGEL_INFO("ShutdownEntityCommunications: Entity communications system shutdown successfully");
        #else
            MS_ANGEL_DEBUG("ShutdownEntityCommunications: Client-side shutdown");
        #endif
    }
    
    // Additional helper functions for advanced systems
    bool AS_IsAdvancedTriggerSystemActive()
    {
        return g_AdvancedSystems.triggerSystemActive;
    }
    
    bool AS_IsHPSequenceSystemActive()
    {
        return g_AdvancedSystems.hpSequenceSystemActive;
    }
    
    bool AS_IsEntitySpawnerActive()
    {
        return g_AdvancedSystems.entitySpawnerActive;
    }
    
    bool AS_IsEntityCommSystemActive()
    {
        return g_AdvancedSystems.entityCommSystemActive;
    }
    
    int AS_GetActiveTriggersCount()
    {
        return (int)g_AdvancedSystems.activeTriggers.size();
    }
    
    int AS_GetActiveSequencesCount()
    {
        return (int)g_AdvancedSystems.activeSequences.size();
    }
    
    //==========================================================================
    // GameMaster Functions Registration
    //==========================================================================
    
    void RegisterGameMasterFunctions(asIScriptEngine* pEngine)
    {
        if (!pEngine) {
            MS_ANGEL_ERROR("RegisterGameMasterFunctions: NULL engine pointer");
            return;
        }
        
        MS_ANGEL_INFO("RegisterGameMasterFunctions: Registering GameMaster communication and event functions...");
        
        // Register communication functions
        pEngine->RegisterGlobalFunction("void SendPlayerMessage(const string &in, const string &in)", 
            asFUNCTION(AS_SendPlayerMessage), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("void SendConsoleMessage(const string &in, const string &in)", 
            asFUNCTION(AS_SendConsoleMessage), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("void SendInfoMessageToAll(const string &in, const string &in)", 
            asFUNCTION(AS_SendInfoMessageToAll), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("void SendMessageToAllPlayers(const string &in, const string &in)", 
            asFUNCTION(AS_SendMessageToAllPlayers), asCALL_CDECL);
        
        // Register external system integration functions
        pEngine->RegisterGlobalFunction("void CallPlayerExternal(const string &in, const string &in, const array<string>@ &in)", 
            asFUNCTION(AS_CallPlayerExternal), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("void CallGameMasterExternal(const string &in, const array<string>@ &in)", 
            asFUNCTION(AS_CallGameMasterExternal), asCALL_CDECL);
        
        // Register event system functions
        // TODO: Fix function pointer issues
        // pEngine->RegisterGlobalFunction("void RegisterEngineEvent(const string &in, funcdef@ callback)", 
        //     asFUNCTION(AS_RegisterEngineEvent), asCALL_CDECL);
        // Note: LogEngineEventHandlers is registered by ASEngineEventManager.cpp
        
        // Register advanced system functions
        pEngine->RegisterGlobalFunction("void InitializeAdvancedTriggerSystem()", 
            asFUNCTION(AS_InitializeAdvancedTriggerSystem), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("void InitializeHPSequenceTrigger()", 
            asFUNCTION(AS_InitializeHPSequenceTrigger), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("void InitializeEntitySpawner()", 
            asFUNCTION(AS_InitializeEntitySpawner), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("void InitializeEntityCommunications()", 
            asFUNCTION(AS_InitializeEntityCommunications), asCALL_CDECL);
        
        pEngine->RegisterGlobalFunction("void ShutdownAdvancedTriggerSystem()", 
            asFUNCTION(AS_ShutdownAdvancedTriggerSystem), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("void ShutdownHPSequenceTrigger()", 
            asFUNCTION(AS_ShutdownHPSequenceTrigger), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("void ShutdownEntitySpawner()", 
            asFUNCTION(AS_ShutdownEntitySpawner), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("void ShutdownEntityCommunications()", 
            asFUNCTION(AS_ShutdownEntityCommunications), asCALL_CDECL);
        
        // Register advanced system status functions
        pEngine->RegisterGlobalFunction("bool IsAdvancedTriggerSystemActive()", 
            asFUNCTION(AS_IsAdvancedTriggerSystemActive), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("bool IsHPSequenceSystemActive()", 
            asFUNCTION(AS_IsHPSequenceSystemActive), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("bool IsEntitySpawnerActive()", 
            asFUNCTION(AS_IsEntitySpawnerActive), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("bool IsEntityCommSystemActive()", 
            asFUNCTION(AS_IsEntityCommSystemActive), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("int GetActiveTriggersCount()", 
            asFUNCTION(AS_GetActiveTriggersCount), asCALL_CDECL);
        pEngine->RegisterGlobalFunction("int GetActiveSequencesCount()", 
            asFUNCTION(AS_GetActiveSequencesCount), asCALL_CDECL);
        
        MS_ANGEL_INFO("RegisterGameMasterFunctions: GameMaster functions registered successfully");
    }
}