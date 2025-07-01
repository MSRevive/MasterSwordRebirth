//==========================================================================
// ASServerEngineImpl.cpp
// 
// Server-side implementation of the AngelScript engine interface
//==========================================================================

#include "ASServerEngineImpl.h"

// Server-side includes
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player/player.h"
#include "gamerules.h"
#include "mslogger.h"

//==========================================================================
// ASServerEngine Implementation
//==========================================================================

ASServerEngine& ASServerEngine::GetInstance()
{
    static ASServerEngine instance;
    return instance;
}

float ASServerEngine::GetGameTimeImpl() const
{
    MS_ANGEL_DEBUG("ServerGetEngineGameTime: Returning gpGlobals->time");
    return gpGlobals->time;
}

std::string ASServerEngine::GetCvarStringImpl(const std::string& name) const
{
    if (name.empty()) {
        MS_ANGEL_DEBUG("ServerGetEngineCvarString: Empty cvar name");
        return "";
    }
    
    const char* value = CVAR_GET_STRING(name.c_str());
    MS_ANGEL_DEBUG("ServerGetEngineCvarString: '%s' = '%s'", name.c_str(), value ? value : "NULL");
    return value ? value : "";
}

std::string ASServerEngine::GetMapNameImpl() const
{
    const char* mapName = STRING(gpGlobals->mapname);
    MS_ANGEL_DEBUG("ServerGetEngineMapName: '%s'", mapName ? mapName : "NULL");
    return mapName ? mapName : "unknown_map";
}

void* ASServerEngine::CreateEntityImpl(const std::string& classname)
{
    if (classname.empty()) {
        MS_ANGEL_DEBUG("ServerCreateEngineEntity: Empty classname");
        return nullptr;
    }
    
    edict_t* pEdict = CREATE_NAMED_ENTITY(MAKE_STRING(classname.c_str()));
    if (!pEdict) {
        MS_ANGEL_DEBUG("ServerCreateEngineEntity: Failed to create entity '%s'", classname.c_str());
        return nullptr;
    }
    
    CBaseEntity* pEntity = CBaseEntity::Instance(pEdict);
    MS_ANGEL_DEBUG("ServerCreateEngineEntity: Created entity '%s' at %p", classname.c_str(), pEntity);
    return pEntity;
}

void ASServerEngine::SetEntityOriginImpl(void* entity, const Vector& origin)
{
    CBaseEntity* pEntity = static_cast<CBaseEntity*>(ValidateEntity(entity));
    if (!pEntity) return;
    
    if (pEntity->edict()) {
        UTIL_SetOrigin(VARS(pEntity->edict()), origin);
        MS_ANGEL_DEBUG("ServerSetEngineEntityOrigin: Set origin to (%f, %f, %f)", origin.x, origin.y, origin.z);
    } else {
        MS_ANGEL_DEBUG("ServerSetEngineEntityOrigin: Entity has no edict");
    }
}

void ASServerEngine::SetEntityNameImpl(void* entity, const std::string& name)
{
    CBaseEntity* pEntity = static_cast<CBaseEntity*>(ValidateEntity(entity));
    if (!pEntity) return;
    
    if (name.empty()) {
        MS_ANGEL_DEBUG("ServerSetEngineEntityName: Empty name");
        return;
    }
    
    if (pEntity->edict()) {
        pEntity->edict()->v.netname = MAKE_STRING(name.c_str());
        MS_ANGEL_DEBUG("ServerSetEngineEntityName: Set name to '%s'", name.c_str());
    } else {
        MS_ANGEL_DEBUG("ServerSetEngineEntityName: Entity has no edict");
    }
}

void ASServerEngine::SetEntityTargetNameImpl(void* entity, const std::string& targetname)
{
    CBaseEntity* pEntity = static_cast<CBaseEntity*>(ValidateEntity(entity));
    if (!pEntity) return;
    
    if (targetname.empty()) {
        MS_ANGEL_DEBUG("ServerSetEngineEntityTargetName: Empty targetname");
        return;
    }
    
    if (pEntity->edict()) {
        pEntity->edict()->v.targetname = MAKE_STRING(targetname.c_str());
        MS_ANGEL_DEBUG("ServerSetEngineEntityTargetName: Set targetname to '%s'", targetname.c_str());
    } else {
        MS_ANGEL_DEBUG("ServerSetEngineEntityTargetName: Entity has no edict");
    }
}

void ASServerEngine::SetEntityHealthImpl(void* entity, float health)
{
    CBaseEntity* pEntity = static_cast<CBaseEntity*>(ValidateEntity(entity));
    if (!pEntity) return;
    
    if (pEntity->edict()) {
        pEntity->edict()->v.health = health;
        MS_ANGEL_DEBUG("ServerSetEngineEntityHealth: Set health to %f", health);
    } else {
        MS_ANGEL_DEBUG("ServerSetEngineEntityHealth: Entity has no edict");
    }
}

Vector ASServerEngine::GetEntityOriginImpl(void* entity) const
{
    CBaseEntity* pEntity = static_cast<CBaseEntity*>(ValidateEntity(entity));
    if (!pEntity) return Vector(0, 0, 0);
    
    if (pEntity->edict()) {
        Vector origin = pEntity->edict()->v.origin;
        MS_ANGEL_DEBUG("ServerGetEngineEntityOrigin: Origin is (%f, %f, %f)", origin.x, origin.y, origin.z);
        return origin;
    } else {
        MS_ANGEL_DEBUG("ServerGetEngineEntityOrigin: Entity has no edict");
        return Vector(0, 0, 0);
    }
}

float ASServerEngine::GetEntityHealthImpl(void* entity) const
{
    CBaseEntity* pEntity = static_cast<CBaseEntity*>(ValidateEntity(entity));
    if (!pEntity) return 0.0f;
    
    if (pEntity->edict()) {
        float health = pEntity->edict()->v.health;
        MS_ANGEL_DEBUG("ServerGetEngineEntityHealth: Health is %f", health);
        return health;
    } else {
        MS_ANGEL_DEBUG("ServerGetEngineEntityHealth: Entity has no edict");
        return 0.0f;
    }
}

int ASServerEngine::GetEntityDeadFlagImpl(void* entity) const
{
    CBaseEntity* pEntity = static_cast<CBaseEntity*>(ValidateEntity(entity));
    if (!pEntity) return DEAD_DEAD; // Assume dead if no entity
    
    if (pEntity->edict()) {
        int deadFlag = pEntity->edict()->v.deadflag;
        MS_ANGEL_DEBUG("ServerGetEngineEntityDeadFlag: Dead flag is %d", deadFlag);
        return deadFlag;
    } else {
        MS_ANGEL_DEBUG("ServerGetEngineEntityDeadFlag: Entity has no edict");
        return DEAD_DEAD;
    }
}

std::string ASServerEngine::GetEntityClassNameImpl(void* entity) const
{
    CBaseEntity* pEntity = static_cast<CBaseEntity*>(ValidateEntity(entity));
    if (!pEntity) return "unknown_entity";
    
    if (pEntity->edict()) {
        const char* className = STRING(pEntity->edict()->v.classname);
        MS_ANGEL_DEBUG("ServerGetEngineEntityClassName: Class name is '%s'", className ? className : "NULL");
        return className ? className : "unknown_entity";
    } else {
        MS_ANGEL_DEBUG("ServerGetEngineEntityClassName: Entity has no edict");
        return "unknown_entity";
    }
}

std::string ASServerEngine::GetPlayerAuthIdImpl(void* player) const
{
    if (!player) return "STEAM_ID_INVALID";
    
    // Use minimal entity structure to access edict
    struct MinimalEntity { void* vtable; edict_t* edict_ptr; };
    MinimalEntity* pEntity = (MinimalEntity*)player;
    if (pEntity && pEntity->edict_ptr) {
        return g_engfuncs.pfnGetPlayerAuthId(pEntity->edict_ptr);
    }
    return "STEAM_ID_INVALID";
}

std::string ASServerEngine::GetPlayerDisplayNameImpl(void* player) const
{
    if (!player) return "Unknown";
    
    struct MinimalEntity { void* vtable; edict_t* edict_ptr; };
    MinimalEntity* pEntity = (MinimalEntity*)player;
    if (pEntity && pEntity->edict_ptr && pEntity->edict_ptr->v.netname) {
        return g_engfuncs.pfnSzFromIndex(pEntity->edict_ptr->v.netname);
    }
    return "Unnamed Player";
}

std::string ASServerEngine::GetPlayerClientAddressImpl(void* player) const
{
    // For now, return a placeholder - getting IP requires more complex setup
    return "Unknown";
}

int ASServerEngine::GetPlayerEntIndexImpl(void* player) const
{
    if (!player) return -1;
    
    struct MinimalEntity { void* vtable; edict_t* edict_ptr; };
    MinimalEntity* pEntity = (MinimalEntity*)player;
    if (pEntity && pEntity->edict_ptr) {
        return g_engfuncs.pfnIndexOfEdict(pEntity->edict_ptr);
    }
    return -1;
}

bool ASServerEngine::IsValidPlayerImpl(void* player) const
{
    if (!player) return false;
    
    struct MinimalEntity { void* vtable; edict_t* edict_ptr; };
    MinimalEntity* pEntity = (MinimalEntity*)player;
    if (pEntity && pEntity->edict_ptr) {
        int index = g_engfuncs.pfnIndexOfEdict(pEntity->edict_ptr);
        return (index > 0 && index <= gpGlobals->maxClients);
    }
    return false;
}

void* ASServerEngine::PlayerByIndexImpl(int index)
{
    return (void*)UTIL_PlayerByIndex(index);
}

int ASServerEngine::GetMaxClientsImpl() const
{
    return gpGlobals->maxClients;
}

void ASServerEngine::SendInfoMsgImpl(void* player, const std::string& message)
{
    if (!player || message.empty()) return;
    
    struct MinimalEntity { void* vtable; edict_t* edict_ptr; };
    MinimalEntity* pEntity = (MinimalEntity*)player;
    if (pEntity && pEntity->edict_ptr) {
        CLIENT_PRINTF(pEntity->edict_ptr, print_console, message.c_str());
    }
}

void ASServerEngine::EmitSoundImpl(void* entity, int channel, const std::string& sound, float volume, float attn, int flags, int pitch)
{
    if (!entity || sound.empty()) return;
    
    struct MinimalEntity { void* vtable; edict_t* edict_ptr; };
    MinimalEntity* pEntity = (MinimalEntity*)entity;
    if (pEntity && pEntity->edict_ptr) {
        EMIT_SOUND_DYN2(pEntity->edict_ptr, channel, sound.c_str(), volume, attn, flags, pitch);
    }
}

//==========================================================================
// Helper Functions
//==========================================================================

void* ASServerEngine::ValidateEntity(void* entity) const
{
    if (!entity) {
        MS_ANGEL_DEBUG("ValidateEntity: NULL entity pointer");
        return nullptr;
    }
    return entity;
}

void* ASServerEngine::ValidatePlayer(void* player) const
{
    if (!player) {
        MS_ANGEL_DEBUG("ValidatePlayer: NULL player pointer");
        return nullptr;
    }
    return player;
}

void* ASServerEngine::GetEdict(void* entity) const
{
    if (!entity) return nullptr;
    CBaseEntity* pEntity = static_cast<CBaseEntity*>(entity);
    return pEntity->edict();
}

// Note: All C-style wrapper functions have been removed.
// ASEntityBindings.cpp now uses ASEngineProvider directly.