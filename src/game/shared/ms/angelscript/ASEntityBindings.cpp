//==========================================================================
// ASEntityBindings.cpp
// 
// Entity-related type bindings for AngelScript integration
// Provides CBaseEntity, CBasePlayer, and related functions for script usage
//==========================================================================

#include "CAngelScript.h"
#include <angelscript.h>
#include "addons/scriptarray/scriptarray.h"
#include <cstdio>
#include <new>
#include <vector>
#include <string>
#include <ctime>
#include <type_traits>

// Include vector.h directly for Vector type
typedef float vec_t;
#include "hl/vector.h"

// Include MSLogger with proper path
#include "mslogger.h"

// Forward declarations and external functions for player management
#ifdef VALVE_DLL
    // Avoid header conflicts by using external declarations
    extern "C" {
        // Engine function prototypes
        const char* GetPlayerAuthId_AS(void* edict);
        void EmitSound_AS(void* edict, int channel, const char* sound, float volume, float attn, int flags, int pitch);
        void SendInfoMsg_AS(void* player, const char* message);
        void* PlayerByIndex_AS(int index);
        int GetMaxClients_AS();
        const char* GetPlayerDisplayName_AS(void* player);
        const char* GetPlayerClientAddress_AS(void* player);
        int GetPlayerEntIndex_AS(void* player);
        int IsValidPlayer_AS(void* player);
    }
#endif

// Engine integration - using external function declarations to avoid header conflicts
// These will be linked from the actual game modules at runtime

// External function prototypes for engine integration
extern "C" {
    float GetEngineGameTime();
    const char* GetEngineCvarString(const char* name);
    const char* GetEngineMapName();
    void* CreateEngineEntity(const char* classname);
    void SetEngineEntityOrigin(void* entity, float x, float y, float z);
    void SetEngineEntityName(void* entity, const char* name);
    void SetEngineEntityTargetName(void* entity, const char* targetname);
    void SetEngineEntityHealth(void* entity, float health);
    float GetEngineEntityHealth(void* entity);
    int GetEngineEntityDeadFlag(void* entity);
    const char* GetEngineEntityClassName(void* entity);
    void GetEngineEntityOrigin(void* entity, float* x, float* y, float* z);
}

// Forward declarations for types we need
class CBaseEntity;
class CBasePlayer;

namespace ASEntityBindings
{
    // Forward declarations for casting functions
    CBaseEntity* PlayerToEntity_Cast(CBasePlayer* pPlayer);
    CBasePlayer* EntityToPlayer_Cast(CBaseEntity* pEntity);
    template<class T> T* Template_Cast(CBaseEntity* pEntity);
    
    // Placeholder implementations for entity methods
    // These need to be connected to actual game code later
    
    // CBaseEntity methods
    Vector Entity_GetOrigin(CBaseEntity* pEntity)
    {
        if (!pEntity)
        {
            MS_ANGEL_ERROR("Entity_GetOrigin: NULL entity pointer");
            return Vector(0, 0, 0);
        }
        
        float x, y, z;
        GetEngineEntityOrigin((void*)pEntity, &x, &y, &z);
        MS_ANGEL_DEBUG("Entity_GetOrigin: (%f, %f, %f)", x, y, z);
        return Vector(x, y, z);
    }
    
    std::string Entity_GetClassName(CBaseEntity* pEntity)
    {
        if (!pEntity)
        {
            MS_ANGEL_ERROR("Entity_GetClassName: NULL entity pointer");
            return "null_entity";
        }
        
        const char* className = GetEngineEntityClassName((void*)pEntity);
        std::string result = className ? className : "unknown_entity";
        MS_ANGEL_DEBUG("Entity_GetClassName: %s", result.c_str());
        return result;
    }
    
    void Entity_SetOrigin(const Vector& origin, CBaseEntity* pEntity)
    {
        if (!pEntity)
        {
            MS_ANGEL_ERROR("Entity_SetOrigin: NULL entity pointer");
            return;
        }
        
        SetEngineEntityOrigin((void*)pEntity, origin.x, origin.y, origin.z);
        MS_ANGEL_DEBUG("Entity_SetOrigin: Set to (%f, %f, %f)", origin.x, origin.y, origin.z);
    }
    
    bool Entity_IsAlive(CBaseEntity* pEntity)
    {
        if (!pEntity)
        {
            MS_ANGEL_DEBUG("Entity_IsAlive: NULL entity pointer - considered dead");
            return false;
        }
        
        float health = GetEngineEntityHealth((void*)pEntity);
        int deadFlag = GetEngineEntityDeadFlag((void*)pEntity);
        
        // Check both health and deadflag (DEAD_NO = 0)
        bool isAlive = (deadFlag == 0) && (health > 0);
        MS_ANGEL_DEBUG("Entity_IsAlive: %s (health: %f, deadflag: %d)", 
                      isAlive ? "alive" : "dead", health, deadFlag);
        return isAlive;
    }
    
    // CBasePlayer methods
    std::string Player_GetName(CBasePlayer* pPlayer)
    {
        if (!pPlayer)
        {
            MS_ANGEL_ERROR("Player_GetName: NULL player pointer");
            return "Unknown Player";
        }
        
#ifdef VALVE_DLL
        const char* displayName = GetPlayerDisplayName_AS((void*)pPlayer);
        std::string result = displayName ? displayName : "Unnamed Player";
#else
        std::string result = "Player"; // Client-side fallback
#endif
        MS_ANGEL_DEBUG("Player_GetName: %s", result.c_str());
        return result;
    }
    
    std::string Player_GetSteamID(CBasePlayer* pPlayer)
    {
        if (!pPlayer)
        {
            MS_ANGEL_ERROR("Player_GetSteamID: NULL player pointer");
            return "STEAM_ID_INVALID";
        }
        
#ifdef VALVE_DLL
        const char* authId = GetPlayerAuthId_AS((void*)pPlayer);
        std::string result = authId ? authId : "STEAM_ID_UNKNOWN";
#else
        std::string result = "STEAM_0:0:00000"; // Client-side fallback
#endif
        MS_ANGEL_DEBUG("Player_GetSteamID: %s", result.c_str());
        return result;
    }
    
    std::string Player_GetIPAddress(CBasePlayer* pPlayer)
    {
        if (!pPlayer)
        {
            MS_ANGEL_ERROR("Player_GetIPAddress: NULL player pointer");
            return "0.0.0.0";
        }
        
#ifdef VALVE_DLL
        // Get IP from player's client address
        const char* address = GetPlayerClientAddress_AS((void*)pPlayer);
        std::string result = address ? address : "Unknown";
#else
        std::string result = "127.0.0.1"; // Client-side fallback
#endif
        MS_ANGEL_DEBUG("Player_GetIPAddress: %s", result.c_str());
        return result;
    }
    
    void Player_SendMessage(const std::string& message, bool reliable, CBasePlayer* pPlayer)
    {
        if (!pPlayer)
        {
            MS_ANGEL_ERROR("Player_SendMessage: NULL player pointer");
            return;
        }
        
        if (message.empty())
        {
            MS_ANGEL_ERROR("Player_SendMessage: Empty message");
            return;
        }
        
#ifdef VALVE_DLL
        // Send as InfoMsg to the player
        SendInfoMsg_AS((void*)pPlayer, message.c_str());
        const char* playerName = GetPlayerDisplayName_AS((void*)pPlayer);
        MS_ANGEL_DEBUG("Player_SendMessage sent to %s: %s", playerName ? playerName : "Unknown", message.c_str());
#else
        MS_ANGEL_DEBUG("Player_SendMessage: %s (client-side stub)", message.c_str());
#endif
    }
    
    void Player_EmitSound(const std::string& sound, float volume, CBasePlayer* pPlayer)
    {
        if (!pPlayer)
        {
            MS_ANGEL_ERROR("Player_EmitSound: NULL player pointer");
            return;
        }
        
        if (sound.empty())
        {
            MS_ANGEL_ERROR("Player_EmitSound: Empty sound name");
            return;
        }
        
        // Clamp volume to valid range
        float clampedVolume = std::max(0.0f, std::min(1.0f, volume));
        
#ifdef VALVE_DLL
        // Emit sound at player's location (CHAN_AUTO=0, ATTN_NORM=0.8, PITCH_NORM=100)
        EmitSound_AS((void*)pPlayer, 0, sound.c_str(), clampedVolume, 0.8f, 0, 100);
        const char* playerName = GetPlayerDisplayName_AS((void*)pPlayer);
        MS_ANGEL_DEBUG("Player_EmitSound: %s at volume %f for %s", sound.c_str(), clampedVolume, playerName ? playerName : "Unknown");
#else
        MS_ANGEL_DEBUG("Player_EmitSound: %s at volume %f (client-side stub)", sound.c_str(), clampedVolume);
#endif
    }
    
    // Global functions
    float AS_GetGameTime()
    {
        float gameTime = GetEngineGameTime();
        MS_ANGEL_DEBUG("GetGameTime: %f", gameTime);
        return gameTime;
    }
    
    // Static player list for GetAllPlayers
    CScriptArray* AS_GetAllPlayers(asIScriptEngine* engine)
    {
        MS_ANGEL_DEBUG("GetAllPlayers called");
        
        // Get the array type for CBasePlayer@
        asITypeInfo* arrayType = engine->GetTypeInfoByDecl("array<CBasePlayer@>");
        if (!arrayType)
        {
            MS_ANGEL_ERROR("Failed to get array<CBasePlayer@> type");
            return nullptr;
        }
        
        // Create a new array
        CScriptArray* array = CScriptArray::Create(arrayType);
        
        // TODO: Populate with actual players
        // For now, return empty array
        
        return array;
    }
    
    std::string AS_GetTimestamp()
    {
        // Get current time
        std::time_t now = std::time(nullptr);
        std::tm* timeinfo = std::localtime(&now);
        
        // Format as YYYY-MM-DD HH:MM:SS
        char buffer[32];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        
        std::string timestamp(buffer);
        MS_ANGEL_DEBUG("GetTimestamp: %s", timestamp.c_str());
        return timestamp;
    }
    
    std::string AS_GetCvar(const std::string& cvarName)
    {
        if (cvarName.empty())
        {
            MS_ANGEL_ERROR("GetCvar: Empty cvar name");
            return "";
        }
        
        const char* value = GetEngineCvarString(cvarName.c_str());
        std::string result = value ? value : "";
        MS_ANGEL_DEBUG("GetCvar '%s': '%s'", cvarName.c_str(), result.c_str());
        return result;
    }
    
    void AS_ChatLog(const std::string& message)
    {
        // TODO: Connect to actual chat logging
        MS_ANGEL_DEBUG("ChatLog: %s", message.c_str());
    }
    
    // Get current map name
    std::string AS_GetMapName()
    {
        const char* mapName = GetEngineMapName();
        std::string result = mapName ? mapName : "unknown_map";
        MS_ANGEL_DEBUG("GetMapName: %s", result.c_str());
        return result;
    }
    
    // Create entity by script name
    void* AS_CreateEntity(const std::string& scriptName)
    {
        if (scriptName.empty())
        {
            MS_ANGEL_ERROR("CreateEntity: Empty script name");
            return nullptr;
        }
        
        void* entity = CreateEngineEntity(scriptName.c_str());
        if (entity)
        {
            MS_ANGEL_INFO("CreateEntity: Successfully created '%s'", scriptName.c_str());
        }
        else
        {
            MS_ANGEL_ERROR("CreateEntity: Failed to create entity '%s'", scriptName.c_str());
        }
        return entity;
    }
    
    // Entity property functions
    void AS_SetEntityName(CBaseEntity* pEntity, const std::string& name)
    {
        if (!pEntity)
        {
            MS_ANGEL_ERROR("SetEntityName: NULL entity pointer");
            return;
        }
        
        SetEngineEntityName((void*)pEntity, name.c_str());
        MS_ANGEL_DEBUG("SetEntityName: Set name to '%s'", name.c_str());
    }
    
    void AS_SetEntityTargetName(CBaseEntity* pEntity, const std::string& targetName)
    {
        if (!pEntity)
        {
            MS_ANGEL_ERROR("SetEntityTargetName: NULL entity pointer");
            return;
        }
        
        SetEngineEntityTargetName((void*)pEntity, targetName.c_str());
        MS_ANGEL_DEBUG("SetEntityTargetName: Set targetname to '%s'", targetName.c_str());
    }
    
    void AS_SetEntityHealth(CBaseEntity* pEntity, float health)
    {
        if (!pEntity)
        {
            MS_ANGEL_ERROR("SetEntityHealth: NULL entity pointer");
            return;
        }
        
        SetEngineEntityHealth((void*)pEntity, health);
        MS_ANGEL_DEBUG("SetEntityHealth: Set health to %f", health);
    }
    
    // Check if entity is dead (more comprehensive than IsAlive)
    bool AS_IsEntityDead(CBaseEntity* pEntity)
    {
        if (!pEntity)
        {
            MS_ANGEL_DEBUG("IsEntityDead: NULL entity pointer - considered dead");
            return true;
        }
        
        float health = GetEngineEntityHealth((void*)pEntity);
        int deadFlag = GetEngineEntityDeadFlag((void*)pEntity);
        
        // Check if entity is dead by health or deadflag (DEAD_NO = 0)
        bool isDead = (deadFlag != 0) || (health <= 0);
        MS_ANGEL_DEBUG("IsEntityDead: %s (health: %f, deadflag: %d)", 
                      isDead ? "dead" : "alive", health, deadFlag);
        return isDead;
    }
    
    // Logging function for scripts (replaces MS_ANGEL_INFO macro)
    void AS_LogAngelInfo(const std::string& message)
    {
        MS_ANGEL_INFO("%s", message.c_str());
    }
    
    void AS_LogAngelDebug(const std::string& message)
    {
        MS_ANGEL_DEBUG("%s", message.c_str());
    }
    
    void AS_LogAngelError(const std::string& message)
    {
        MS_ANGEL_ERROR("%s", message.c_str());
    }
    
    // Casting functions for inheritance support
    CBaseEntity* PlayerToEntity_Cast(CBasePlayer* pPlayer)
    {
        if (!pPlayer)
        {
            MS_ANGEL_DEBUG("PlayerToEntity_Cast: NULL player pointer");
            return nullptr;
        }
        
        // Direct cast - CBasePlayer inherits from CBaseEntity
        CBaseEntity* pEntity = reinterpret_cast<CBaseEntity*>(pPlayer);
        MS_ANGEL_DEBUG("PlayerToEntity_Cast: Successfully cast player to entity");
        return pEntity;
    }
    
    CBasePlayer* EntityToPlayer_Cast(CBaseEntity* pEntity)
    {
        if (!pEntity)
        {
            MS_ANGEL_DEBUG("EntityToPlayer_Cast: NULL entity pointer");
            return nullptr;
        }
        
        // Check if this entity is actually a player
        // This should integrate with actual game logic to validate entity type
        std::string className = Entity_GetClassName(pEntity);
        if (className != "player" && className != "CBasePlayer")
        {
            MS_ANGEL_DEBUG("EntityToPlayer_Cast: Entity '%s' is not a player", className.c_str());
            return nullptr;
        }
        
        CBasePlayer* pPlayer = reinterpret_cast<CBasePlayer*>(pEntity);
        MS_ANGEL_DEBUG("EntityToPlayer_Cast: Successfully cast entity to player");
        return pPlayer;
    }
    
    // Template casting function for AngelScript cast<T>() support
    template<class T>
    T* Template_Cast(CBaseEntity* pEntity)
    {
        if (!pEntity)
        {
            MS_ANGEL_DEBUG("Template_Cast: NULL entity pointer");
            return nullptr;
        }
        
        // For now, only support CBasePlayer casting
        // This can be extended for other entity types later
        if (std::is_same<T, CBasePlayer>::value)
        {
            return reinterpret_cast<T*>(EntityToPlayer_Cast(pEntity));
        }
        
        MS_ANGEL_DEBUG("Template_Cast: Unsupported cast type");
        return nullptr;
    }
    
    // Additional player management functions
    std::string AS_GetPlayerCurrentMap()
    {
        // Get current map name - this is the same for all players on the server
        const char* mapName = GetEngineMapName();
        std::string result = mapName ? mapName : "unknown_map";
        MS_ANGEL_DEBUG("GetPlayerCurrentMap: %s", result.c_str());
        return result;
    }
    
    // Global current player context (set during script execution)
    static void* g_pCurrentScriptPlayer = nullptr;
    
    void AS_SetCurrentPlayerContext(CBasePlayer* pPlayer)
    {
        g_pCurrentScriptPlayer = (void*)pPlayer;
#ifdef VALVE_DLL
        const char* playerName = pPlayer ? GetPlayerDisplayName_AS((void*)pPlayer) : "NULL";
        MS_ANGEL_DEBUG("SetCurrentPlayerContext: %s", playerName);
#else
        MS_ANGEL_DEBUG("SetCurrentPlayerContext: %p", pPlayer);
#endif
    }
    
    int AS_GetCurrentPlayerID()
    {
        if (!g_pCurrentScriptPlayer)
        {
            MS_ANGEL_ERROR("GetCurrentPlayerID: No player context set");
            return -1;
        }
        
#ifdef VALVE_DLL
        int playerID = GetPlayerEntIndex_AS(g_pCurrentScriptPlayer);
        const char* playerName = GetPlayerDisplayName_AS(g_pCurrentScriptPlayer);
        MS_ANGEL_DEBUG("GetCurrentPlayerID: %d (%s)", playerID, playerName ? playerName : "Unknown");
        return playerID;
#else
        MS_ANGEL_DEBUG("GetCurrentPlayerID: stub - returning -1");
        return -1;
#endif
    }
    
    void AS_SendPlayerMessage(const std::string& playerName, const std::string& title, const std::string& message)
    {
        if (playerName.empty() || message.empty())
        {
            MS_ANGEL_ERROR("SendPlayerMessage: Empty player name or message");
            return;
        }
        
#ifdef VALVE_DLL
        // Find player by name
        int maxClients = GetMaxClients_AS();
        for (int i = 1; i <= maxClients; i++)
        {
            void* pPlayer = PlayerByIndex_AS(i);
            if (!pPlayer || !IsValidPlayer_AS(pPlayer))
                continue;
                
            // Compare display names
            const char* displayName = GetPlayerDisplayName_AS(pPlayer);
            if (displayName && strcmp(displayName, playerName.c_str()) == 0)
            {
                // Send HUD message with title if provided
                if (!title.empty())
                {
                    std::string fullMsg = title + "\\n" + message;
                    SendInfoMsg_AS(pPlayer, fullMsg.c_str());
                }
                else
                {
                    SendInfoMsg_AS(pPlayer, message.c_str());
                }
                
                MS_ANGEL_DEBUG("SendPlayerMessage sent to %s: [%s] %s", 
                              playerName.c_str(), title.c_str(), message.c_str());
                return;
            }
        }
        
        MS_ANGEL_ERROR("SendPlayerMessage: Player '%s' not found", playerName.c_str());
#else
        MS_ANGEL_DEBUG("SendPlayerMessage: %s - [%s] %s (client-side stub)", 
                      playerName.c_str(), title.c_str(), message.c_str());
#endif
    }
    
    void RegisterEntityInheritance(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("[ASEntityBindings] Setting up entity inheritance...");
        
        // Since this AngelScript version doesn't have implicit/explicit casting behaviors,
        // we'll use global functions for casting operations
        
        // Register template casting function for cast<T>() support
        int r = pEngine->RegisterGlobalFunction("CBasePlayer@ cast(CBaseEntity@)", 
            asFUNCTIONPR(Template_Cast<CBasePlayer>, (CBaseEntity*), CBasePlayer*), asCALL_CDECL);
        if (r < 0) {
            MS_ANGEL_ERROR("Failed to register template cast<CBasePlayer> function");
        } else {
            MS_ANGEL_DEBUG("   ✓ Template cast<CBasePlayer@>() function registered");
        }
        
        // Register convenience casting functions
        r = pEngine->RegisterGlobalFunction("CBasePlayer@ ToPlayer(CBaseEntity@)", 
            asFUNCTION(EntityToPlayer_Cast), asCALL_CDECL);
        if (r < 0) {
            MS_ANGEL_ERROR("Failed to register ToPlayer() function");
        } else {
            MS_ANGEL_DEBUG("   ✓ ToPlayer() function registered");
        }
        
        r = pEngine->RegisterGlobalFunction("CBaseEntity@ ToEntity(CBasePlayer@)", 
            asFUNCTION(PlayerToEntity_Cast), asCALL_CDECL);
        if (r < 0) {
            MS_ANGEL_ERROR("Failed to register ToEntity() function");
        } else {
            MS_ANGEL_DEBUG("   ✓ ToEntity() function registered");
        }
        
        MS_ANGEL_INFO("[ASEntityBindings] Entity inheritance setup complete");
        MS_ANGEL_INFO("   Note: Use cast<CBasePlayer@>(entity) or ToPlayer(entity) for casting");
    }
    
    void RegisterEntityTypes(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("[ASEntityBindings] Registering entity types...");
        
        // First register CBaseEntity as a reference type
        int r = pEngine->RegisterObjectType("CBaseEntity", 0, asOBJ_REF | asOBJ_NOCOUNT);
        if (r < 0) {
            MS_ANGEL_ERROR("Failed to register CBaseEntity type");
            return;
        }
        MS_ANGEL_DEBUG("   ✓ CBaseEntity type registered");
        
        // Register CBaseEntity methods
        r = pEngine->RegisterObjectMethod("CBaseEntity", "Vector3 GetOrigin() const", 
            asFUNCTION(Entity_GetOrigin), asCALL_CDECL_OBJLAST);
        if (r < 0) MS_ANGEL_ERROR("Failed to register CBaseEntity::GetOrigin");
        
        r = pEngine->RegisterObjectMethod("CBaseEntity", "string GetClassName() const", 
            asFUNCTION(Entity_GetClassName), asCALL_CDECL_OBJLAST);
        if (r < 0) MS_ANGEL_ERROR("Failed to register CBaseEntity::GetClassName");
        
        r = pEngine->RegisterObjectMethod("CBaseEntity", "void SetOrigin(const Vector3 &in)", 
            asFUNCTION(Entity_SetOrigin), asCALL_CDECL_OBJLAST);
        if (r < 0) MS_ANGEL_ERROR("Failed to register CBaseEntity::SetOrigin");
        
        r = pEngine->RegisterObjectMethod("CBaseEntity", "bool IsAlive() const", 
            asFUNCTION(Entity_IsAlive), asCALL_CDECL_OBJLAST);
        if (r < 0) MS_ANGEL_ERROR("Failed to register CBaseEntity::IsAlive");
        
        // Now register CBasePlayer as inheriting from CBaseEntity
        r = pEngine->RegisterObjectType("CBasePlayer", 0, asOBJ_REF | asOBJ_NOCOUNT);
        if (r < 0) {
            MS_ANGEL_ERROR("Failed to register CBasePlayer type");
            return;
        }
        MS_ANGEL_DEBUG("   ✓ CBasePlayer type registered");
        
        // Since AngelScript inheritance isn't working as expected in this version,
        // we need to manually register CBaseEntity methods on CBasePlayer as well
        r = pEngine->RegisterObjectMethod("CBasePlayer", "Vector3 GetOrigin() const", 
            asFUNCTION(Entity_GetOrigin), asCALL_CDECL_OBJLAST);
        if (r < 0) MS_ANGEL_ERROR("Failed to register CBasePlayer::GetOrigin");
        
        r = pEngine->RegisterObjectMethod("CBasePlayer", "string GetClassName() const", 
            asFUNCTION(Entity_GetClassName), asCALL_CDECL_OBJLAST);
        if (r < 0) MS_ANGEL_ERROR("Failed to register CBasePlayer::GetClassName");
        
        r = pEngine->RegisterObjectMethod("CBasePlayer", "void SetOrigin(const Vector3 &in)", 
            asFUNCTION(Entity_SetOrigin), asCALL_CDECL_OBJLAST);
        if (r < 0) MS_ANGEL_ERROR("Failed to register CBasePlayer::SetOrigin");
        
        r = pEngine->RegisterObjectMethod("CBasePlayer", "bool IsAlive() const", 
            asFUNCTION(Entity_IsAlive), asCALL_CDECL_OBJLAST);
        if (r < 0) MS_ANGEL_ERROR("Failed to register CBasePlayer::IsAlive");
        
        // Now register CBasePlayer-specific methods
        
        // Register CBasePlayer specific methods
        r = pEngine->RegisterObjectMethod("CBasePlayer", "string GetName() const", 
            asFUNCTION(Player_GetName), asCALL_CDECL_OBJLAST);
        if (r < 0) MS_ANGEL_ERROR("Failed to register CBasePlayer::GetName");
        
        r = pEngine->RegisterObjectMethod("CBasePlayer", "string GetSteamID() const", 
            asFUNCTION(Player_GetSteamID), asCALL_CDECL_OBJLAST);
        if (r < 0) MS_ANGEL_ERROR("Failed to register CBasePlayer::GetSteamID");
        
        r = pEngine->RegisterObjectMethod("CBasePlayer", "string GetIPAddress() const", 
            asFUNCTION(Player_GetIPAddress), asCALL_CDECL_OBJLAST);
        if (r < 0) MS_ANGEL_ERROR("Failed to register CBasePlayer::GetIPAddress");
        
        r = pEngine->RegisterObjectMethod("CBasePlayer", "void SendMessage(const string &in, bool = true)", 
            asFUNCTION(Player_SendMessage), asCALL_CDECL_OBJLAST);
        if (r < 0) MS_ANGEL_ERROR("Failed to register CBasePlayer::SendMessage");
        
        r = pEngine->RegisterObjectMethod("CBasePlayer", "void EmitSound(const string &in, float = 1.0f)", 
            asFUNCTION(Player_EmitSound), asCALL_CDECL_OBJLAST);
        if (r < 0) MS_ANGEL_ERROR("Failed to register CBasePlayer::EmitSound");
        
        // Set up inheritance relationships after both types are registered
        RegisterEntityInheritance(pEngine);
        
        MS_ANGEL_INFO("[ASEntityBindings] Entity types registered successfully");
    }
    
    void RegisterGlobalFunctions(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("[ASEntityBindings] Registering global entity functions...");
        
        // Register GetGameTime (override the one from ASBuiltinFunctions if needed)
        int r = pEngine->RegisterGlobalFunction("float GetGameTime()", 
            asFUNCTION(AS_GetGameTime), asCALL_CDECL);
        if (r < 0) MS_ANGEL_ERROR("Failed to register GetGameTime");
        
        // Register GetAllPlayers
        r = pEngine->RegisterGlobalFunction("array<CBasePlayer@>@ GetAllPlayers()", 
            asFUNCTION(AS_GetAllPlayers), asCALL_CDECL);
        if (r < 0) MS_ANGEL_ERROR("Failed to register GetAllPlayers");
        
        // Register GetTimestamp
        r = pEngine->RegisterGlobalFunction("string GetTimestamp()", 
            asFUNCTION(AS_GetTimestamp), asCALL_CDECL);
        if (r < 0) MS_ANGEL_ERROR("Failed to register GetTimestamp");
        
        // Register GetCvar
        r = pEngine->RegisterGlobalFunction("string GetCvar(const string &in)", 
            asFUNCTION(AS_GetCvar), asCALL_CDECL);
        if (r < 0) MS_ANGEL_ERROR("Failed to register GetCvar");
        
        // Register GetMapName
        r = pEngine->RegisterGlobalFunction("string GetMapName()", 
            asFUNCTION(AS_GetMapName), asCALL_CDECL);
        if (r < 0) MS_ANGEL_ERROR("Failed to register GetMapName");
        
        // Register CreateEntity
        r = pEngine->RegisterGlobalFunction("EntityHandle CreateEntity(const string &in)", 
            asFUNCTION(AS_CreateEntity), asCALL_CDECL);
        if (r < 0) MS_ANGEL_ERROR("Failed to register CreateEntity");
        
        // Register entity property functions
        r = pEngine->RegisterGlobalFunction("void SetEntityName(EntityHandle, const string &in)", 
            asFUNCTION(AS_SetEntityName), asCALL_CDECL);
        if (r < 0) MS_ANGEL_ERROR("Failed to register SetEntityName");
        
        r = pEngine->RegisterGlobalFunction("void SetEntityTargetName(EntityHandle, const string &in)", 
            asFUNCTION(AS_SetEntityTargetName), asCALL_CDECL);
        if (r < 0) MS_ANGEL_ERROR("Failed to register SetEntityTargetName");
        
        r = pEngine->RegisterGlobalFunction("void SetEntityHealth(EntityHandle, float)", 
            asFUNCTION(AS_SetEntityHealth), asCALL_CDECL);
        if (r < 0) MS_ANGEL_ERROR("Failed to register SetEntityHealth");
        
        // Register IsEntityDead
        r = pEngine->RegisterGlobalFunction("bool IsEntityDead(EntityHandle)", 
            asFUNCTION(AS_IsEntityDead), asCALL_CDECL);
        if (r < 0) MS_ANGEL_ERROR("Failed to register IsEntityDead");
        
        // Register ChatLog
        r = pEngine->RegisterGlobalFunction("void ChatLog(const string &in)", 
            asFUNCTION(AS_ChatLog), asCALL_CDECL);
        if (r < 0) MS_ANGEL_ERROR("Failed to register ChatLog");
        
        // Register player management functions
        r = pEngine->RegisterGlobalFunction("string GetPlayerCurrentMap()", 
            asFUNCTION(AS_GetPlayerCurrentMap), asCALL_CDECL);
        if (r < 0) MS_ANGEL_ERROR("Failed to register GetPlayerCurrentMap");
        
        r = pEngine->RegisterGlobalFunction("int GetCurrentPlayerID()", 
            asFUNCTION(AS_GetCurrentPlayerID), asCALL_CDECL);
        if (r < 0) MS_ANGEL_ERROR("Failed to register GetCurrentPlayerID");
        
        r = pEngine->RegisterGlobalFunction("void SendPlayerMessage(const string &in, const string &in, const string &in)", 
            asFUNCTION(AS_SendPlayerMessage), asCALL_CDECL);
        if (r < 0) MS_ANGEL_ERROR("Failed to register SendPlayerMessage");
        
        // Register logging functions to replace MS_ANGEL_INFO macro
        r = pEngine->RegisterGlobalFunction("void MS_ANGEL_INFO(const string &in)", 
            asFUNCTION(AS_LogAngelInfo), asCALL_CDECL);
        if (r < 0) MS_ANGEL_ERROR("Failed to register MS_ANGEL_INFO");
        
        r = pEngine->RegisterGlobalFunction("void MS_ANGEL_DEBUG(const string &in)", 
            asFUNCTION(AS_LogAngelDebug), asCALL_CDECL);
        if (r < 0) MS_ANGEL_ERROR("Failed to register MS_ANGEL_DEBUG");
        
        r = pEngine->RegisterGlobalFunction("void MS_ANGEL_ERROR(const string &in)", 
            asFUNCTION(AS_LogAngelError), asCALL_CDECL);
        if (r < 0) MS_ANGEL_ERROR("Failed to register MS_ANGEL_ERROR");
        
        MS_ANGEL_INFO("[ASEntityBindings] Global entity functions registered successfully");
    }
    
    void RegisterAll(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("[ASEntityBindings] Starting entity bindings registration...");
        
        RegisterEntityTypes(pEngine);
        RegisterGlobalFunctions(pEngine);
        
        MS_ANGEL_INFO("[ASEntityBindings] Entity bindings registration complete");
    }
}