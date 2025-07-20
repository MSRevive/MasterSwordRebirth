//==========================================================================
// ASEntityBindings.cpp - asbind20 Implementation
// 
// Entity-related type bindings for AngelScript integration
// Provides CBaseEntity, CBasePlayer, and related functions for script usage
// Uses asbind20 for clean, type-safe binding syntax
//==========================================================================

// Include standard library headers first
#include <cstdio>
#include <new>
#include <vector>
#include <string>
#include <ctime>
#include <cmath>    // For sqrt in vector.h
#include <type_traits>
#include <angelscript.h>  // For asIContext and asGetActiveContext
#include "addons/scriptarray/scriptarray.h"

// Include asbind20 for modern binding syntax
#include "../../../../../thirdparty/asbind20/asbind.hpp"

// Include vector.h directly for Vector type
typedef float vec_t;
#include "../../../server/hl/vector.h"

// Include MSLogger with proper path
#include "mslogger.h"

// Include real Master Sword entity classes first
#ifdef CLIENT_DLL
    // Client-side includes
    #include "hud.h"
    #include "cl_util.h"
#else
    // Server-side includes
    #include "extdll.h"
    #include "util.h"
    #include "hl/cbase.h"
    #include "player/player.h"
#endif

// Include AngelScript headers after engine headers
// Using asbind20 for clean, type-safe binding syntax
#include "ASEngineInterface.h"
#include "ASEngineBindings.h"
#include "ASCoreTypes.h"

// Note: All external C function declarations have been removed.
// All engine integration now uses ASEngineProvider directly.

// Engine globals - declared globally to avoid namespace issues
#ifndef CLIENT_DLL
extern globalvars_t *gpGlobals;
extern enginefuncs_t g_engfuncs;
#endif

namespace ASEntityBindings
{
    // Forward declarations for casting functions
    CBaseEntity* PlayerToEntity_Cast(CBasePlayer* pPlayer);
    CBasePlayer* EntityToPlayer_Cast(CBaseEntity* pEntity);
    template<class T> T* Template_Cast(void* pEntity);
    
    // Reference counting functions for AngelScript reference types
    // Note: These are no-ops since entities are managed by the game engine
    void AddRef_CBaseEntity(CBaseEntity* entity)
    {
        // No-op: Engine manages entity lifetime
        MS_ANGEL_DEBUG("AddRef_CBaseEntity called for entity %p", entity);
    }
    
    void Release_CBaseEntity(CBaseEntity* entity) 
    {
        // No-op: Engine manages entity lifetime
        MS_ANGEL_DEBUG("Release_CBaseEntity called for entity %p", entity);
    }
    
    void AddRef_CBasePlayer(CBasePlayer* player)
    {
        // No-op: Engine manages player lifetime
        MS_ANGEL_DEBUG("AddRef_CBasePlayer called for player %p", player);
    }
    
    void Release_CBasePlayer(CBasePlayer* player)
    {
        // No-op: Engine manages player lifetime
        MS_ANGEL_DEBUG("Release_CBasePlayer called for player %p", player);
    }
    
    // Helper functions for CBaseEntity methods that need extra logic
    Vector GetEntityOrigin(CBaseEntity* pEntity)
    {
        if (!pEntity || !pEntity->pev)
            return Vector(0, 0, 0);
        return pEntity->pev->origin;
    }
    
    std::string GetEntityClassName(CBaseEntity* pEntity)
    {
        if (!pEntity || !pEntity->pev)
            return "null_entity";
        return STRING(pEntity->pev->classname);
    }
    
    void SetEntityOrigin(CBaseEntity* pEntity, const Vector& origin)
    {
        if (!pEntity || !pEntity->pev)
            return;
        UTIL_SetOrigin(pEntity->pev, origin);
    }
    
    float GetEntityHealth(CBaseEntity* pEntity)
    {
        if (!pEntity || !pEntity->pev)
            return 0.0f;
        return pEntity->pev->health;
    }
    
    void SetEntityHealth(CBaseEntity* pEntity, float health)
    {
        if (!pEntity || !pEntity->pev)
            return;
        pEntity->pev->health = health;
    }
    
    // Helper functions for CBasePlayer methods
    std::string GetPlayerDisplayName(CBasePlayer* pPlayer)
    {
        if (!pPlayer || !pPlayer->pev)
            return "Unknown Player";
        return STRING(pPlayer->pev->netname);
    }
    
    std::string GetPlayerAuthID(CBasePlayer* pPlayer)
    {
        if (!pPlayer)
            return "STEAM_ID_INVALID";
        return pPlayer->AuthID().c_str();
    }
    
    std::string GetPlayerTitle(CBasePlayer* pPlayer)
    {
        if (!pPlayer)
            return "Unknown";
        return pPlayer->GetTitle();
    }
    
    float GetPlayerMaxHP(CBasePlayer* pPlayer)
    {
        if (!pPlayer)
            return 0.0f;
        return pPlayer->MaxHP();
    }
    
    float GetPlayerMaxMP(CBasePlayer* pPlayer)
    {
        if (!pPlayer)
            return 0.0f;
        return pPlayer->MaxMP();
    }
    
    // Global functions - use ASEngineProvider for cross-platform compatibility
    float AS_GetGameTime()
    {
        float gameTime = ASEngineProvider::GetGameTime();
        MS_ANGEL_DEBUG("GetGameTime: %f", gameTime);
        return gameTime;
    }
    
    // Static player list for GetAllPlayers - uses real CBasePlayer instances
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
        
#ifndef CLIENT_DLL
        // Server-side: Populate with actual connected players using real engine functions
        
        for (int i = 1; i <= gpGlobals->maxClients; i++)
        {
            edict_t* pEdict = g_engfuncs.pfnPEntityOfEntIndex(i);
            if (!pEdict || pEdict->free)
                continue;
                
            CBaseEntity* pEntity = CBaseEntity::Instance(pEdict);
            if (!pEntity || !pEntity->IsPlayer())
                continue;
                
            CBasePlayer* pPlayer = static_cast<CBasePlayer*>(pEntity);
            // Validate player is connected and has valid data
            if (pPlayer && pPlayer->pev && (pPlayer->pev->flags & FL_CLIENT))
            {
                array->InsertLast(&pPlayer);
                MS_ANGEL_DEBUG("Added player %d: %s", i, STRING(pPlayer->pev->netname));
            }
        }
#else
        // Client-side: Can't access other players, return empty array
        MS_ANGEL_DEBUG("GetAllPlayers: Client-side, returning empty array");
#endif
        
        MS_ANGEL_DEBUG("GetAllPlayers: Added %d players to array", array->GetSize());
        return array;
    }
    
    // Player management functions for AngelScript
    int AS_GetPlayerCount()
    {
        int count = ASEngineProvider::GetPlayerCount();
        MS_ANGEL_DEBUG("GetPlayerCount: %d players", count);
        return count;
    }
    
    CBasePlayer* AS_PlayerByIndex(int index)
    {
#ifndef CLIENT_DLL
        
        if (index < 1 || index > gpGlobals->maxClients)
        {
            MS_ANGEL_ERROR("PlayerByIndex: Invalid index %d", index);
            return nullptr;
        }
        
        edict_t* pEdict = g_engfuncs.pfnPEntityOfEntIndex(index);
        if (!pEdict || pEdict->free)
        {
            MS_ANGEL_DEBUG("PlayerByIndex: No valid edict at index %d", index);
            return nullptr;
        }
        
        CBaseEntity* pEntity = CBaseEntity::Instance(pEdict);
        if (!pEntity || !pEntity->IsPlayer())
        {
            MS_ANGEL_DEBUG("PlayerByIndex: Entity at index %d is not a player", index);
            return nullptr;
        }
        
        CBasePlayer* pPlayer = static_cast<CBasePlayer*>(pEntity);
        if (!pPlayer->pev || !(pPlayer->pev->flags & FL_CLIENT))
        {
            MS_ANGEL_DEBUG("PlayerByIndex: Player at index %d is not connected", index);
            return nullptr;
        }
        
        MS_ANGEL_DEBUG("PlayerByIndex: Found connected player at index %d: %s", index, STRING(pPlayer->pev->netname));
        return pPlayer;
#else
        // Client-side: Can't access other players
        MS_ANGEL_DEBUG("PlayerByIndex: Client-side, returning nullptr");
        return nullptr;
#endif
    }
    
    bool AS_IsConnected(CBasePlayer* pPlayer)
    {
        if (!pPlayer || !pPlayer->pev)
        {
            MS_ANGEL_DEBUG("IsConnected: NULL player pointer or pev");
            return false;
        }
        
        // Check if player is connected using real engine data
        bool connected = !(pPlayer->pev->flags & FL_SPECTATOR) && pPlayer->IsAlive();
        MS_ANGEL_DEBUG("IsConnected: Player %s", connected ? "connected" : "disconnected");
        return connected;
    }
    
    std::string AS_GetDisplayName(CBasePlayer* pPlayer)
    {
        if (!pPlayer)
        {
            MS_ANGEL_ERROR("GetDisplayName: NULL player pointer");
            return "Unknown Player";
        }
        
        std::string name = GetPlayerDisplayName(pPlayer);
        MS_ANGEL_DEBUG("GetDisplayName: '%s'", name.c_str());
        return name;
    }
    
    std::string AS_GetSteamID(CBasePlayer* pPlayer)
    {
        if (!pPlayer)
        {
            MS_ANGEL_ERROR("GetSteamID: NULL player pointer");
            return "STEAM_ID_INVALID";
        }
        
        std::string steamID = GetPlayerAuthID(pPlayer);
        MS_ANGEL_DEBUG("GetSteamID: '%s'", steamID.c_str());
        return steamID;
    }
    
    bool AS_IsAdmin(CBasePlayer* pPlayer)
    {
        if (!pPlayer)
        {
            MS_ANGEL_DEBUG("IsAdmin: NULL player pointer");
            return false;
        }
        
        // Use ASEngineProvider for admin check since this requires server-specific logic
        bool isAdmin = ASEngineProvider::IsPlayerAdmin((void*)pPlayer);
        MS_ANGEL_DEBUG("IsAdmin: Player %s admin", isAdmin ? "is" : "is not");
        return isAdmin;
    }
    
    std::string AS_GetClientAddress(CBasePlayer* pPlayer)
    {
        if (!pPlayer)
        {
            MS_ANGEL_ERROR("GetClientAddress: NULL player pointer");
            return "0.0.0.0";
        }
        
        // Use ASEngineProvider for client address since this requires engine access
        std::string address = ASEngineProvider::GetPlayerClientAddress((void*)pPlayer);
        MS_ANGEL_DEBUG("GetClientAddress: '%s'", address.c_str());
        return address;
    }
    
    // Quest data functions
    std::string AS_GetPlayerQuestData(const std::string& playerID, const std::string& key)
    {
        if (playerID.empty() || key.empty())
        {
            MS_ANGEL_ERROR("GetPlayerQuestData: Empty playerID or key");
            return "";
        }
        
        std::string value = ASEngineProvider::GetPlayerQuestData(playerID, key);
        MS_ANGEL_DEBUG("GetPlayerQuestData: Player '%s', key '%s' = '%s'", 
                      playerID.c_str(), key.c_str(), value.c_str());
        return value;
    }
    
    void AS_SetPlayerQuestData(const std::string& playerID, const std::string& key, const std::string& value)
    {
        if (playerID.empty() || key.empty())
        {
            MS_ANGEL_ERROR("SetPlayerQuestData: Empty playerID or key");
            return;
        }
        
        ASEngineProvider::SetPlayerQuestData(playerID, key, value);
        MS_ANGEL_DEBUG("SetPlayerQuestData: Player '%s', set key '%s' = '%s'", 
                      playerID.c_str(), key.c_str(), value.c_str());
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
        
        std::string result = ASEngineProvider::GetCvarString(cvarName);
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
        std::string result = ASEngineProvider::GetMapName();
        MS_ANGEL_DEBUG("GetMapName: %s", result.c_str());
        return result;
    }
    
    // Create entity by script name (returns EntityHandle)
    EntityHandle AS_CreateEntity(const std::string& scriptName)
    {
        if (scriptName.empty())
        {
            MS_ANGEL_ERROR("CreateEntity: Empty script name");
            return EntityHandle(0);
        }
        
        void* entity = ASEngineProvider::CreateEntity(scriptName);
        if (entity)
        {
            MS_ANGEL_INFO("CreateEntity: Successfully created '%s'", scriptName.c_str());
            return EntityHandle(reinterpret_cast<int>(entity));
        }
        else
        {
            MS_ANGEL_ERROR("CreateEntity: Failed to create entity '%s'", scriptName.c_str());
            return EntityHandle(0);
        }
    }
    
    // Entity property functions using EntityHandle
    void AS_SetEntityName(const EntityHandle& handle, const std::string& name)
    {
        void* entity = reinterpret_cast<void*>(handle.value);
        if (!entity)
        {
            MS_ANGEL_ERROR("SetEntityName: NULL entity pointer");
            return;
        }
        
        ASEngineProvider::SetEntityName(entity, name);
        MS_ANGEL_DEBUG("SetEntityName: Set name to '%s'", name.c_str());
    }
    
    void AS_SetEntityTargetName(const EntityHandle& handle, const std::string& targetName)
    {
        void* entity = reinterpret_cast<void*>(handle.value);
        if (!entity)
        {
            MS_ANGEL_ERROR("SetEntityTargetName: NULL entity pointer");
            return;
        }
        
        ASEngineProvider::SetEntityTargetName(entity, targetName);
        MS_ANGEL_DEBUG("SetEntityTargetName: Set targetname to '%s'", targetName.c_str());
    }
    
    void AS_SetEntityHealth(const EntityHandle& handle, float health)
    {
        void* entity = reinterpret_cast<void*>(handle.value);
        if (!entity)
        {
            MS_ANGEL_ERROR("SetEntityHealth: NULL entity pointer");
            return;
        }
        
        ASEngineProvider::SetEntityHealth(entity, health);
        MS_ANGEL_DEBUG("SetEntityHealth: Set health to %f", health);
    }
    
    // Check if entity is dead (more comprehensive than IsAlive)
    bool AS_IsEntityDead(const EntityHandle& handle)
    {
        void* entity = reinterpret_cast<void*>(handle.value);
        if (!entity)
        {
            MS_ANGEL_DEBUG("IsEntityDead: NULL entity pointer - considered dead");
            return true;
        }
        
        float health = ASEngineProvider::GetEntityHealth(entity);
        int deadFlag = ASEngineProvider::GetEntityDeadFlag(entity);
        
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
        
        // Safe cast - CBasePlayer inherits from CBaseEntity (through CMSMonster)
        MS_ANGEL_DEBUG("PlayerToEntity_Cast: Successfully cast player to entity");
        return static_cast<CBaseEntity*>(pPlayer);
    }
    
    // Note: RefCastPlayer function removed - asbind20 handles inheritance automatically
    
    CBasePlayer* EntityToPlayer_Cast(CBaseEntity* pEntity)
    {
        if (!pEntity)
        {
            MS_ANGEL_DEBUG("EntityToPlayer_Cast: NULL entity pointer");
            return nullptr;
        }
        
        // Check if this entity is actually a player using the real IsPlayer() method
        if (!pEntity->IsPlayer())
        {
            MS_ANGEL_DEBUG("EntityToPlayer_Cast: Entity is not a player");
            return nullptr;
        }
        
        MS_ANGEL_DEBUG("EntityToPlayer_Cast: Successfully cast entity to player");
        return static_cast<CBasePlayer*>(pEntity);
    }
    
    // Template casting function for AngelScript cast<T>() support
    template<class T>
    T* Template_Cast(void* pEntity)
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
            return reinterpret_cast<T*>(EntityToPlayer_Cast(reinterpret_cast<CBaseEntity*>(pEntity)));
        }
        
        MS_ANGEL_DEBUG("Template_Cast: Unsupported cast type");
        return nullptr;
    }
    
    // Additional player management functions
    std::string AS_GetPlayerCurrentMap()
    {
        // Get current map name - this is the same for all players on the server
        std::string result = ASEngineProvider::GetMapName();
        MS_ANGEL_DEBUG("GetPlayerCurrentMap: %s", result.c_str());
        return result;
    }
    
    // Global current player context (set during script execution)
    static void* g_pCurrentScriptPlayer = nullptr;
    
    void AS_SetCurrentPlayerContext(CBasePlayer* pPlayer)
    {
        g_pCurrentScriptPlayer = (void*)pPlayer;
        std::string playerName = pPlayer ? ASEngineProvider::GetPlayerDisplayName((void*)pPlayer) : "NULL";
        MS_ANGEL_DEBUG("SetCurrentPlayerContext: %s", playerName.c_str());
    }
    
    int AS_GetCurrentPlayerID()
    {
        if (!g_pCurrentScriptPlayer)
        {
            MS_ANGEL_ERROR("GetCurrentPlayerID: No player context set");
            return -1;
        }
        
        int playerID = ASEngineProvider::GetPlayerEntIndex(g_pCurrentScriptPlayer);
        std::string playerName = ASEngineProvider::GetPlayerDisplayName(g_pCurrentScriptPlayer);
        MS_ANGEL_DEBUG("GetCurrentPlayerID: %d (%s)", playerID, playerName.c_str());
        return playerID;
    }
    
    void AS_SendPlayerMessage(const std::string& playerName, const std::string& title, const std::string& message)
    {
        if (playerName.empty() || message.empty())
        {
            MS_ANGEL_ERROR("SendPlayerMessage: Empty player name or message");
            return;
        }
        
        // Find player by name using ASEngineProvider
        int maxClients = ASEngineProvider::GetMaxClients();
        for (int i = 1; i <= maxClients; i++)
        {
            void* pPlayer = ASEngineProvider::PlayerByIndex(i);
            if (!pPlayer || !ASEngineProvider::IsValidPlayer(pPlayer))
                continue;
                
            // Compare display names
            std::string displayName = ASEngineProvider::GetPlayerDisplayName(pPlayer);
            if (displayName == playerName)
            {
                // Send HUD message with title if provided
                if (!title.empty())
                {
                    std::string fullMsg = title + "\\n" + message;
                    ASEngineProvider::SendInfoMsg(pPlayer, fullMsg);
                }
                else
                {
                    ASEngineProvider::SendInfoMsg(pPlayer, message);
                }
                
                MS_ANGEL_DEBUG("SendPlayerMessage sent to %s: [%s] %s", 
                              playerName.c_str(), title.c_str(), message.c_str());
                return;
            }
        }
        
        MS_ANGEL_ERROR("SendPlayerMessage: Player '%s' not found", playerName.c_str());
    }
    
    
    // Global engine pointer for wrapper functions
    static asIScriptEngine* g_pStaticEngine = nullptr;
    
    // Wrapper function that doesn't need asIContext
    CScriptArray* AS_GetAllPlayersWrapper()
    {
        if (!g_pStaticEngine) {
            MS_ANGEL_ERROR("AS_GetAllPlayersWrapper: Engine not initialized");
            return nullptr;
        }
        return AS_GetAllPlayers(g_pStaticEngine);
    }
    
    void RegisterCBaseEntity(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("[ASEntityBindings] Registering CBaseEntity type with asbind20...");
        
        // Register CBaseEntity type with asbind20
        asbind20::ref_class<CBaseEntity>(pEngine, "CBaseEntity")
            // Reference counting behaviors (required for reference types)
            .addref(&AddRef_CBaseEntity)
            .release(&Release_CBaseEntity)
            // Helper methods using lambdas - return type must match AngelScript registration
            .method("Vector3 GetOrigin()", [](CBaseEntity* entity) { return GetEntityOrigin(entity); })
            .method("string GetClassName()", [](CBaseEntity* entity) { return GetEntityClassName(entity); })
            .method("void SetOrigin(const Vector3 &in)", [](CBaseEntity* entity, const Vector& origin) { SetEntityOrigin(entity, origin); })
            .method("float GetHealth()", [](CBaseEntity* entity) { return GetEntityHealth(entity); })
            .method("void SetHealth(float)", [](CBaseEntity* entity, float health) { SetEntityHealth(entity, health); })
            // Direct CBaseEntity methods
            .method("bool IsAlive()", &CBaseEntity::IsAlive)
            .method("string DisplayName()", &CBaseEntity::DisplayName)
            .method("Vector3 Center()", &CBaseEntity::Center)
            .method("float Volume()", &CBaseEntity::Volume)
            .method("float Weight()", &CBaseEntity::Weight)
            .method("bool IsPlayer()", &CBaseEntity::IsPlayer);
        
        MS_ANGEL_INFO("CBaseEntity registration complete");
    }

    void RegisterCBasePlayer(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("[ASEntityBindings] Registering comprehensive CBasePlayer type with asbind20...");
        
        // Comprehensive CBasePlayer registration using enhanced asbind20 patterns
        asbind20::ref_class<CBasePlayer>(pEngine, "CBasePlayer")
            // Reference counting behaviors (required for reference types)
            .addref(&AddRef_CBasePlayer)
            .release(&Release_CBasePlayer)
            
            // Essential identification methods with automatic calling convention detection
            .method("string DisplayName() const", &CBasePlayer::DisplayName)
            .method("string GetName() const", &CBasePlayer::DisplayName)  // Alias for GameMaster script compatibility
            .method("string GETPLAYERAUTHID() const", [](CBasePlayer* player) { 
                return player ? player->AuthID().c_str() : std::string("STEAM_ID_INVALID"); 
            })
            .method("int GetEntIndex() const", [](CBasePlayer* player) { 
                return player ? player->entindex() : 0; 
            })
            
            // State checking with automatic type conversion
            .method("bool IsConnected() const", [](CBasePlayer* player) {
                if (!player || !player->pev) return false;
                return !(player->pev->flags & FL_SPECTATOR) && player->IsAlive();
            })
            .method("bool IsAlive() const", &CBasePlayer::IsAlive)
            .method("bool IsAdmin() const", [](CBasePlayer* player) {
                if (!player) return false;
                return ASEngineProvider::IsPlayerAdmin((void*)player);
            })
            
            // Positional and game state methods
            .method("Vector3 GetOrigin() const", [](CBasePlayer* player) { 
                return GetEntityOrigin(static_cast<CBaseEntity*>(player)); 
            })
            .method("float GetHealth() const", [](CBasePlayer* player) { 
                return GetEntityHealth(static_cast<CBaseEntity*>(player)); 
            })
            
            // Master Sword specific methods for command system
            .method("string GetTitle() const", [](CBasePlayer* player) { 
                if (!player) return std::string("Unknown");
                return ASEngineProvider::GetPlayerTitle((void*)player);
            })
            .method("float MaxHP() const", [](CBasePlayer* player) { 
                if (!player) return 0.0f;
                return ASEngineProvider::GetPlayerMaxHP((void*)player);
            })
            .method("float MaxMP() const", [](CBasePlayer* player) { 
                if (!player) return 0.0f;
                return ASEngineProvider::GetPlayerMaxMP((void*)player);
            })
            .method("bool IsElite() const", [](CBasePlayer* player) { 
                if (!player) return false;
                return ASEngineProvider::IsPlayerElite((void*)player);
            })
            .method("string GetPartyName() const", [](CBasePlayer* player) { 
                if (!player) return std::string("");
                return ASEngineProvider::GetPlayerPartyName((void*)player);
            })
            .method("bool IsLocalHost() const", [](CBasePlayer* player) { 
                if (!player) return false;
                return ASEngineProvider::IsPlayerLocalHost((void*)player);
            })

            .method("string GetSteamID() const", [](CBasePlayer* player) { 
                if (!player) return std::string("");
                return ASEngineProvider::GetPlayerAuthId((void*)player);
            })
            
            // CBaseEntity inherited methods (using lambdas for wrapper functions)
            .method("string GetClassName() const", [](CBasePlayer* player) { 
                return GetEntityClassName(static_cast<CBaseEntity*>(player)); 
            })
            .method("void SetOrigin(const Vector3 &in)", [](CBasePlayer* player, const Vector& origin) { 
                SetEntityOrigin(static_cast<CBaseEntity*>(player), origin); 
            })
            .method("void SetHealth(float)", [](CBasePlayer* player, float health) { 
                SetEntityHealth(static_cast<CBaseEntity*>(player), health); 
            })
            .method("Vector3 Center() const", &CBasePlayer::Center)
            .method("float Volume() const", &CBasePlayer::Volume)
            .method("float Weight() const", &CBasePlayer::Weight)
            .method("bool IsPlayer() const", &CBasePlayer::IsPlayer)
            
            // Additional useful methods for command processing
            .method("void PlaySound(const string &in)", [](CBasePlayer* player, const std::string& sound) {
                if (player) player->PlaySound(CHAN_AUTO, sound.c_str(), 1.0f);
            })
            .method("void SendInfoMsg(const string &in)", [](CBasePlayer* player, const std::string& msg) {
                if (player) player->SendInfoMsg("%s", msg.c_str());
            })
            .method("void SendEventMsg(const string &in)", [](CBasePlayer* player, const std::string& msg) {
                if (player) player->SendEventMsg(msg.c_str());
            })
            
            // Custom equality comparison using pointer comparison (most appropriate for commands)
            .method("bool opEquals(const CBasePlayer@+ other) const", [](CBasePlayer* player, CBasePlayer* other) {
                return player == other;  // Simple pointer comparison
            })
            ;
        
        MS_ANGEL_INFO("Comprehensive CBasePlayer registration complete with enhanced asbind20 patterns");
    }

    void RegisterEntityTypes(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        // Store engine for wrapper functions
        g_pStaticEngine = pEngine;
        
        MS_ANGEL_INFO("[ASEntityBindings] Registering real entity types...");
        
        // Register CBaseEntity with direct AngelScript API
        RegisterCBaseEntity(pEngine);
        
        // Register CBasePlayer with inheritance support
        RegisterCBasePlayer(pEngine);
        
        // Register casting functions with asbind20
        asbind20::global(pEngine)
            .function("CBaseEntity@ ToEntity(CBasePlayer@)", PlayerToEntity_Cast)
            .function("CBasePlayer@ ToPlayer(CBaseEntity@)", EntityToPlayer_Cast);
        
        MS_ANGEL_INFO("[ASEntityBindings] Entity types registration complete");
    }
    
    void RegisterGlobalFunctions(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("[ASEntityBindings] Registering global entity functions with asbind20...");
        
        // Register all global functions with asbind20
        asbind20::global(pEngine)
            // Core utility functions
            .function("string GetTimestamp()", AS_GetTimestamp)
            // Chat and communication
            .function("void ChatLog(const string &in)", [](const std::string& message) { AS_ChatLog(message); })
            // Player management
            .function("string GetPlayerCurrentMap()", AS_GetPlayerCurrentMap)
            .function("int GetCurrentPlayerID()", AS_GetCurrentPlayerID)
            .function("void SendPlayerMessage(const string &in, const string &in, const string &in)", [](const std::string& playerName, const std::string& title, const std::string& message) {
                AS_SendPlayerMessage(playerName, title, message);
            })
            // Real player management functions that work with actual CBasePlayer objects
            .function("array<CBasePlayer@>@ GetAllPlayers()", AS_GetAllPlayersWrapper)
            .function("int GetPlayerCount()", AS_GetPlayerCount)
            .function("CBasePlayer@ PlayerByIndex(int)", AS_PlayerByIndex)
            .function("bool IsConnected(CBasePlayer@)", AS_IsConnected)
            .function("string GetDisplayName(CBasePlayer@)", AS_GetDisplayName)
            .function("string GetSteamID(CBasePlayer@)", AS_GetSteamID)
            .function("string GetPlayerSteamID(CBasePlayer@)", AS_GetSteamID)  // Alias for VotingCommands.as compatibility
            .function("bool IsAdmin(CBasePlayer@)", AS_IsAdmin)
            .function("string GetClientAddress(CBasePlayer@)", AS_GetClientAddress)
            // Quest data functions
            .function("string GetPlayerQuestData(const string &in, const string &in)", [](const std::string& playerID, const std::string& key) {
                return AS_GetPlayerQuestData(playerID, key);
            })
            .function("void SetPlayerQuestData(const string &in, const string &in, const string &in)", [](const std::string& playerID, const std::string& key, const std::string& value) {
                AS_SetPlayerQuestData(playerID, key, value);
            })
            // Logging functions for scripts
            .function("void MS_ANGEL_INFO(const string &in)", [](const std::string& message) { AS_LogAngelInfo(message); })
            .function("void MS_ANGEL_DEBUG(const string &in)", [](const std::string& message) { AS_LogAngelDebug(message); })
            .function("void MS_ANGEL_ERROR(const string &in)", [](const std::string& message) { AS_LogAngelError(message); });
        
        MS_ANGEL_INFO("[ASEntityBindings] Global entity functions registered successfully");
    }
    
    // Test function to validate entity bindings are working
    void AS_TestEntityBindings()
    {
        MS_ANGEL_INFO("[ASEntityBindings] Testing entity bindings...");
        
#ifndef CLIENT_DLL
        // Test player access
        CBasePlayer* pTestPlayer = AS_PlayerByIndex(1);
        if (pTestPlayer)
        {
            std::string playerName = GetPlayerDisplayName(pTestPlayer);
            float maxHP = GetPlayerMaxHP(pTestPlayer);
            float maxMP = GetPlayerMaxMP(pTestPlayer);
            std::string title = GetPlayerTitle(pTestPlayer);
            
            MS_ANGEL_INFO("Test Player Found: %s, MaxHP: %.1f, MaxMP: %.1f, Title: %s", 
                         playerName.c_str(), maxHP, maxMP, title.c_str());
        }
        else
        {
            MS_ANGEL_INFO("No test player found for entity binding validation");
        }
        
        // Test GetAllPlayers
        if (g_pStaticEngine)
        {
            CScriptArray* playerArray = AS_GetAllPlayers(g_pStaticEngine);
            if (playerArray)
            {
                MS_ANGEL_INFO("GetAllPlayers returned %d players", playerArray->GetSize());
                playerArray->Release();
            }
        }
#endif
        
        MS_ANGEL_INFO("[ASEntityBindings] Entity binding test complete");
    }
    
    void RegisterAll(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("[ASEntityBindings] Starting entity bindings registration...");
        
        // Note: Core types (Vector3, EntityHandle, etc.) are registered by ASBindings.cpp
        // We only register entity-specific bindings here
        
        // Register the new template-based engine bindings
        ASEngineBindings::RegisterAll(pEngine);
        
        // Register entity types and legacy functions
        RegisterEntityTypes(pEngine);
        RegisterGlobalFunctions(pEngine);
        
        // Test the entity bindings to ensure they work
        AS_TestEntityBindings();
        
        MS_ANGEL_INFO("[ASEntityBindings] Entity bindings registration complete");
    }
}