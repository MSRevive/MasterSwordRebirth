//==========================================================================
// ASClientEngineImpl.cpp
// 
// Client-side implementation of the AngelScript engine interface
//==========================================================================

#include "ASClientEngineImpl.h"

// Include wrect.h first to define types used by engine headers
#include "wrect.h"

// Client-side includes
#include "cl_dll.h"
#include "cl_util.h"
#include "cl_entity.h"
#include "parsemsg.h"
#include "mslogger.h"

// Client engine interface
extern cl_enginefunc_t gEngfuncs;

//==========================================================================
// ASClientEngine Implementation
//==========================================================================

ASClientEngine& ASClientEngine::GetInstance()
{
    static ASClientEngine instance;
    return instance;
}

float ASClientEngine::GetGameTimeImpl() const
{
    float time = gEngfuncs.GetClientTime();
    MS_ANGEL_DEBUG("ClientGetEngineGameTime: %f", time);
    return time;
}

std::string ASClientEngine::GetCvarStringImpl(const std::string& name) const
{
    if (name.empty()) {
        MS_ANGEL_DEBUG("ClientGetEngineCvarString: Empty cvar name");
        return "";
    }
    
    const char* value = gEngfuncs.pfnGetCvarString(name.c_str());
    MS_ANGEL_DEBUG("ClientGetEngineCvarString: '%s' = '%s'", name.c_str(), value ? value : "NULL");
    return value ? value : "";
}

std::string ASClientEngine::GetMapNameImpl() const
{
    // On client side, we can get the level name
    const char* levelName = gEngfuncs.pfnGetLevelName();
    if (levelName && strlen(levelName) > 0) {
        // Remove "maps/" prefix and ".bsp" suffix if present
        std::string mapName = levelName;
        if (mapName.find("maps/") == 0) {
            mapName = mapName.substr(5);
        }
        if (mapName.find(".bsp") != std::string::npos) {
            mapName = mapName.substr(0, mapName.find(".bsp"));
        }
        MS_ANGEL_DEBUG("ClientGetEngineMapName: '%s'", mapName.c_str());
        return mapName;
    }
    
    MS_ANGEL_DEBUG("ClientGetEngineMapName: unknown_map");
    return "unknown_map";
}

void* ASClientEngine::CreateEntityImpl(const std::string& classname)
{
    // Client-side entities are typically read-only from server
    // This is a stub implementation
    MS_ANGEL_DEBUG("ClientCreateEngineEntity: Client-side entity creation not supported for '%s'", classname.c_str());
    return nullptr;
}

void ASClientEngine::SetEntityOriginImpl(void* entity, const Vector& origin)
{
    // Client-side entities are typically read-only from server
    MS_ANGEL_DEBUG("ClientSetEngineEntityOrigin: Client-side entity modification not supported");
}

void ASClientEngine::SetEntityNameImpl(void* entity, const std::string& name)
{
    // Client-side entities are typically read-only from server
    MS_ANGEL_DEBUG("ClientSetEngineEntityName: Client-side entity modification not supported");
}

void ASClientEngine::SetEntityTargetNameImpl(void* entity, const std::string& targetname)
{
    // Client-side entities are typically read-only from server
    MS_ANGEL_DEBUG("ClientSetEngineEntityTargetName: Client-side entity modification not supported");
}

void ASClientEngine::SetEntityHealthImpl(void* entity, float health)
{
    // Client-side entities are typically read-only from server
    MS_ANGEL_DEBUG("ClientSetEngineEntityHealth: Client-side entity modification not supported");
}

Vector ASClientEngine::GetEntityOriginImpl(void* entity) const
{
    cl_entity_t* pEntity = static_cast<cl_entity_t*>(ValidateClientEntity(entity));
    if (pEntity) {
        Vector origin(pEntity->origin[0], pEntity->origin[1], pEntity->origin[2]);
        MS_ANGEL_DEBUG("ClientGetEngineEntityOrigin: (%f, %f, %f)", origin.x, origin.y, origin.z);
        return origin;
    }
    
    MS_ANGEL_DEBUG("ClientGetEngineEntityOrigin: Invalid entity");
    return Vector(0, 0, 0);
}

float ASClientEngine::GetEntityHealthImpl(void* entity) const
{
    // Client doesn't typically have access to arbitrary entity health
    // This would need to be synchronized from server
    MS_ANGEL_DEBUG("ClientGetEngineEntityHealth: Client-side health access limited");
    return 100.0f; // Default value
}

int ASClientEngine::GetEntityDeadFlagImpl(void* entity) const
{
    // Client doesn't typically have access to entity dead flags
    MS_ANGEL_DEBUG("ClientGetEngineEntityDeadFlag: Client-side deadflag access limited");
    return 0; // DEAD_NO
}

std::string ASClientEngine::GetEntityClassNameImpl(void* entity) const
{
    cl_entity_t* pEntity = static_cast<cl_entity_t*>(ValidateClientEntity(entity));
    if (pEntity) {
        // Client-side entities don't have direct classname access like server
        MS_ANGEL_DEBUG("ClientGetEngineEntityClassName: client_entity");
        return "client_entity"; // Generic client entity
    }
    
    MS_ANGEL_DEBUG("ClientGetEngineEntityClassName: unknown_entity");
    return "unknown_entity";
}

std::string ASClientEngine::GetPlayerAuthIdImpl(void* player) const
{
    // Client-side doesn't have access to other players' auth IDs for security
    MS_ANGEL_DEBUG("ClientGetPlayerAuthId: Client-side access not available");
    return "STEAM_ID_UNAVAILABLE";
}

std::string ASClientEngine::GetPlayerDisplayNameImpl(void* player) const
{
    // Client can access player names from player info
    // This would need proper implementation with player index lookup
    MS_ANGEL_DEBUG("ClientGetPlayerDisplayName: Client-side player name access");
    return "Client Player";
}

std::string ASClientEngine::GetPlayerClientAddressImpl(void* player) const
{
    // Client-side doesn't have access to IP addresses for security
    MS_ANGEL_DEBUG("ClientGetPlayerClientAddress: Client-side access not available");
    return "127.0.0.1";
}

int ASClientEngine::GetPlayerEntIndexImpl(void* player) const
{
    // This would need proper implementation
    MS_ANGEL_DEBUG("ClientGetPlayerEntIndex: Client-side implementation needed");
    return -1;
}

bool ASClientEngine::IsValidPlayerImpl(void* player) const
{
    // Basic validation
    MS_ANGEL_DEBUG("ClientIsValidPlayer: Basic validation");
    return player != nullptr;
}

void* ASClientEngine::PlayerByIndexImpl(int index)
{
    // Client-side player lookup
    if (index < 1 || index > gEngfuncs.GetMaxClients()) {
        MS_ANGEL_DEBUG("ClientPlayerByIndex: Invalid index %d", index);
        return nullptr;
    }
    
    // This would need proper implementation to return client player entity
    MS_ANGEL_DEBUG("ClientPlayerByIndex: Index %d", index);
    return nullptr; // Stub
}

int ASClientEngine::GetMaxClientsImpl() const
{
    int maxClients = gEngfuncs.GetMaxClients();
    MS_ANGEL_DEBUG("ClientGetMaxClients: %d", maxClients);
    return maxClients;
}

void ASClientEngine::SendInfoMsgImpl(void* player, const std::string& message)
{
    // Client-side message display
    if (message.empty()) return;
    
    // Use client-side console print
    gEngfuncs.Con_Printf("%s\n", message.c_str());
    MS_ANGEL_DEBUG("ClientSendInfoMsg: %s", message.c_str());
}

void ASClientEngine::EmitSoundImpl(void* entity, int channel, const std::string& sound, float volume, float attn, int flags, int pitch)
{
    if (sound.empty()) return;
    
    // Client-side sound emission - simplified approach
    Vector origin(0, 0, 0);
    cl_entity_t* pEntity = static_cast<cl_entity_t*>(ValidateClientEntity(entity));
    if (pEntity) {
        origin = Vector(pEntity->origin[0], pEntity->origin[1], pEntity->origin[2]);
    }
    
    // Use simplified client sound API to avoid efx_api_s type issues
    gEngfuncs.pfnPlaySoundByNameAtLocation(const_cast<char*>(sound.c_str()), volume, origin);
    MS_ANGEL_DEBUG("ClientEmitSound: %s at volume %f", sound.c_str(), volume);
}

//==========================================================================
// Helper Functions
//==========================================================================

void* ASClientEngine::ValidateClientEntity(void* entity) const
{
    if (!entity) {
        MS_ANGEL_DEBUG("ValidateClientEntity: NULL entity pointer");
        return nullptr;
    }
    
    // Basic validation - in a real implementation, this would need more checks
    return entity;
}

// Note: All C-style wrapper functions have been removed.
// The new template system uses ASEngineProvider directly through asbind20.
// Client-side doesn't need legacy C functions as ASEntityBindings uses server-side functions.