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
#include <asbind20/asbind.hpp>

// Include vector.h directly for Vector type
typedef float vec_t;
#include "hl/vector.h"

// Include MSLogger with proper path
#include "mslogger.h"

// Include real Master Sword entity classes first
#ifdef CLIENT_DLL
    // Client-side includes
    #include "hud.h"
    #include "cl_util.h"
#else
    // Server-side includes (VALVE_DLL is defined for server builds)
    #include "extdll.h"
    #include "util.h"
    #include "hl/cbase.h"
    #include "player/player.h"
    #include "svglobals.h"  // For g_pGameMasterEntity
    #include "monsters/msmonster.h"
    #include "weapons/genericitem.h"
    #include "weapons/weapons.h"  // For CBasePlayerItem and CBasePlayerWeapon
#endif

#ifdef VALVE_DLL
    #include "enginecallback.h"  // For WRITE_STRING_MAX and network message functions
#endif

// Include AngelScript headers after engine headers
// Using asbind20 for clean, type-safe binding syntax
#include "ASEngineInterface.h"
#include "ASEngineBindings.h"
#include "ASCoreTypes.h"
#include "ASMonsterBindings.h"

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
    
    // ========================================================================
    // Message Color Enum for AngelScript
    // Maps to hudevent_e from player.h for colored HUD messages
    // ========================================================================
    enum class MessageColor : int
    {
        White = 0,      // HUDEVENT_NORMAL - Normal color (off white)
        Gray = 1,       // HUDEVENT_UNABLE - Unable to do something (dark gray)
        Yellow = 2,     // HUDEVENT_ATTACK - Your attack results (yellowish)
        Red = 3,        // HUDEVENT_ATTACKED - You were attacked (red)
        Green = 4,      // HUDEVENT_GREEN - Something good (green)
        Blue = 5        // HUDEVENT_BLUE - Something blue (blue)
    };
    
    // ========================================================================
    // Player Messaging Wrapper Functions
    // Replacements for CScript::ScriptCmd_Message and ScriptCmd_InfoMessage
    // ========================================================================
    
    /**
     * SendColoredMessage - Send a colored event message to player's HUD
     * Replacement for: playermessage, rplayermessage, gplayermessage, etc.
     * 
     * @param player The player to send the message to
     * @param color The color/type of message (MessageColor enum)
     * @param message The message text to display
     */
    void SendColoredMessage(CBasePlayer* player, MessageColor color, const std::string& message)
    {
        #ifdef VALVE_DLL
            if (!player || message.empty()) return;
            
            // Apply message length limit (140 chars as per original implementation)
            std::string finalMsg = message;
            if (finalMsg.length() > 140) {
                finalMsg = finalMsg.substr(0, 140) + "*";
            }
            
            // Add newline if not already present
            if (!finalMsg.empty() && finalMsg[finalMsg.length() - 1] != '\n') {
                finalMsg += "\n";
            }
            
            // Send message based on color using hudevent_e values
            switch (color) {
                case MessageColor::White:
                    // Use SendInfoMsg for white/normal messages (matches "playermessage")
                    player->SendInfoMsg("%s", finalMsg.c_str());
                    break;
                    
                case MessageColor::Gray:
                    player->SendEventMsg(HUDEVENT_UNABLE, finalMsg.c_str());
                    break;
                    
                case MessageColor::Yellow:
                    player->SendEventMsg(HUDEVENT_ATTACK, finalMsg.c_str());
                    break;
                    
                case MessageColor::Red:
                    player->SendEventMsg(HUDEVENT_ATTACKED, finalMsg.c_str());
                    break;
                    
                case MessageColor::Green:
                    player->SendEventMsg(HUDEVENT_GREEN, finalMsg.c_str());
                    break;
                    
                case MessageColor::Blue:
                    player->SendEventMsg(HUDEVENT_BLUE, finalMsg.c_str());
                    break;
                    
                default:
                    // Default to normal white message
                    player->SendInfoMsg("%s", finalMsg.c_str());
                    break;
            }
            
            MS_ANGEL_DEBUG("SendColoredMessage: Sent %s message to %s: %s", 
                static_cast<int>(color) == 0 ? "white" :
                static_cast<int>(color) == 1 ? "gray" :
                static_cast<int>(color) == 2 ? "yellow" :
                static_cast<int>(color) == 3 ? "red" :
                static_cast<int>(color) == 4 ? "green" : "blue",
                player->DisplayName(),
                finalMsg.c_str());
        #endif
    }
    
    /**
     * SendHUDInfoMessage - Send an info box message to player's HUD
     * Replacement for: infomsg <player> <title> <message>
     * 
     * @param player The player to send the message to
     * @param title The title text for the info box (max 120 chars)
     * @param message The body text for the info box (max 120 chars)
     */
    void SendHUDInfoMessage(CBasePlayer* player, const std::string& title, const std::string& message)
    {
        #ifdef VALVE_DLL
            if (!player) return;
            
            // Apply length limits (120 chars as per original implementation)
            std::string finalTitle = title;
            if (finalTitle.length() > 120) {
                finalTitle = finalTitle.substr(0, 120) + "*\n";
            }
            
            std::string finalMsg = message;
            if (finalMsg.length() > 120) {
                finalMsg = finalMsg.substr(0, 120) + "*\n";
            }
            
            // Send HUD info message
            player->SendHUDMsg(finalTitle.c_str(), finalMsg.c_str());
            
            MS_ANGEL_DEBUG("SendHUDInfoMessage: Sent to %s - Title: %s, Message: %s", 
                player->DisplayName(),
                finalTitle.c_str(),
                finalMsg.c_str());
        #endif
    }
    
    // Helper functions for CBaseEntity methods that need extra logic
    Vector GetEntityOrigin(CBaseEntity* pEntity)
    {
        if (!pEntity || FNullEnt(pEntity))
            return Vector(0, 0, 0);
        return pEntity->pev->origin;
    }
    
    std::string GetEntityClassName(CBaseEntity* pEntity)
    {
        if (!pEntity || FNullEnt(pEntity))
            return "null_entity";
        return STRING(pEntity->pev->classname);
    }
    
    void SetEntityOrigin(CBaseEntity* pEntity, const Vector& origin)
    {
        if (!pEntity || FNullEnt(pEntity))
            return;
        UTIL_SetOrigin(pEntity->pev, origin);
    }
    
    float GetEntityHealth(CBaseEntity* pEntity)
    {
        if (!pEntity || FNullEnt(pEntity))
            return 0.0f;
        return pEntity->pev->health;
    }
    
    void SetEntityHealth(CBaseEntity* pEntity, float health)
    {
        if (!pEntity || FNullEnt(pEntity))
            return;
        pEntity->pev->health = health;
    }
    
    // Helper functions for CBasePlayer methods
    std::string GetPlayerDisplayName(CBasePlayer* pPlayer)
    {
        if (!pPlayer || FNullEnt(pPlayer))
            return "Unknown Player";
        return STRING(pPlayer->pev->netname);
    }
    
    std::string GetPlayerAuthID(CBasePlayer* pPlayer)
    {
        if (!pPlayer || FNullEnt(pPlayer->edict()))
            return "STEAM_ID_INVALID";
            
        return GETPLAYERAUTHID(pPlayer->edict());
    }
    
    std::string GetPlayerTitle(CBasePlayer* pPlayer)
    {
        if (!pPlayer || FNullEnt(pPlayer))
            return "Unknown";
        return pPlayer->GetTitle();
    }
    
    float GetPlayerMaxHP(CBasePlayer* pPlayer)
    {
        if (!pPlayer || FNullEnt(pPlayer))
            return 0.0f;
        return pPlayer->MaxHP();
    }
    
    float GetPlayerMaxMP(CBasePlayer* pPlayer)
    {
        if (!pPlayer || FNullEnt(pPlayer))
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
        
#ifdef VALVE_DLL
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
#ifdef VALVE_DLL
        
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
    
    CBasePlayer* AS_PlayerBySteamID(const std::string& steamID)
    {
#ifdef VALVE_DLL
        if (steamID.empty())
        {
            MS_ANGEL_ERROR("PlayerBySteamID: Empty Steam ID");
            return nullptr;
        }
        
        for (int i = 1; i <= gpGlobals->maxClients; i++)
        {
            CBasePlayer* pPlayer = AS_PlayerByIndex(i);
            if (pPlayer)
            {
                std::string playerSteamID = pPlayer->AuthID().c_str();
                if (playerSteamID == steamID)
                {
                    MS_ANGEL_DEBUG("PlayerBySteamID: Found player with Steam ID %s", steamID.c_str());
                    return pPlayer;
                }
            }
        }
        
        MS_ANGEL_DEBUG("PlayerBySteamID: No player found with Steam ID %s", steamID.c_str());
        return nullptr;
#else
        // Client-side: Can't access other players
        MS_ANGEL_DEBUG("PlayerBySteamID: Client-side, returning nullptr");
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
        
        std::string steamID = GETPLAYERAUTHID(pPlayer->edict());
        //std::string steamID = GetPlayerAuthID(pPlayer);
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
    
    // Enum for NPC spawn modes
    enum class ScriptMode
    {
        Legacy = 0,      // Load and execute MSCScript file (legacy behavior)
        Angel = 1,    // Skip MSCScript loading (for AngelScript-managed entities)
        Both = 2    // Load and execute both MSCScript and AngelScript
    };
    
    // SpawnNPC - Creates an NPC at a specific position
    // Returns CMSMonster@ pointer to the created monster
    // spawnMode: Legacy (default) loads legacy MSCScript, Angel skips it
    CMSMonster* AS_SpawnNPC(const std::string& scriptName, const Vector& position, CScriptArray* params, ScriptMode spawnMode = ScriptMode::Legacy)
    {
#ifdef VALVE_DLL
        if (scriptName.empty())
        {
            MS_ANGEL_ERROR("SpawnNPC: Empty script name");
            return nullptr;
        }
        
        const char* modeStr = spawnMode == ScriptMode::Legacy ? "Legacy" : 
                              (spawnMode == ScriptMode::Angel ? "Angel" : "Both");
        MS_ANGEL_INFO("SpawnNPC: Creating NPC '%s' at (%.1f, %.1f, %.1f) [Mode: %s]", 
                      scriptName.c_str(), position.x, position.y, position.z, modeStr);
        
        // Create the monster entity using engine function
        CMSMonster* pMonster = (CMSMonster*)GET_PRIVATE(CREATE_NAMED_ENTITY(MAKE_STRING("ms_npc")));
        if (!pMonster || FNullEnt(pMonster))
        {
            MS_ANGEL_ERROR("SpawnNPC: Failed to create ms_npc entity");
            return nullptr;
        }
        
        // Set origin before spawning
        pMonster->pev->origin = position;
        
        if (spawnMode == ScriptMode::Legacy)
        {
            // Legacy mode: Spawn the monster with the MSCScript
            pMonster->Spawn(scriptName.c_str());
        }
        else if (spawnMode == ScriptMode::Angel)
        {
            // Angel mode: Manually initialize without loading MSCScript
            // This prevents SUB_Remove() from being called when no script file exists
            pMonster->m_ScriptName = ""; // No script - AngelScript manages this entity
            pMonster->m_DisplayName = scriptName.c_str();
            pMonster->pev->classname = MAKE_STRING("ms_npc");
            
            // Set basic properties for a minimal entity
            pMonster->pev->health = 1;
            pMonster->m_HP = 1;
            pMonster->pev->max_health = 1;
            pMonster->m_MaxHP = 1;
            pMonster->pev->takedamage = DAMAGE_NO;
            pMonster->pev->solid = SOLID_NOT;
            pMonster->pev->movetype = MOVETYPE_NONE;
            pMonster->pev->flags |= FL_MONSTER;
            pMonster->pev->deadflag = DEAD_NO;
            pMonster->pev->gravity = 0;
            
            // Make invisible
            pMonster->pev->effects |= EF_NODRAW;
            pMonster->pev->rendermode = kRenderTransTexture;
            pMonster->pev->renderamt = 0;
            
            // Set size (minimal)
            UTIL_SetSize(pMonster->pev, Vector(-16, -16, 0), Vector(16, 16, 32));
            
            // Call Precache to set up basic properties
            pMonster->Precache();
        }
        else // ScriptMode::Both
        {
            // Both mode: Initialize manually first (to ensure entity persists),
            // then attempt to load MSCScript if it exists
            MS_ANGEL_INFO("SpawnNPC: Both mode - initializing entity manually first");
            
            pMonster->m_ScriptName = scriptName.c_str();
            pMonster->m_DisplayName = scriptName.c_str();
            pMonster->pev->classname = MAKE_STRING("ms_npc");
            
            // Set basic properties for a minimal entity
            pMonster->pev->health = 1;
            pMonster->m_HP = 1;
            pMonster->pev->max_health = 1;
            pMonster->m_MaxHP = 1;
            pMonster->pev->takedamage = DAMAGE_NO;
            pMonster->pev->solid = SOLID_NOT;
            pMonster->pev->movetype = MOVETYPE_NONE;
            pMonster->pev->flags |= FL_MONSTER;
            pMonster->pev->deadflag = DEAD_NO;
            pMonster->pev->gravity = 0;
            
            // Make invisible by default
            pMonster->pev->effects |= EF_NODRAW;
            pMonster->pev->rendermode = kRenderTransTexture;
            pMonster->pev->renderamt = 0;
            
            // Set size (minimal)
            UTIL_SetSize(pMonster->pev, Vector(-16, -16, 0), Vector(16, 16, 32));
            
            // Call Precache to set up basic properties
            pMonster->Precache();
            
            // Now try to load the MSCScript if it exists
            // Script_Add returns nullptr if script doesn't exist or fails to load
            IScripted* pScripted = pMonster->GetScripted();
            if (pScripted)
            {
                CScript* pScript = pScripted->Script_Add(scriptName.c_str(), pMonster);
                if (pScript)
                {
                    MS_ANGEL_INFO("SpawnNPC: Both mode - MSCScript '%s' loaded successfully", scriptName.c_str());
                    // Run script events to initialize script-side properties
                    pScripted->RunScriptEvents();
                }
                else
                {
                    MS_ANGEL_INFO("SpawnNPC: Both mode - MSCScript '%s' not found or failed to load (continuing with AngelScript only)", scriptName.c_str());
                }
            }
        }
        
        MS_ANGEL_INFO("SpawnNPC: Successfully spawned '%s' at index %d", 
                      scriptName.c_str(), pMonster->entindex());
        
        return pMonster;
#else
        MS_ANGEL_ERROR("SpawnNPC: Client cannot spawn NPCs");
        return nullptr;
#endif
    }
    
    // SpawnItem - Creates an item at a specific position
    // Returns CBasePlayerItem@ pointer to the created item
    CBasePlayerItem* AS_SpawnItem(const std::string& scriptName, const Vector& position, CScriptArray* params)
    {
#ifdef VALVE_DLL
        if (scriptName.empty())
        {
            MS_ANGEL_ERROR("SpawnItem: Empty script name");
            return nullptr;
        }
        
        MS_ANGEL_INFO("SpawnItem: Creating item '%s' at (%.1f, %.1f, %.1f)", 
                      scriptName.c_str(), position.x, position.y, position.z);
        
        // Create the item using the item manager
        CGenericItem* pItem = NewGenericItem(scriptName.c_str());
        if (!pItem)
        {
            MS_ANGEL_ERROR("SpawnItem: Failed to create item '%s'", scriptName.c_str());
            return nullptr;
        }
        
        // Set origin
        pItem->pev->origin = position;
        
        MS_ANGEL_INFO("SpawnItem: Successfully spawned '%s' at index %d", 
                      scriptName.c_str(), pItem->entindex());
        
        return pItem;
#else
        MS_ANGEL_ERROR("SpawnItem: Client cannot spawn items");
        return nullptr;
#endif
    }
    
    // Vote menu opening function - opens a menu with custom options using the game master entity
    void AS_OpenVoteMenu(CBasePlayer* pPlayer, const std::string& title, const CScriptArray* options)
    {
#ifdef VALVE_DLL
        MS_ANGEL_INFO("OpenVoteMenu called for player %s with title '%s'", 
                     pPlayer ? pPlayer->DisplayName() : "NULL", title.c_str());
        
        if (!pPlayer || !pPlayer->pev || !options)
        {
            MS_ANGEL_ERROR("OpenVoteMenu: Invalid parameters (player=%p, pev=%p, options=%p)", 
                          pPlayer, pPlayer ? pPlayer->pev : NULL, options);
            return;
        }
        
        // Check inventory full
        if (pPlayer->NumItems() >= NUM_MAX_ITEMS)
        {
            pPlayer->SendEventMsg(HUDEVENT_UNABLE, "Cannot use menus while inventory is full.");
            MS_ANGEL_DEBUG("OpenVoteMenu: Player inventory full");
            return;
        }
        
        // Get the game master entity from global handle (required for menu system)
        MS_ANGEL_DEBUG("OpenVoteMenu: Using global game_master entity handle");
        CBaseEntity* pGameMaster = g_pGameMasterEntity;
        
        // Check if game_master entity is invalid (NULL, freed memory, or null edict)
        if (!pGameMaster || !pGameMaster->pev || ((uintptr_t)pGameMaster->pev == 0xdddddddd) || FNullEnt(pGameMaster->edict()))
        {
            MS_ANGEL_ERROR("OpenVoteMenu: Global game_master entity handle is invalid!");
            MS_ANGEL_ERROR("  pGameMaster=%p, pev=%p", pGameMaster, pGameMaster ? pGameMaster->pev : NULL);
            MS_ANGEL_ERROR("  This likely means ServerActivate hasn't run yet or game_master creation failed.");
            
            // Try to find it by searching as fallback
            MS_ANGEL_DEBUG("  Attempting fallback search for game_master by netname...");
            pGameMaster = UTIL_FindEntityByString(NULL, "netname", "-game_master");
            
            if (pGameMaster)
            {
                // Update the global so we don't need to search again
                g_pGameMasterEntity = pGameMaster;
                MS_ANGEL_INFO("  Fallback search succeeded! Updated global game_master entity handle (index %d)", 
                             pGameMaster->entindex());
            }
            else
            {
                MS_ANGEL_ERROR("  Fallback search also failed - game_master entity doesn't exist!");
                
                // Debug: list all ms_npc entities to see what's available
                CBaseEntity* pTest = NULL;
                int npcCount = 0;
                MS_ANGEL_DEBUG("  Searching for ms_npc entities...");
                while ((pTest = UTIL_FindEntityByClassname(pTest, "ms_npc")) != NULL)
                {
                    npcCount++;
                    if (pTest->pev && pTest->pev->netname)
                    {
                        MS_ANGEL_DEBUG("    Found ms_npc #%d with netname: '%s'", npcCount, STRING(pTest->pev->netname));
                    }
                    else
                    {
                        MS_ANGEL_DEBUG("    Found ms_npc #%d with no netname", npcCount);
                    }
                }
                MS_ANGEL_DEBUG("  Total ms_npc entities found: %d", npcCount);
                
                if (npcCount == 0)
                {
                    pPlayer->SendEventMsg(HUDEVENT_UNABLE, "Vote system initializing - please try again in a moment.");
                    MS_ANGEL_ERROR("  No NPCs found at all - server may still be starting up");
                }
                else
                {
                    pPlayer->SendEventMsg(HUDEVENT_UNABLE, "Vote system error: game master not ready.");
                    MS_ANGEL_ERROR("  Found %d NPCs but none with netname '-game_master'", npcCount);
                }
                return;
            }
        }

        CMSMonster* pGM = pGameMaster->IsMSMonster() ? (CMSMonster*)pGameMaster : NULL;

        if (!pGM || !pGM->entindex())
        {
            MS_ANGEL_ERROR("OpenVoteMenu: game_master is not a CMSMonster or has no entity index!");
            return;
        }

        MS_ANGEL_INFO("OpenVoteMenu: Found game_master entity at index %d", pGameMaster->entindex());
        

        
        int playerIndex = pPlayer->entindex();
        MS_ANGEL_INFO("Opening vote menu for %s (index %d) with title '%s' via game_master entity %d", 
                      pPlayer->DisplayName(), playerIndex, title.c_str(), pGM->entindex());
        
        // Clear and set up menu options for this player
        mslist<menuoption_t>& menuOptions = pGM->m_MenuOptions[playerIndex];
        menuOptions.clearitems();
        
        // Set protection flag to prevent old menu system from clearing these options
        pGM->m_MenuOptionsProtected[playerIndex] = true;
        MS_ANGEL_INFO("*** VOTE MENU PROTECTION: SET for player %d (entity: %d, options: %d) ***", 
                     playerIndex, pGM->entindex(), menuOptions.size());
        
        MS_ANGEL_INFO("Storing menu options at index %d (player: %s)", playerIndex, pPlayer->DisplayName());
        
        // Add each vote option to the game master's menu
        asUINT optionCount = options->GetSize();
        for (asUINT i = 0; i < optionCount; i++)
        {
            const std::string* optionStr = static_cast<const std::string*>(options->At(i));
            if (optionStr)
            {
                menuoption_t menuOption;
                clrmem(menuOption);
                menuOption.Access = MOA_ALL;
                menuOption.Title = optionStr->c_str();
                menuOption.Type = MOT_CALLBACK;
                menuOption.Data = optionStr->c_str();
                menuOption.CB_Name = "game_vote_menu_callback";  // Set callback event name
                menuOptions.add(menuOption);
                MS_ANGEL_INFO("  Added option %d: '%s' (CB: %s)", i, optionStr->c_str(), menuOption.CB_Name.c_str());
            }
        }
        
        MS_ANGEL_INFO("Total options stored for player %d: %d", playerIndex, menuOptions.size());
        
        // Send menu initialization message (message type 25)
        MESSAGE_BEGIN(MSG_ONE, g_netmsg[NETMSG_CLDLLFUNC], NULL, pPlayer->pev);
        WRITE_BYTE(25);  // Menu init
        WRITE_LONG(pGM->entindex());  // Game master entity index
        WRITE_STRING_LIMIT(title.c_str(), WRITE_STRING_MAX);
        MESSAGE_END();
        
        // Send each menu option (message type 26)
        for (asUINT i = 0; i < optionCount; i++)
        {
            const std::string* optionStr = static_cast<const std::string*>(options->At(i));
            if (optionStr)
            {
                MESSAGE_BEGIN(MSG_ONE, g_netmsg[NETMSG_CLDLLFUNC], NULL, pPlayer->pev);
                WRITE_BYTE(26);  // Menu option
                WRITE_BYTE(MOA_ALL);   // Access level
                WRITE_STRING_LIMIT(optionStr->c_str(), 92);  // Option title
                WRITE_BYTE(MOT_CALLBACK);   // Type
                WRITE_STRING_LIMIT(optionStr->c_str(), 92);  // Option data
                MESSAGE_END();
            }
        }
        
        pPlayer->InMenu = true;
        MS_ANGEL_INFO("Opened vote menu for %s with %d options via game_master", 
                     pPlayer->DisplayName(), optionCount);
#endif
    }
    
    // Entity property functions using CBaseEntity@
    void AS_SetEntityName(CBaseEntity* entity, const std::string& name)
    {
        if (!entity)
        {
            MS_ANGEL_ERROR("SetEntityName: NULL entity pointer");
            return;
        }
        
        ASEngineProvider::SetEntityName((void*)entity, name);
        MS_ANGEL_DEBUG("SetEntityName: Set name to '%s'", name.c_str());
    }
    
    void AS_SetEntityTargetName(CBaseEntity* entity, const std::string& targetName)
    {
        if (!entity)
        {
            MS_ANGEL_ERROR("SetEntityTargetName: NULL entity pointer");
            return;
        }
        
        ASEngineProvider::SetEntityTargetName((void*)entity, targetName);
        MS_ANGEL_DEBUG("SetEntityTargetName: Set targetname to '%s'", targetName.c_str());
    }
    
    void AS_SetEntityHealth(CBaseEntity* entity, float health)
    {
        if (!entity)
        {
            MS_ANGEL_ERROR("SetEntityHealth: NULL entity pointer");
            return;
        }
        
        ASEngineProvider::SetEntityHealth((void*)entity, health);
        MS_ANGEL_DEBUG("SetEntityHealth: Set health to %f", health);
    }
    
    // Check if entity is dead (more comprehensive than IsAlive)
    bool AS_IsEntityDead(CBaseEntity* entity)
    {
        if (!entity)
        {
            MS_ANGEL_DEBUG("IsEntityDead: NULL entity pointer - considered dead");
            return true;
        }
        
        float health = ASEngineProvider::GetEntityHealth((void*)entity);
        int deadFlag = ASEngineProvider::GetEntityDeadFlag((void*)entity);
        
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
    
    //==========================================================================
    // Entity String Conversion Functions
    //==========================================================================
    
    /**
     * Convert entity string to CBaseEntity
     * Entity strings are in format "PentP(index,address)"
     */
    CBaseEntity* StringToEntity(const std::string& entityString)
    {
        if (entityString.empty())
        {
            MS_ANGEL_DEBUG("StringToEntity: Empty entity string");
            return nullptr;
        }
        
        // Use the shared utility function from sharedutil.h/cpp
        CBaseEntity* pEntity = StringToEnt(entityString.c_str());
        
        if (!pEntity)
        {
            MS_ANGEL_DEBUG("StringToEntity: Failed to convert '%s' to entity", entityString.c_str());
            return nullptr;
        }
        
        MS_ANGEL_DEBUG("StringToEntity: Successfully converted '%s' to entity", entityString.c_str());
        return pEntity;
    }
    
    /**
     * Convert entity string to CBasePlayer
     * Entity strings are in format "PentP(index,address)"
     */
    CBasePlayer* StringToPlayer(const std::string& entityString)
    {
        CBaseEntity* pEntity = StringToEntity(entityString);
        if (!pEntity)
        {
            return nullptr;
        }
        
        return EntityToPlayer_Cast(pEntity);
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
            .method("bool IsPlayer()", &CBaseEntity::IsPlayer)
            // Additional entity configuration methods
            .method("void SetNetName(const string &in)", [](CBaseEntity* entity, const std::string& netname) {
#ifdef VALVE_DLL
                if (FNullEnt(entity))
                    return;

                if (entity && entity->IsMSMonster()) {
                    CMSMonster* pMonster = static_cast<CMSMonster*>(entity);
                    pMonster->m_NetName = netname.c_str();
                    pMonster->pev->netname = MAKE_STRING(pMonster->m_NetName.c_str());
                }
#endif
            })
            .method("string GetNetName()", [](CBaseEntity* entity) -> std::string {
#ifdef VALVE_DLL
                if (FNullEnt(entity))
                    return "";

                if (entity && entity->pev->netname) {
                    return STRING(entity->pev->netname);
                }
#endif
                return "";
            })
            .method("void SetRenderMode(int)", [](CBaseEntity* entity, int renderMode) {
#ifdef VALVE_DLL
                if (entity && !FNullEnt(entity)) {
                    entity->pev->rendermode = renderMode;
                }
#endif
            })
            .method("void SetRenderAmount(int)", [](CBaseEntity* entity, int renderAmount) {
#ifdef VALVE_DLL
                if (entity && !FNullEnt(entity)) {
                    entity->pev->renderamt = renderAmount;
                }
#endif
            })
            .method("void SetTakeDamage(int)", [](CBaseEntity* entity, int takeDamage) {
#ifdef VALVE_DLL
                if (entity && !FNullEnt(entity)) {
                    entity->pev->takedamage = takeDamage;
                }
#endif
            })
            .method("void SetGodMode(bool)", [](CBaseEntity* entity, bool godMode) {
#ifdef VALVE_DLL
                if (entity && !FNullEnt(entity)) {
                    if (godMode)
                        entity->pev->flags |= FL_GODMODE;
                    else
                        entity->pev->flags &= ~FL_GODMODE;
                }
#endif
            });
        
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
            
            // Enhanced messaging methods with color support
            .method("void SendColoredMessage(MessageColor, const string &in)", SendColoredMessage)
            .method("void SendHUDInfoMessage(const string &in, const string &in)", SendHUDInfoMessage)
            
            // Map transition methods
            .method("void SetTransitionFields(const string &in, const string &in, const string &in)", 
                [](CBasePlayer* player, const std::string& localSpawn, const std::string& destMap, const std::string& destSpawn) {
                #ifdef VALVE_DLL
                    if (!player) {
                        MS_ANGEL_DEBUG("SetTransitionFields: NULL player pointer");
                        return;
                    }
                    
                    MS_ANGEL_INFO("SetTransitionFields: BEFORE - OldTrans='%s', NextMap='%s', NextTrans='%s'",
                                   player->m_OldTransition, player->m_NextMap, player->m_NextTransition);
                    
                    // Set transition fields - this is what msarea_transition does
                    strncpy(player->m_OldTransition, localSpawn.c_str(), sizeof(player->m_OldTransition) - 1);
                    player->m_OldTransition[sizeof(player->m_OldTransition) - 1] = '\0';
                    
                    strncpy(player->m_NextMap, destMap.c_str(), sizeof(player->m_NextMap) - 1);
                    player->m_NextMap[sizeof(player->m_NextMap) - 1] = '\0';
                    
                    strncpy(player->m_NextTransition, destSpawn.c_str(), sizeof(player->m_NextTransition) - 1);
                    player->m_NextTransition[sizeof(player->m_NextTransition) - 1] = '\0';
                    
                    // Set spawn transition to current spawn
                    player->m_SpawnTransition = player->m_OldTransition;
                    
                    MS_ANGEL_INFO("SetTransitionFields: AFTER - OldTrans='%s', NextMap='%s', NextTrans='%s', SpawnTrans='%s'",
                                   player->m_OldTransition, player->m_NextMap, player->m_NextTransition, player->m_SpawnTransition);
                    
                    // Save the character with updated transition data
                    player->SaveChar();
                    
                    MS_ANGEL_INFO("SetTransitionFields: Character saved for player %s", player->AuthID().c_str());
                #endif
            })
            .method("string GetOldTransition() const", [](CBasePlayer* player) -> std::string {
                if (!player) return "";
                return player->m_OldTransition;
            })
            .method("string GetNextMap() const", [](CBasePlayer* player) -> std::string {
                if (!player) return "";
                return player->m_NextMap;
            })
            .method("string GetNextTransition() const", [](CBasePlayer* player) -> std::string {
                if (!player) return "";
                return player->m_NextTransition;
            })
            .method("int GetJoinType() const", [](CBasePlayer* player) -> int {
                if (!player) return 0;
                return player->m_JoinType;
            })
            .method("void SetJoinType(int)", [](CBasePlayer* player, int joinType) {
                #ifdef VALVE_DLL
                    if (!player) {
                        MS_ANGEL_DEBUG("SetJoinType: NULL player pointer");
                        return;
                    }
                    player->m_JoinType = joinType;
                    MS_ANGEL_INFO("SetJoinType: Player %s JoinType set to %d", player->AuthID().c_str(), joinType);
                #endif
            })
#ifdef VALVE_DLL
            .method("bool MoveToSpawnSpot()", [](CBasePlayer* player) -> bool {
                if (!player) {
                    MS_ANGEL_DEBUG("MoveToSpawnSpot: NULL player pointer");
                    return false;
                }
                bool result = player->MoveToSpawnSpot();
                MS_ANGEL_INFO("MoveToSpawnSpot: Player %s moved to spawn: %s",
                    player->AuthID().c_str(), result ? "true" : "false");
                return result;
               })
            .method("void SetSpawnTransition(const string &in)", [](CBasePlayer* player, const std::string& transName) {
                if (!player) {
                    MS_ANGEL_DEBUG("SetSpawnTransition: NULL player pointer");
                    return;
                }
                if (player->m_SpawnTransition != NULL) {
                    strncpy((char*)player->m_SpawnTransition, transName.c_str(), 32);
                    ((char*)player->m_SpawnTransition)[31] = '\0';
                    MS_ANGEL_INFO("SetSpawnTransition: Player %s spawn transition set to '%s'",
                        player->AuthID().c_str(), transName.c_str());
                }
            })
            .method("string GetSpawnTransition() const", [](CBasePlayer* player) -> std::string {
                if (!player || !player->m_SpawnTransition) return "";
                return std::string(player->m_SpawnTransition);
            })
#endif
            // Inventory/Item management methods
            #ifdef VALVE_DLL
            .method("CBasePlayerItem@ GetItemBySlot(int) const", [](CBasePlayer* player, int slot) -> CBasePlayerItem* {
                if (!player || !player->pev) {
                    MS_ANGEL_ERROR("GetItemBySlot: NULL player pointer");
                    return nullptr;
                }
                if (slot < 0 || slot >= MAX_ITEM_TYPES) {
                    MS_ANGEL_ERROR("GetItemBySlot: Invalid slot %d", slot);
                    return nullptr;
                }
                CBasePlayerItem* pItem = player->m_rgpPlayerItems[slot];
                MS_ANGEL_DEBUG("GetItemBySlot: Player %s, slot %d = %p", player->DisplayName(), slot, pItem);
                return pItem;
            })
            .method("CBasePlayerWeapon@ GetActiveWeapon() const", [](CBasePlayer* player) -> CBasePlayerWeapon* {
                if (!player || !player->pev) {
                    MS_ANGEL_ERROR("GetActiveWeapon: NULL player pointer");
                    return nullptr;
                }
                CBasePlayerItem* pItem = player->m_pActiveItem;
                if (!pItem) {
                    MS_ANGEL_DEBUG("GetActiveWeapon: Player %s has no active item", player->DisplayName());
                    return nullptr;
                }
                // Check if the active item is a weapon
                CBasePlayerWeapon* pWeapon = pItem->GetWeaponPtr() ? (CBasePlayerWeapon*)pItem : nullptr;
                MS_ANGEL_DEBUG("GetActiveWeapon: Player %s active weapon = %p", player->DisplayName(), pWeapon);
                return pWeapon;
            })
            .method("array<CBasePlayerItem@>@ GetInventory()", [](CBasePlayer* player) -> CScriptArray* {
                if (!g_pStaticEngine) {
                    MS_ANGEL_ERROR("GetInventory: Engine not initialized");
                    return nullptr;
                }
                // Get the array type for CBasePlayerItem@
                asITypeInfo* arrayType = g_pStaticEngine->GetTypeInfoByDecl("array<CBasePlayerItem@>");
                if (!arrayType) {
                    MS_ANGEL_ERROR("GetInventory: Failed to get array<CBasePlayerItem@> type");
                    return nullptr;
                }
                // Create a new array
                CScriptArray* array = CScriptArray::Create(arrayType);
                if (!player || !player->pev) {
                    MS_ANGEL_ERROR("GetInventory: NULL player pointer");
                    return array;  // Return empty array
                }
                // Iterate through all item slots
                for (int i = 0; i < MAX_ITEM_TYPES; i++) {
                    CBasePlayerItem* pItem = player->m_rgpPlayerItems[i];
                    // Walk the linked list of items in this slot
                    while (pItem != nullptr) {
                        array->InsertLast(&pItem);
                        pItem = pItem->m_pNext;
                    }
                }
                MS_ANGEL_DEBUG("GetInventory: Player %s has %d items", player->DisplayName(), array->GetSize());
                return array;
            })
            .method("bool HasItem(const string &in) const", [](CBasePlayer* player, const std::string& itemName) -> bool {
                if (!player || !player->pev || itemName.empty()) {
                    MS_ANGEL_ERROR("HasItem: Invalid parameters");
                    return false;
                }
                // Search through all item slots
                for (int i = 0; i < MAX_ITEM_TYPES; i++) {
                    CBasePlayerItem* pItem = player->m_rgpPlayerItems[i];
                    // Walk the linked list of items in this slot
                    while (pItem != nullptr) {
                        if (pItem->ItemName == itemName.c_str()) {
                            MS_ANGEL_DEBUG("HasItem: Player %s has item '%s'", player->DisplayName(), itemName.c_str());
                            return true;
                        }
                        pItem = pItem->m_pNext;
                    }
                }
                MS_ANGEL_DEBUG("HasItem: Player %s does not have item '%s'", player->DisplayName(), itemName.c_str());
                return false;
            })
            #endif
       
            // Custom equality comparison using pointer comparison (most appropriate for commands)
            .method("bool opEquals(const CBasePlayer@+ other) const", [](CBasePlayer* player, CBasePlayer* other) {
                return player == other;  // Simple pointer comparison
            })
            
            // Inherit from CBaseEntity (called AFTER registering CBasePlayer's own methods
            // so that overrides like IsAlive() are registered first)
            .base<CBaseEntity>();
        
        MS_ANGEL_INFO("Comprehensive CBasePlayer registration complete with enhanced asbind20 patterns");
    }



    void RegisterEntityTypes(asIScriptEngine* pEngine)
    {
        if (!pEngine) return;
        
        // Store engine for wrapper functions
        g_pStaticEngine = pEngine;
        
        MS_ANGEL_INFO("[ASEntityBindings] Registering real entity types...");
        
        // Register MessageColor enum FIRST (before CBasePlayer that uses it)
        pEngine->RegisterEnum("MessageColor");
        pEngine->RegisterEnumValue("MessageColor", "White", static_cast<int>(MessageColor::White));
        pEngine->RegisterEnumValue("MessageColor", "Gray", static_cast<int>(MessageColor::Gray));
        pEngine->RegisterEnumValue("MessageColor", "Yellow", static_cast<int>(MessageColor::Yellow));
        pEngine->RegisterEnumValue("MessageColor", "Red", static_cast<int>(MessageColor::Red));
        pEngine->RegisterEnumValue("MessageColor", "Green", static_cast<int>(MessageColor::Green));
        pEngine->RegisterEnumValue("MessageColor", "Blue", static_cast<int>(MessageColor::Blue));
        
        // Note: CBaseEntity is now registered earlier in ASBindings.cpp (Step 3)
        // This ensures it's available before any derived types use .base<CBaseEntity>()
        
        // Register monster types in inheritance order (CBaseEntity → CBaseMonster → CMSMonster → CBasePlayer)
        ASMonsterBindings::RegisterAll(pEngine);
        
        // Register CBasePlayer with inheritance support (inherits from CMSMonster)
        RegisterCBasePlayer(pEngine);
        
        // Register casting functions with asbind20
        asbind20::global(pEngine)
            .function("CBaseEntity@ ToEntity(CBasePlayer@)", PlayerToEntity_Cast)
            .function("CBasePlayer@ ToPlayer(CBaseEntity@)", EntityToPlayer_Cast)
            // Entity string conversion functions
            .function("CBaseEntity@ StringToEntity(const string &in)", StringToEntity)
            .function("CBasePlayer@ StringToPlayer(const string &in)", StringToPlayer);
        
        // Note: Monster casting functions are registered by ASMonsterBindings::RegisterAll()
        
        // Register engine constants for entity configuration
        // Using static variables for AngelScript global properties
        static const int const_kRenderNormal = 0;
        static const int const_kRenderTransColor = 1;
        static const int const_kRenderTransTexture = 2;
        static const int const_kRenderGlow = 3;
        static const int const_kRenderTransAlpha = 4;
        static const int const_kRenderTransAdd = 5;
        static const int const_DAMAGE_NO = 0;
        static const int const_DAMAGE_YES = 1;
        static const int const_DAMAGE_AIM = 2;
        
        pEngine->RegisterGlobalProperty("const int kRenderNormal", (void*)&const_kRenderNormal);
        pEngine->RegisterGlobalProperty("const int kRenderTransColor", (void*)&const_kRenderTransColor);
        pEngine->RegisterGlobalProperty("const int kRenderTransTexture", (void*)&const_kRenderTransTexture);
        pEngine->RegisterGlobalProperty("const int kRenderGlow", (void*)&const_kRenderGlow);
        pEngine->RegisterGlobalProperty("const int kRenderTransAlpha", (void*)&const_kRenderTransAlpha);
        pEngine->RegisterGlobalProperty("const int kRenderTransAdd", (void*)&const_kRenderTransAdd);
        
        pEngine->RegisterGlobalProperty("const int DAMAGE_NO", (void*)&const_DAMAGE_NO);
        pEngine->RegisterGlobalProperty("const int DAMAGE_YES", (void*)&const_DAMAGE_YES);
        pEngine->RegisterGlobalProperty("const int DAMAGE_AIM", (void*)&const_DAMAGE_AIM);
        
        // Register ScriptMode enum for controlling script loading behavior
        pEngine->RegisterEnum("ScriptMode");
        pEngine->RegisterEnumValue("ScriptMode", "Legacy", static_cast<int>(ScriptMode::Legacy));
        pEngine->RegisterEnumValue("ScriptMode", "Angel", static_cast<int>(ScriptMode::Angel));
        pEngine->RegisterEnumValue("ScriptMode", "Both", static_cast<int>(ScriptMode::Both));
        
        // Note: MessageColor enum is registered earlier (before CBasePlayer registration)
        
        MS_ANGEL_INFO("[ASEntityBindings] Entity types and constants registration complete");
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
            .function("CBasePlayer@ PlayerBySteamID(const string &in)", AS_PlayerBySteamID)
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
            .function("void MS_ANGEL_ERROR(const string &in)", [](const std::string& message) { AS_LogAngelError(message); })
            // Vote menu opening function
            .function("void OpenVoteMenu(CBasePlayer@, const string &in, const array<string> &in)", AS_OpenVoteMenu)
            // Spawn functions
            .function("CMSMonster@ SpawnNPC(const string &in, const Vector3 &in, const array<string>@ = null, ScriptMode = Legacy)", 
                +[](const std::string& scriptName, const Vector& position, CScriptArray* params, ScriptMode spawnMode) -> CMSMonster* {
                    return AS_SpawnNPC(scriptName, position, params, spawnMode);
                })
            .function("CBasePlayerItem@ SpawnItem(const string &in, const Vector3 &in, const array<string>@ = null)", 
                +[](const std::string& scriptName, const Vector& position, CScriptArray* params) -> CBasePlayerItem* {
                    return AS_SpawnItem(scriptName, position, params);
                });
        
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
        
        // Note: Core types (Vector3, Color, etc.) are registered by ASBindings.cpp
        // We only register entity-specific bindings here
        
        // Register entity types first (CBaseEntity, CBasePlayer)
        RegisterEntityTypes(pEngine);
        
        // Now register engine bindings that depend on entity types
        ASEngineBindings::RegisterAll(pEngine);
        
        // Register global functions
        RegisterGlobalFunctions(pEngine);
        
        // Test the entity bindings to ensure they work
        AS_TestEntityBindings();
        
        MS_ANGEL_INFO("[ASEntityBindings] Entity bindings registration complete");
    }
}