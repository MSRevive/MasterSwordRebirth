//==========================================================================
// ASClientEngine.cpp
// 
// Client-side implementations of engine integration functions for AngelScript
//==========================================================================

#include "hud.h"
#include "cl_util.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "mslogger.h"

// Client-side implementations using engine connections
extern "C" {

    float ClientGetEngineGameTime()
    {
        MS_ANGEL_DEBUG("ClientGetEngineGameTime: Returning client time");
        return gEngfuncs.GetClientTime();
    }

    const char* ClientGetEngineCvarString(const char* name)
    {
        if (!name) {
            MS_ANGEL_DEBUG("ClientGetEngineCvarString: NULL cvar name");
            return "";
        }
        
        // Client doesn't have direct access to cvars in the same way
        MS_ANGEL_DEBUG("ClientGetEngineCvarString: '%s' - limited client access", name);
        return "";
    }

    const char* ClientGetEngineMapName()
    {
        // Client can get map name from the engine
        const char* mapName = gEngfuncs.pfnGetLevelName();
        MS_ANGEL_DEBUG("ClientGetEngineMapName: '%s'", mapName ? mapName : "NULL");
        return mapName ? mapName : "unknown_map";
    }

    void* ClientCreateEngineEntity(const char* classname)
    {
        if (!classname) {
            MS_ANGEL_DEBUG("ClientCreateEngineEntity: NULL classname");
            return nullptr;
        }
        
        // Client can't create entities
        MS_ANGEL_DEBUG("ClientCreateEngineEntity: Client cannot create entities");
        return nullptr;
    }

    void ClientSetEngineEntityOrigin(void* entity, float x, float y, float z)
    {
        MS_ANGEL_DEBUG("ClientSetEngineEntityOrigin: Client cannot modify entity origins");
    }

    void ClientSetEngineEntityName(void* entity, const char* name)
    {
        MS_ANGEL_DEBUG("ClientSetEngineEntityName: Client cannot modify entity names");
    }

    void ClientSetEngineEntityTargetName(void* entity, const char* targetname)
    {
        MS_ANGEL_DEBUG("ClientSetEngineEntityTargetName: Client cannot modify entity targetnames");
    }

    void ClientSetEngineEntityHealth(void* entity, float health)
    {
        MS_ANGEL_DEBUG("ClientSetEngineEntityHealth: Client cannot modify entity health");
    }

    float ClientGetEngineEntityHealth(void* entity)
    {
        if (!entity) {
            MS_ANGEL_DEBUG("ClientGetEngineEntityHealth: NULL entity");
            return 0.0f;
        }
        
        // Client has limited entity information
        MS_ANGEL_DEBUG("ClientGetEngineEntityHealth: Limited client access");
        return 100.0f; // Default health
    }

    int ClientGetEngineEntityDeadFlag(void* entity)
    {
        if (!entity) {
            MS_ANGEL_DEBUG("ClientGetEngineEntityDeadFlag: NULL entity");
            return 1; // Dead
        }
        
        MS_ANGEL_DEBUG("ClientGetEngineEntityDeadFlag: Limited client access");
        return 0; // Alive by default
    }

    const char* ClientGetEngineEntityClassName(void* entity)
    {
        if (!entity) {
            MS_ANGEL_DEBUG("ClientGetEngineEntityClassName: NULL entity");
            return "";
        }
        
        MS_ANGEL_DEBUG("ClientGetEngineEntityClassName: Limited client access");
        return "info_target"; // Default class
    }

    void ClientGetEngineEntityOrigin(void* entity, float* x, float* y, float* z)
    {
        if (!entity) {
            MS_ANGEL_DEBUG("ClientGetEngineEntityOrigin: NULL entity");
            if (x) *x = 0.0f;
            if (y) *y = 0.0f;
            if (z) *z = 0.0f;
            return;
        }
        
        // Client has limited entity information
        MS_ANGEL_DEBUG("ClientGetEngineEntityOrigin: Limited client access");
        if (x) *x = 0.0f;
        if (y) *y = 0.0f;
        if (z) *z = 0.0f;
    }
}