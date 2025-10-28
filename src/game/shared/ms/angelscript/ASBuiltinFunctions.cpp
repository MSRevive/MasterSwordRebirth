//==========================================================================
// ASBuiltinFunctions.cpp - Using asbind20
//
// Built-in function implementations for AngelScript integration
// Converts legacy $-functions to AngelScript global functions
//==========================================================================

#include "ASBuiltinFunctions.h"
#include "ASEngineInterface.h"
#include "ASScriptContext.h"
#include "ASBindExtensions.h"
#include "CAngelScriptManager.h"
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
    #include "svglobals.h"
    extern globalvars_t *gpGlobals;
    extern enginefuncs_t g_engfuncs;
    // Note: gmsgSayText is handled via ASEngineProvider pattern
#endif
#include <cmath>
#include <map>
#include "hl/vector.h"
#include "mslogger.h"
#include "sharedutil.h"
#include "ASEngineEventManager.h"


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
// Helper function to register function contexts after asbind20 registration
//==========================================================================
static void RegisterFunctionContext(asIScriptEngine* pEngine, const char* funcDecl, ScriptContext context)
{
    ASScriptContextManager* pContextMgr = ASScriptContextManager::Instance();
    if (!pContextMgr) return;
    
    // Find the function
    asIScriptFunction* func = pEngine->GetGlobalFunctionByDecl(funcDecl);
    if (func)
    {
        pContextMgr->RegisterFunctionContext(func->GetName(), context);
    }
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
        
        // Register math functions with context-aware asbind20 extension
        // Note: Basic float functions (sin, cos, sqrt, abs, ceil, min, max) are registered in ASCoreTypes.cpp
        // We add integer versions and additional utility functions
        using namespace asbind20::ctx;
        asbind20::global_ctx(pEngine)
            // Integer math functions to complement float versions from ASCoreTypes - all SHARED
            .function("int abs(int)", &AS_AbsInt, shared)
            .function("int min(int, int)", &AS_MinInt, shared)
            .function("int max(int, int)", &AS_MaxInt, shared);
        
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
        
        // Register vector creation and manipulation functions with context-aware asbind20
        using namespace asbind20::ctx;
        asbind20::global_ctx(pEngine)
            .function("Vector3 CreateVector(float, float, float)", &AS_CreateVector, shared)
            .function("float GetVectorX(const Vector3 &in)", &AS_GetVectorX, shared)
            .function("float GetVectorY(const Vector3 &in)", &AS_GetVectorY, shared)
            .function("float GetVectorZ(const Vector3 &in)", &AS_GetVectorZ, shared)
            .function("float Distance(const Vector3 &in, const Vector3 &in)", &AS_Distance, shared)
            .function("float DotProduct(const Vector3 &in, const Vector3 &in)", &AS_DotProduct, shared);
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
    CBaseEntity* AS_FindEntityByName(const std::string& name)
    {
        MS_ANGEL_DEBUG("FindEntityByName: Searching for '%s'", name.c_str());
        // TODO: Implement proper entity finding
        // For now, return nullptr
        return nullptr;
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
    bool AS_IsValidEntity(CBaseEntity* entity)
    {
        MS_ANGEL_DEBUG("IsValidEntity: Checking entity %p", entity);
        // Check if entity pointer is valid and not freed
        if (!entity)
            return false;
#ifdef VALVE_DLL
        return !FNullEnt(entity);
#else
        return true;  // Client-side: assume valid if not null
#endif
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
        
        // Register game functions with context-aware asbind20
        // Note: GetTimestamp, GetPlayerCount, GetAllPlayers are registered in ASEntityBindings.cpp
        // Note: formatInt is provided by scriptstdstring addon with enhanced functionality
        using namespace asbind20::ctx;
        asbind20::global_ctx(pEngine)
            // String utility functions - SHARED
            .function("string formatFloat(float, const string &in, int, int)", &AS_FormatFloat, shared)
            // Random functions - SHARED
            .function("float Random(float, float)", &AS_Random, shared)
            .function("int RandomInt(int, int)", &AS_RandomInt, shared)
            // Logging functions - SHARED
            .function("void LogMessage(const string &in)", &AS_LogMessage, shared)
            .function("void DeveloperMessage(int, const string &in)", &AS_DeveloperMessage, shared)
            // Angle manipulation functions - SHARED
            .function("Vector3 CreateAngles(float, float, float)", &AS_CreateAngles, shared)
            .function("float GetAnglePitch(const Vector3 &in)", &AS_GetAnglePitch, shared)
            .function("float GetAngleYaw(const Vector3 &in)", &AS_GetAngleYaw, shared)
            .function("float GetAngleRoll(const Vector3 &in)", &AS_GetAngleRoll, shared);
        
        // Note: GetAllPlayers is registered in ASEntityBindings.cpp to avoid duplication
        
        // Register split function with old API + context tracking
        pEngine->RegisterGlobalFunction("array<string>@ split(const string &in, const string &in)", 
            asFUNCTION(AS_SplitWrapper), asCALL_CDECL);
        RegisterFunctionContext(pEngine, "array<string>@ split(const string &in, const string &in)", ScriptContext::SHARED);
        
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
                    //ASEngineProvider::SendInfoMsg((void*)pPlayer, fullMessage);
                    pPlayer->SendHelpMsg("game_transition", title.c_str(), message.c_str());
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
    // MovePlayerToRandomSpawn - Move player to a random spawn point within range
    // Converted from torandomspawn script command (scriptcmds.cpp:7034)
    // SERVER ONLY
    //==========================================================================

    bool AS_MovePlayerToRandomSpawn(CBasePlayer* pPlayer, float maxDistance)
    {
#ifdef VALVE_DLL
        if (!pPlayer) {
            MS_ANGEL_ERROR("MovePlayerToRandomSpawn: NULL player pointer");
            return false;
        }
        
        // Find all spawn points within maxDistance
        mslist<CBaseEntity*> spawnpoints;
        CBaseEntity* pSpot = NULL;
        
        // Find all ms_player_spawn entities (SPAWN_GENERIC constant from svglobals.h)
        while ((pSpot = UTIL_FindEntityByClassname(pSpot, SPAWN_GENERIC)) != NULL)
        {
            float distance = (pSpot->pev->origin - pPlayer->pev->origin).Length();
            if (distance <= maxDistance)
            {
                spawnpoints.add(pSpot);
            }
        }
        
        if (spawnpoints.size() > 0)
        {
            // Pick a random spawn point
            int loc = RANDOM_LONG(0, spawnpoints.size() - 1);
            pPlayer->pev->origin = spawnpoints[loc]->pev->origin;
            
            MS_ANGEL_INFO("MovePlayerToRandomSpawn: Moved player %s to random spawn (found %d nearby spawns)", 
                           pPlayer->AuthID().c_str(), (int)spawnpoints.size());
            return true;
        }
        else
        {
            MS_ANGEL_DEBUG("MovePlayerToRandomSpawn: No spawn points found within %.1f units for player %s", 
                           maxDistance, pPlayer->AuthID().c_str());
            return false;
        }
#else
        return false; // Not supported on client
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
}

//==========================================================================
// Bridge function for legacy script system to call AngelScript handlers
//==========================================================================
void CallAngelScriptEventHandler(const char* eventName, msstringlist* params)
{
    using namespace ASBuiltinFunctions;
    
    if (!eventName || !eventName[0]) {
        return;
    }
    
    // CRITICAL: Check if AngelScript engine is still initialized
    // During map transitions or engine shutdown, the AngelScript manager may be destroyed
    // Attempting to call event handlers in this state causes crashes at as_context.cpp:4978
    CAngelScriptManager* pManager = CAngelScriptManager::Instance();
    if (!pManager || !pManager->IsInitialized()) {
        // Engine is not initialized or is shutting down - silently return
        return;
    }
    
    // Cache to track events we've already searched for and found nothing
    static std::map<std::string, bool> s_eventCache;
    
    // Check cache first to avoid repeated searches for non-existent events
    auto cacheIt = s_eventCache.find(eventName);
    if (cacheIt != s_eventCache.end() && !cacheIt->second) {
        // We've already searched for this event and found nothing
        return;
    }
    
    // First try registered event handlers
    auto it = g_CustomEventHandlers.find(eventName);
    if (it != g_CustomEventHandlers.end() && !it->second.empty()) {
        // Cache that this event has handlers
        s_eventCache[eventName] = true;
        
        MS_ANGEL_DEBUG("CallAngelScriptEventHandler: Found %d registered handlers for event '%s'", 
                      (int)it->second.size(), eventName);
    
    // Call all registered handlers
    for (asIScriptFunction* handler : it->second) {
        if (!handler) continue;
        
        // CRITICAL: Always create a new context for event handlers
        // Never reuse asGetActiveContext() as it may already be executing
        // and will fail with asCONTEXT_ACTIVE when calling Prepare()
        asIScriptEngine* engine = handler->GetEngine();
        if (!engine) {
            MS_ANGEL_ERROR("CallAngelScriptEventHandler: Handler has no engine for event '%s'", eventName);
            continue;
        }
        
        asIScriptContext* ctx = pManager->AcquireContext();
        if (!ctx) {
            MS_ANGEL_ERROR("CallAngelScriptEventHandler: Failed to acquire context for event '%s'", eventName);
            continue;
        }
        
        // Prepare the context
        int result = ctx->Prepare(handler);
        if (result < 0) {
            MS_ANGEL_ERROR("CallAngelScriptEventHandler: Failed to prepare context for event '%s' (code: %d)", eventName, result);
            pManager->ReleaseContext(ctx);
            continue;
        }
        
        // Set parameters - convert msstringlist to std::string vector
        std::vector<std::string> stringParams;
        if (params && params->size() > 0) {
            for (size_t i = 0; i < params->size(); i++) {
                msstring& msStr = (*params)[i];
                const char* cstr = msStr.c_str();
                MS_ANGEL_DEBUG("CallAngelScriptEventHandler: Handler Param[%d] = '%s' (len=%d)", 
                              (int)i, cstr ? cstr : "(null)", cstr ? (int)strlen(cstr) : 0);
                stringParams.push_back(cstr ? cstr : "");
            }
        }
        
        int paramCount = handler->GetParamCount();
        for (int i = 0; i < paramCount && i < (int)stringParams.size(); i++) {
            // Check if parameter is by reference
            int typeId;
            asDWORD flags;
            handler->GetParam(i, &typeId, &flags);
            
            // Pass the string appropriately
            if (flags & asTM_INREF || flags & asTM_INOUTREF) {
                ctx->SetArgAddress(i, &stringParams[i]);
            } else {
                ctx->SetArgObject(i, &stringParams[i]);
            }
        }
        
        // Execute the handler
        result = ctx->Execute();
        if (result != asEXECUTION_FINISHED) {
            if (result == asEXECUTION_EXCEPTION) {
                MS_ANGEL_ERROR("CallAngelScriptEventHandler: Exception in handler for event '%s': %s", 
                             eventName, ctx->GetExceptionString());
            } else {
                MS_ANGEL_ERROR("CallAngelScriptEventHandler: Handler execution failed for event '%s' (result: %d)", 
                             eventName, result);
            }
        } else {
            //MS_ANGEL_DEBUG("CallAngelScriptEventHandler: Handler executed successfully for event '%s'", eventName);
        }
        
        // Return context to pool
        pManager->ReleaseContext(ctx);
    }
        return; // Found and called registered handlers
    }
    
    // If no registered handlers, try to find global function by name in all modules
    // Only log if we haven't checked this event before
    if (cacheIt == s_eventCache.end()) {
        MS_ANGEL_DEBUG("CallAngelScriptEventHandler: No registered handlers for '%s', searching for global function...", 
                      eventName);
    }
    
    // Note: pManager was already checked at the beginning of this function,
    // but we check again here for defensive programming
    asIScriptEngine* pEngine = pManager->GetEngine();
    if (!pEngine) {
        return;
    }
    
    // Search all modules for a global function with this name
    for (asUINT i = 0; i < pEngine->GetModuleCount(); i++) {
        asIScriptModule* pModule = pEngine->GetModuleByIndex(i);
        if (!pModule) continue;
        
        asIScriptFunction* pFunction = pModule->GetFunctionByName(eventName);
        if (!pFunction) continue;
        
        // Cache that we found this function
        s_eventCache[eventName] = true;
        
        //MS_ANGEL_INFO("CallAngelScriptEventHandler: Found global function '%s' in module '%s'", eventName, pModule->GetName());
        
        // CRITICAL: Always create a new context for event handlers
        // Never reuse asGetActiveContext() as it may already be executing
        asIScriptContext* ctx = pManager->AcquireContext();
        if (!ctx) {
            MS_ANGEL_ERROR("CallAngelScriptEventHandler: Failed to acquire context for function '%s'", eventName);
            continue;
        }
        
        // Prepare the context
        int result = ctx->Prepare(pFunction);
        if (result < 0) {
            MS_ANGEL_ERROR("CallAngelScriptEventHandler: Failed to prepare context for function '%s' (code: %d)", eventName, result);
            pManager->ReleaseContext(ctx);
            continue;
        }
        
        // Set parameters - convert msstringlist to std::string vector
        std::vector<std::string> stringParams;
        if (params && params->size() > 0) {
            for (size_t j = 0; j < params->size(); j++) {
                msstring& msStr = (*params)[j];
                const char* cstr = msStr.c_str();
                MS_ANGEL_DEBUG("CallAngelScriptEventHandler: Param[%d] = '%s' (len=%d)", 
                              (int)j, cstr ? cstr : "(null)", cstr ? (int)strlen(cstr) : 0);
                stringParams.push_back(cstr ? cstr : "");
            }
        }
        
        int paramCount = pFunction->GetParamCount();
        for (int j = 0; j < paramCount && j < (int)stringParams.size(); j++) {
            int typeId;
            asDWORD flags;
            pFunction->GetParam(j, &typeId, &flags);
            
            if (flags & asTM_INREF || flags & asTM_INOUTREF) {
                ctx->SetArgAddress(j, &stringParams[j]);
            } else {
                ctx->SetArgObject(j, &stringParams[j]);
            }
        }
        
        // Execute
        result = ctx->Execute();
        if (result != asEXECUTION_FINISHED) {
            if (result == asEXECUTION_EXCEPTION) {
                MS_ANGEL_ERROR("CallAngelScriptEventHandler: Exception in function '%s': %s", 
                             eventName, ctx->GetExceptionString());
            } else {
                MS_ANGEL_ERROR("CallAngelScriptEventHandler: Function execution failed for '%s' (result: %d)", 
                             eventName, result);
            }
        }
        
        // Return context to pool
        pManager->ReleaseContext(ctx);
        
        // Only call the first matching function found
        return;
    }
    
    // Cache that we didn't find this event/function (false = not found)
    s_eventCache[eventName] = false;
    
    // Only log the first time we search and don't find it
    if (cacheIt == s_eventCache.end()) {
        MS_ANGEL_DEBUG("CallAngelScriptEventHandler: No global function named '%s' found in any module", eventName);
    }
}

namespace ASBuiltinFunctions
{
    
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
        
        #ifdef VALVE_DLL
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
        
        #ifdef VALVE_DLL
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
        
        #ifdef VALVE_DLL
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
        
        #ifdef VALVE_DLL
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
        
        #ifdef VALVE_DLL
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
        
        #ifdef VALVE_DLL
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
        
        #ifdef VALVE_DLL
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
        
        #ifdef VALVE_DLL
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
    // Server Command Execution
    //==========================================================================
    
    /**
     * Execute a server command from AngelScript
     * This is a critical function for game management
     */
    void AS_ExecuteServerCommand(const std::string& command)
    {
        #ifdef CLIENT_DLL
            MS_ANGEL_ERROR("ExecuteServerCommand: Cannot execute server commands from client");
            return;
        #else
            if (command.empty()) {
                MS_ANGEL_DEBUG("ExecuteServerCommand: Empty command");
                return;
            }
            
            // Security: Basic validation to prevent dangerous commands
            // Block commands that could crash or compromise the server
            if (command.find("quit") != std::string::npos || 
                command.find("exit") != std::string::npos ||
                command.find("rcon_password") != std::string::npos) {
                MS_ANGEL_ERROR("ExecuteServerCommand: Blocked dangerous command '%s'", command.c_str());
                return;
            }
            
            MS_ANGEL_INFO("ExecuteServerCommand: Executing '%s'", command.c_str());
            
            // Execute the command using the Half-Life engine
            SERVER_COMMAND(const_cast<char*>(command.c_str()));
            SERVER_EXECUTE();
        #endif
    }
    
    //==========================================================================
    // Map Validation
    //==========================================================================
    
    /**
     * Check if a map exists on the server
     * Uses the engine's file system to validate map existence
     */
    bool AS_EngineMapExists(const std::string& mapName)
    {
        #ifdef CLIENT_DLL
            MS_ANGEL_DEBUG("EngineMapExists: Client cannot check map existence");
            return false;
        #else
            if (mapName.empty()) {
                MS_ANGEL_DEBUG("EngineMapExists: Empty map name");
                return false;
            }
            
            // Use ASEngineProvider to check map existence
            bool exists = ASEngineProvider::EngineMapExists(mapName);
            MS_ANGEL_DEBUG("EngineMapExists: Map '%s' %s", mapName.c_str(), exists ? "exists" : "does not exist");
            return exists;
        #endif
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
        
        // All GameMaster functions are SERVER_ONLY
        using namespace asbind20::ctx;
        ASScriptContextManager* pContextMgr = ASScriptContextManager::Instance();
        
        // Helper lambda to register with context
        auto regFunc = [&](const char* decl, auto func) {
            pEngine->RegisterGlobalFunction(decl, asFUNCTION(func), asCALL_CDECL);
            if (pContextMgr) {
                asIScriptFunction* asFunc = pEngine->GetGlobalFunctionByDecl(decl);
                if (asFunc) pContextMgr->RegisterFunctionContext(asFunc->GetName(), ScriptContext::SERVER_ONLY);
            }
        };
        
        // Register server command execution
        regFunc("void ExecuteServerCommand(const string &in)", AS_ExecuteServerCommand);
        regFunc("bool EngineMapExists(const string &in)", AS_EngineMapExists);
        
        // Register communication functions
        regFunc("void SendPlayerMessage(const string &in, const string &in)", AS_SendPlayerMessage);
        regFunc("void SendConsoleMessage(const string &in, const string &in)", AS_SendConsoleMessage);
        regFunc("void SendInfoMessageToAll(const string &in, const string &in)", AS_SendInfoMessageToAll);
        regFunc("void SendMessageToAllPlayers(const string &in, const string &in)", AS_SendMessageToAllPlayers);
        
        // Register external system integration functions
        regFunc("void CallPlayerExternal(const string &in, const string &in, const array<string>@ &in)", AS_CallPlayerExternal);
        regFunc("void CallGameMasterExternal(const string &in, const array<string>@ &in)", AS_CallGameMasterExternal);
        regFunc("bool MovePlayerToRandomSpawn(CBasePlayer@, float)", AS_MovePlayerToRandomSpawn);
        
        // Register advanced system functions
        regFunc("void InitializeAdvancedTriggerSystem()", AS_InitializeAdvancedTriggerSystem);
        regFunc("void InitializeHPSequenceTrigger()", AS_InitializeHPSequenceTrigger);
        regFunc("void InitializeEntitySpawner()", AS_InitializeEntitySpawner);
        regFunc("void InitializeEntityCommunications()", AS_InitializeEntityCommunications);
        
        regFunc("void ShutdownAdvancedTriggerSystem()", AS_ShutdownAdvancedTriggerSystem);
        regFunc("void ShutdownHPSequenceTrigger()", AS_ShutdownHPSequenceTrigger);
        regFunc("void ShutdownEntitySpawner()", AS_ShutdownEntitySpawner);
        regFunc("void ShutdownEntityCommunications()", AS_ShutdownEntityCommunications);
        
        // Register advanced system status functions
        regFunc("bool IsAdvancedTriggerSystemActive()", AS_IsAdvancedTriggerSystemActive);
        regFunc("bool IsHPSequenceSystemActive()", AS_IsHPSequenceSystemActive);
        regFunc("bool IsEntitySpawnerActive()", AS_IsEntitySpawnerActive);
        regFunc("bool IsEntityCommSystemActive()", AS_IsEntityCommSystemActive);
        regFunc("int GetActiveTriggersCount()", AS_GetActiveTriggersCount);
        regFunc("int GetActiveSequencesCount()", AS_GetActiveSequencesCount);
        
        MS_ANGEL_INFO("RegisterGameMasterFunctions: GameMaster functions registered successfully");
    }
}