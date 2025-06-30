//==========================================================================
// ASPlayerBridge.cpp
// 
// Bridge functions to connect AngelScript player bindings to the actual
// game server implementation, avoiding header conflicts in shared code
//==========================================================================

// Minimal includes to avoid conflicts
#include "extdll.h"
#include "enginecallback.h"

// Forward declarations
struct edict_s;
typedef struct edict_s edict_t;

// External variables and functions
extern globalvars_t* gpGlobals;

// Forward declaration for CBaseEntity
class CBaseEntity;
extern CBaseEntity* UTIL_PlayerByIndex(int playerIndex);

// Bridge functions to connect ASEntityBindings to actual game code
extern "C" {
    const char* GetPlayerAuthId_AS(void* edict)
    {
        if (!edict) return "STEAM_ID_INVALID";
        return g_engfuncs.pfnGetPlayerAuthId((edict_t*)edict);
    }
    
    void EmitSound_AS(void* edict, int channel, const char* sound, float volume, float attn, int flags, int pitch)
    {
        if (!edict || !sound) return;
        EMIT_SOUND_DYN2((edict_t*)edict, channel, sound, volume, attn, flags, pitch);
    }
    
    void SendInfoMsg_AS(void* player, const char* message)
    {
        // Simplified implementation - use CLIENT_PRINTF for console output
        if (!player || !message) return;
        // Cast to minimal entity structure to get edict
        struct MinimalEntity { void* vtable; edict_t* edict_ptr; };
        MinimalEntity* pEntity = (MinimalEntity*)player;
        if (pEntity && pEntity->edict_ptr)
        {
            CLIENT_PRINTF(pEntity->edict_ptr, print_console, message);
        }
    }
    
    void* PlayerByIndex_AS(int index)
    {
        return (void*)UTIL_PlayerByIndex(index);
    }
    
    int GetMaxClients_AS()
    {
        return gpGlobals->maxClients;
    }
    
    const char* GetPlayerDisplayName_AS(void* player)
    {
        if (!player) return "Unknown";
        // Use minimal entity structure to access edict
        struct MinimalEntity { void* vtable; edict_t* edict_ptr; };
        MinimalEntity* pEntity = (MinimalEntity*)player;
        if (pEntity && pEntity->edict_ptr && pEntity->edict_ptr->v.netname)
        {
            return g_engfuncs.pfnSzFromIndex(pEntity->edict_ptr->v.netname);
        }
        return "Unnamed Player";
    }
    
    const char* GetPlayerClientAddress_AS(void* player)
    {
        // For now, return a placeholder - getting IP requires more complex setup
        return "Unknown";
    }
    
    int GetPlayerEntIndex_AS(void* player)
    {
        if (!player) return -1;
        struct MinimalEntity { void* vtable; edict_t* edict_ptr; };
        MinimalEntity* pEntity = (MinimalEntity*)player;
        if (pEntity && pEntity->edict_ptr)
        {
            return g_engfuncs.pfnIndexOfEdict(pEntity->edict_ptr);
        }
        return -1;
    }
    
    int IsValidPlayer_AS(void* player)
    {
        if (!player) return 0;
        struct MinimalEntity { void* vtable; edict_t* edict_ptr; };
        MinimalEntity* pEntity = (MinimalEntity*)player;
        if (pEntity && pEntity->edict_ptr)
        {
            int index = g_engfuncs.pfnIndexOfEdict(pEntity->edict_ptr);
            return (index > 0 && index <= gpGlobals->maxClients) ? 1 : 0;
        }
        return 0;
    }
    
    // Function to set player context for AngelScript execution
    void AS_SetPlayerContext_AS(void* player)
    {
        // This will be called by the script system when executing scripts for a specific player
        // Implementation will be added when we integrate with the script execution system
    }
}