//==========================================================================
// ASServerEngine.cpp
// 
// Server-side implementations of engine integration functions for AngelScript
//==========================================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player/player.h"
#include "gamerules.h"
#include "mslogger.h"

// Server-side implementations using actual engine connections
extern "C" {

    float ServerGetEngineGameTime()
    {
        MS_ANGEL_DEBUG("ServerGetEngineGameTime: Returning gpGlobals->time");
        return gpGlobals->time;
    }

    const char* ServerGetEngineCvarString(const char* name)
    {
        if (!name) {
            MS_ANGEL_DEBUG("ServerGetEngineCvarString: NULL cvar name");
            return "";
        }
        
        const char* value = CVAR_GET_STRING(name);
        MS_ANGEL_DEBUG("ServerGetEngineCvarString: '%s' = '%s'", name, value ? value : "NULL");
        return value ? value : "";
    }

    const char* ServerGetEngineMapName()
    {
        const char* mapName = STRING(gpGlobals->mapname);
        MS_ANGEL_DEBUG("ServerGetEngineMapName: '%s'", mapName ? mapName : "NULL");
        return mapName ? mapName : "unknown_map";
    }

    void* ServerCreateEngineEntity(const char* classname)
    {
        if (!classname) {
            MS_ANGEL_DEBUG("ServerCreateEngineEntity: NULL classname");
            return nullptr;
        }
        
        edict_t* pEdict = CREATE_NAMED_ENTITY(MAKE_STRING(classname));
        if (!pEdict) {
            MS_ANGEL_DEBUG("ServerCreateEngineEntity: Failed to create entity '%s'", classname);
            return nullptr;
        }
        
        CBaseEntity* pEntity = CBaseEntity::Instance(pEdict);
        MS_ANGEL_DEBUG("ServerCreateEngineEntity: Created entity '%s' at %p", classname, pEntity);
        return pEntity;
    }

    void ServerSetEngineEntityOrigin(void* entity, float x, float y, float z)
    {
        if (!entity) {
            MS_ANGEL_DEBUG("ServerSetEngineEntityOrigin: NULL entity");
            return;
        }
        
        CBaseEntity* pEntity = static_cast<CBaseEntity*>(entity);
        Vector origin(x, y, z);
        
        if (pEntity->edict()) {
            UTIL_SetOrigin(VARS(pEntity->edict()), origin);
            MS_ANGEL_DEBUG("ServerSetEngineEntityOrigin: Set origin to (%f, %f, %f)", x, y, z);
        } else {
            MS_ANGEL_DEBUG("ServerSetEngineEntityOrigin: Entity has no edict");
        }
    }

    void ServerSetEngineEntityName(void* entity, const char* name)
    {
        if (!entity) {
            MS_ANGEL_DEBUG("ServerSetEngineEntityName: NULL entity");
            return;
        }
        
        if (!name) {
            MS_ANGEL_DEBUG("ServerSetEngineEntityName: NULL name");
            return;
        }
        
        CBaseEntity* pEntity = static_cast<CBaseEntity*>(entity);
        if (pEntity->edict()) {
            pEntity->edict()->v.netname = MAKE_STRING(name);
            MS_ANGEL_DEBUG("ServerSetEngineEntityName: Set name to '%s'", name);
        } else {
            MS_ANGEL_DEBUG("ServerSetEngineEntityName: Entity has no edict");
        }
    }

    void ServerSetEngineEntityTargetName(void* entity, const char* targetname)
    {
        if (!entity) {
            MS_ANGEL_DEBUG("ServerSetEngineEntityTargetName: NULL entity");
            return;
        }
        
        if (!targetname) {
            MS_ANGEL_DEBUG("ServerSetEngineEntityTargetName: NULL targetname");
            return;
        }
        
        CBaseEntity* pEntity = static_cast<CBaseEntity*>(entity);
        if (pEntity->edict()) {
            pEntity->edict()->v.targetname = MAKE_STRING(targetname);
            MS_ANGEL_DEBUG("ServerSetEngineEntityTargetName: Set targetname to '%s'", targetname);
        } else {
            MS_ANGEL_DEBUG("ServerSetEngineEntityTargetName: Entity has no edict");
        }
    }

    void ServerSetEngineEntityHealth(void* entity, float health)
    {
        if (!entity) {
            MS_ANGEL_DEBUG("ServerSetEngineEntityHealth: NULL entity");
            return;
        }
        
        CBaseEntity* pEntity = static_cast<CBaseEntity*>(entity);
        if (pEntity->edict()) {
            pEntity->edict()->v.health = health;
            MS_ANGEL_DEBUG("ServerSetEngineEntityHealth: Set health to %f", health);
        } else {
            MS_ANGEL_DEBUG("ServerSetEngineEntityHealth: Entity has no edict");
        }
    }

    float ServerGetEngineEntityHealth(void* entity)
    {
        if (!entity) {
            MS_ANGEL_DEBUG("ServerGetEngineEntityHealth: NULL entity");
            return 0.0f;
        }
        
        CBaseEntity* pEntity = static_cast<CBaseEntity*>(entity);
        if (pEntity->edict()) {
            float health = pEntity->edict()->v.health;
            MS_ANGEL_DEBUG("ServerGetEngineEntityHealth: Health is %f", health);
            return health;
        } else {
            MS_ANGEL_DEBUG("ServerGetEngineEntityHealth: Entity has no edict");
            return 0.0f;
        }
    }

    int ServerGetEngineEntityDeadFlag(void* entity)
    {
        if (!entity) {
            MS_ANGEL_DEBUG("ServerGetEngineEntityDeadFlag: NULL entity");
            return DEAD_DEAD; // Assume dead if no entity
        }
        
        CBaseEntity* pEntity = static_cast<CBaseEntity*>(entity);
        if (pEntity->edict()) {
            int deadFlag = pEntity->edict()->v.deadflag;
            MS_ANGEL_DEBUG("ServerGetEngineEntityDeadFlag: Dead flag is %d", deadFlag);
            return deadFlag;
        } else {
            MS_ANGEL_DEBUG("ServerGetEngineEntityDeadFlag: Entity has no edict");
            return DEAD_DEAD;
        }
    }

    const char* ServerGetEngineEntityClassName(void* entity)
    {
        if (!entity) {
            MS_ANGEL_DEBUG("ServerGetEngineEntityClassName: NULL entity");
            return "unknown_entity";
        }
        
        CBaseEntity* pEntity = static_cast<CBaseEntity*>(entity);
        if (pEntity->edict()) {
            const char* className = STRING(pEntity->edict()->v.classname);
            MS_ANGEL_DEBUG("ServerGetEngineEntityClassName: Class name is '%s'", className ? className : "NULL");
            return className ? className : "unknown_entity";
        } else {
            MS_ANGEL_DEBUG("ServerGetEngineEntityClassName: Entity has no edict");
            return "unknown_entity";
        }
    }

    void ServerGetEngineEntityOrigin(void* entity, float* x, float* y, float* z)
    {
        if (!entity) {
            MS_ANGEL_DEBUG("ServerGetEngineEntityOrigin: NULL entity");
            if (x) *x = 0.0f;
            if (y) *y = 0.0f;
            if (z) *z = 0.0f;
            return;
        }
        
        CBaseEntity* pEntity = static_cast<CBaseEntity*>(entity);
        if (pEntity->edict()) {
            Vector origin = pEntity->edict()->v.origin;
            if (x) *x = origin.x;
            if (y) *y = origin.y;
            if (z) *z = origin.z;
            MS_ANGEL_DEBUG("ServerGetEngineEntityOrigin: Origin is (%f, %f, %f)", origin.x, origin.y, origin.z);
        } else {
            MS_ANGEL_DEBUG("ServerGetEngineEntityOrigin: Entity has no edict");
            if (x) *x = 0.0f;
            if (y) *y = 0.0f;
            if (z) *z = 0.0f;
        }
    }
}