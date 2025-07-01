//==========================================================================
// ASEntityBindings.cpp - Using asbind20
// 
// Entity-related type bindings for AngelScript integration
// Provides CBaseEntity, CBasePlayer, and related functions for script usage
//==========================================================================

#include "CAngelScript.h"
#include <asbind20/asbind.hpp>
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

// Include new template-based engine interface
#include "ASEngineInterface.h"
#include "ASEngineBindings.h"

// Dummy type definitions for asbind20 registration
// These are only used for type registration and not actual implementation
namespace {
    struct CBaseEntity_Dummy { 
        void* _dummy; 
    };
    struct CBasePlayer_Dummy { 
        void* _dummy; 
    };
}

// Use the dummy types for registration
#define CBaseEntity CBaseEntity_Dummy
#define CBasePlayer CBasePlayer_Dummy

// Note: All external C function declarations have been removed.
// All engine integration now uses ASEngineProvider directly.

// Remove forward declarations - using dummy types via macros

namespace ASEntityBindings
{
    // Forward declarations for casting functions
    CBaseEntity* PlayerToEntity_Cast(CBasePlayer* pPlayer);
    CBasePlayer* EntityToPlayer_Cast(CBaseEntity* pEntity);
    template<class T> T* Template_Cast(void* pEntity);
    
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
        
        Vector origin = ASEngineProvider::GetEntityOrigin((void*)pEntity);
        MS_ANGEL_DEBUG("Entity_GetOrigin: (%f, %f, %f)", origin.x, origin.y, origin.z);
        return origin;
    }
    
    std::string Entity_GetClassName(CBaseEntity* pEntity)
    {
        if (!pEntity)
        {
            MS_ANGEL_ERROR("Entity_GetClassName: NULL entity pointer");
            return "null_entity";
        }
        
        std::string result = ASEngineProvider::GetEntityClassName((void*)pEntity);
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
        
        ASEngineProvider::SetEntityOrigin((void*)pEntity, origin);
        MS_ANGEL_DEBUG("Entity_SetOrigin: Set to (%f, %f, %f)", origin.x, origin.y, origin.z);
    }
    
    bool Entity_IsAlive(CBaseEntity* pEntity)
    {
        if (!pEntity)
        {
            MS_ANGEL_DEBUG("Entity_IsAlive: NULL entity pointer - considered dead");
            return false;
        }
        
        float health = ASEngineProvider::GetEntityHealth((void*)pEntity);
        int deadFlag = ASEngineProvider::GetEntityDeadFlag((void*)pEntity);
        
        // Check both health and deadflag (DEAD_NO = 0)
        bool isAlive = (deadFlag == 0) && (health > 0);
        MS_ANGEL_DEBUG("Entity_IsAlive: %s (health: %f, deadflag: %d)", 
                      isAlive ? "alive" : "dead", health, deadFlag);
        return isAlive;
    }
    
    // CBasePlayer methods (using dummy type)
    std::string Player_GetName(CBasePlayer* pPlayer)
    {
        if (!pPlayer)
        {
            MS_ANGEL_ERROR("Player_GetName: NULL player pointer");
            return "Unknown Player";
        }
        
        std::string result = ASEngineProvider::GetPlayerDisplayName((void*)pPlayer);
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
        
        std::string result = ASEngineProvider::GetPlayerAuthId((void*)pPlayer);
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
        
        std::string result = ASEngineProvider::GetPlayerClientAddress((void*)pPlayer);
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
        
        ASEngineProvider::SendInfoMsg((void*)pPlayer, message);
        std::string playerName = ASEngineProvider::GetPlayerDisplayName((void*)pPlayer);
        MS_ANGEL_DEBUG("Player_SendMessage sent to %s: %s", playerName.c_str(), message.c_str());
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
        
        // Emit sound at player's location (CHAN_AUTO=0, ATTN_NORM=0.8, PITCH_NORM=100)
        ASEngineProvider::EmitSound((void*)pPlayer, 0, sound, clampedVolume, 0.8f, 0, 100);
        std::string playerName = ASEngineProvider::GetPlayerDisplayName((void*)pPlayer);
        MS_ANGEL_DEBUG("Player_EmitSound: %s at volume %f for %s", sound.c_str(), clampedVolume, playerName.c_str());
    }
    
    // Global functions
    float AS_GetGameTime()
    {
        float gameTime = ASEngineProvider::GetGameTime();
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
    
    // Create entity by script name
    void* AS_CreateEntity(const std::string& scriptName)
    {
        if (scriptName.empty())
        {
            MS_ANGEL_ERROR("CreateEntity: Empty script name");
            return nullptr;
        }
        
        void* entity = ASEngineProvider::CreateEntity(scriptName);
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
        
        ASEngineProvider::SetEntityName((void*)pEntity, name);
        MS_ANGEL_DEBUG("SetEntityName: Set name to '%s'", name.c_str());
    }
    
    void AS_SetEntityTargetName(CBaseEntity* pEntity, const std::string& targetName)
    {
        if (!pEntity)
        {
            MS_ANGEL_ERROR("SetEntityTargetName: NULL entity pointer");
            return;
        }
        
        ASEngineProvider::SetEntityTargetName((void*)pEntity, targetName);
        MS_ANGEL_DEBUG("SetEntityTargetName: Set targetname to '%s'", targetName.c_str());
    }
    
    void AS_SetEntityHealth(CBaseEntity* pEntity, float health)
    {
        if (!pEntity)
        {
            MS_ANGEL_ERROR("SetEntityHealth: NULL entity pointer");
            return;
        }
        
        ASEngineProvider::SetEntityHealth((void*)pEntity, health);
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
        
        float health = ASEngineProvider::GetEntityHealth((void*)pEntity);
        int deadFlag = ASEngineProvider::GetEntityDeadFlag((void*)pEntity);
        
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
        MS_ANGEL_DEBUG("PlayerToEntity_Cast: Successfully cast player to entity");
        return reinterpret_cast<CBaseEntity*>(pPlayer);
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
        
        MS_ANGEL_DEBUG("EntityToPlayer_Cast: Successfully cast entity to player");
        return reinterpret_cast<CBasePlayer*>(pEntity);
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
    
    void RegisterEntityInheritance(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("[ASEntityBindings] Setting up entity inheritance...");
        
        // Since this AngelScript version doesn't have implicit/explicit casting behaviors,
        // we'll use global functions for casting operations
        
        // Register casting functions with asbind20
        asbind20::global(pEngine)
            // Direct casting functions (simpler approach)
            .function("CBasePlayer@ ToPlayer(CBaseEntity@)", &EntityToPlayer_Cast)
            .function("CBaseEntity@ ToEntity(CBasePlayer@)", &PlayerToEntity_Cast);
        
        MS_ANGEL_INFO("[ASEntityBindings] Entity inheritance setup complete");
        MS_ANGEL_INFO("   Note: Use ToPlayer(entity) or ToEntity(player) for casting");
    }
    
    void RegisterEntityTypes(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("[ASEntityBindings] Registering entity types...");
        
        // Register CBaseEntity as a reference type with asbind20
        asbind20::ref_class<CBaseEntity>(pEngine, "CBaseEntity", asOBJ_NOCOUNT)
            .method("Vector3 GetOrigin() const", 
                +[](CBaseEntity* self) -> Vector { return Entity_GetOrigin(self); })
            .method("string GetClassName() const", 
                +[](CBaseEntity* self) -> std::string { return Entity_GetClassName(self); })
            .method("void SetOrigin(const Vector3 &in)", 
                +[](CBaseEntity* self, const Vector& origin) { Entity_SetOrigin(origin, self); })
            .method("bool IsAlive() const", 
                +[](CBaseEntity* self) -> bool { return Entity_IsAlive(self); });
        
        MS_ANGEL_DEBUG("   ✓ CBaseEntity type registered");
        
        // Register CBasePlayer as a reference type with asbind20
        asbind20::ref_class<CBasePlayer>(pEngine, "CBasePlayer", asOBJ_NOCOUNT)
            // Since AngelScript inheritance isn't working as expected in this version,
            // we need to manually register CBaseEntity methods on CBasePlayer as well
            .method("Vector3 GetOrigin() const", 
                +[](CBasePlayer* self) -> Vector { return Entity_GetOrigin(reinterpret_cast<CBaseEntity*>(self)); })
            .method("string GetClassName() const", 
                +[](CBasePlayer* self) -> std::string { return Entity_GetClassName(reinterpret_cast<CBaseEntity*>(self)); })
            .method("void SetOrigin(const Vector3 &in)", 
                +[](CBasePlayer* self, const Vector& origin) { Entity_SetOrigin(origin, reinterpret_cast<CBaseEntity*>(self)); })
            .method("bool IsAlive() const", 
                +[](CBasePlayer* self) -> bool { return Entity_IsAlive(reinterpret_cast<CBaseEntity*>(self)); })
            // CBasePlayer-specific methods
            .method("string GetName() const", 
                +[](CBasePlayer* self) -> std::string { return Player_GetName(self); })
            .method("string GetSteamID() const", 
                +[](CBasePlayer* self) -> std::string { return Player_GetSteamID(self); })
            .method("string GetIPAddress() const", 
                +[](CBasePlayer* self) -> std::string { return Player_GetIPAddress(self); })
            .method("void SendMessage(const string &in, bool = true)", 
                +[](CBasePlayer* self, const std::string& msg, bool center) { Player_SendMessage(msg, center, self); })
            .method("void EmitSound(const string &in, float = 1.0f)", 
                +[](CBasePlayer* self, const std::string& sound, float volume) { Player_EmitSound(sound, volume, self); });
        
        MS_ANGEL_DEBUG("   ✓ CBasePlayer type registered");
        
        // Set up inheritance relationships after both types are registered
        RegisterEntityInheritance(pEngine);
        
        MS_ANGEL_INFO("[ASEntityBindings] Entity types registered successfully");
    }
    
    void RegisterGlobalFunctions(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("[ASEntityBindings] Registering global entity functions...");
        
        // Register global functions with asbind20
        asbind20::global(pEngine)
            // Core game functions
            .function("float GetGameTime()", &AS_GetGameTime)
            .function("string GetTimestamp()", &AS_GetTimestamp)
            .function("string GetCvar(const string &in)", &AS_GetCvar)
            .function("string GetMapName()", &AS_GetMapName)
            // Entity management
            .function("EntityHandle CreateEntity(const string &in)", &AS_CreateEntity)
            .function("void SetEntityName(EntityHandle, const string &in)", &AS_SetEntityName)
            .function("void SetEntityTargetName(EntityHandle, const string &in)", &AS_SetEntityTargetName)
            .function("void SetEntityHealth(EntityHandle, float)", &AS_SetEntityHealth)
            .function("bool IsEntityDead(EntityHandle)", &AS_IsEntityDead)
            // Chat and communication
            .function("void ChatLog(const string &in)", &AS_ChatLog)
            // Player management
            .function("string GetPlayerCurrentMap()", &AS_GetPlayerCurrentMap)
            .function("int GetCurrentPlayerID()", &AS_GetCurrentPlayerID)
            .function("void SendPlayerMessage(const string &in, const string &in, const string &in)", &AS_SendPlayerMessage)
            // Logging functions
            .function("void MS_ANGEL_INFO(const string &in)", &AS_LogAngelInfo)
            .function("void MS_ANGEL_DEBUG(const string &in)", &AS_LogAngelDebug)
            .function("void MS_ANGEL_ERROR(const string &in)", &AS_LogAngelError);
        
        MS_ANGEL_INFO("[ASEntityBindings] Global entity functions registered successfully");
    }
    
    void RegisterAll(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        MS_ANGEL_INFO("[ASEntityBindings] Starting entity bindings registration...");
        
        // Register the new template-based engine bindings first
        ASEngineBindings::RegisterAll(pEngine);
        
        // Register entity types and legacy functions
        RegisterEntityTypes(pEngine);
        RegisterGlobalFunctions(pEngine);
        
        MS_ANGEL_INFO("[ASEntityBindings] Entity bindings registration complete");
    }
}