//==========================================================================
// ASEngineStubs.cpp
// 
// Default stub implementations of engine integration functions
// These will be replaced by actual implementations in server-specific files
//==========================================================================

#include "mslogger.h"

// Forward declare the server implementation functions
#ifdef VALVE_DLL
// Server-side implementations (defined in ASServerEngine.cpp)
extern "C" {
    float ServerGetEngineGameTime();
    const char* ServerGetEngineCvarString(const char* name);
    const char* ServerGetEngineMapName();
    void* ServerCreateEngineEntity(const char* classname);
    void ServerSetEngineEntityOrigin(void* entity, float x, float y, float z);
    void ServerSetEngineEntityName(void* entity, const char* name);
    void ServerSetEngineEntityTargetName(void* entity, const char* targetname);
    void ServerSetEngineEntityHealth(void* entity, float health);
    float ServerGetEngineEntityHealth(void* entity);
    int ServerGetEngineEntityDeadFlag(void* entity);
    const char* ServerGetEngineEntityClassName(void* entity);
    void ServerGetEngineEntityOrigin(void* entity, float* x, float* y, float* z);
}
#endif

// Client-side implementations (defined in ASClientEngine.cpp)
#ifdef CLIENT_DLL
extern "C" {
    float ClientGetEngineGameTime();
    const char* ClientGetEngineCvarString(const char* name);
    const char* ClientGetEngineMapName();
    void* ClientCreateEngineEntity(const char* classname);
    void ClientSetEngineEntityOrigin(void* entity, float x, float y, float z);
    void ClientSetEngineEntityName(void* entity, const char* name);
    void ClientSetEngineEntityTargetName(void* entity, const char* targetname);
    void ClientSetEngineEntityHealth(void* entity, float health);
    float ClientGetEngineEntityHealth(void* entity);
    int ClientGetEngineEntityDeadFlag(void* entity);
    const char* ClientGetEngineEntityClassName(void* entity);
    void ClientGetEngineEntityOrigin(void* entity, float* x, float* y, float* z);
}
#endif

// Implementation router functions - route to server or client implementations
extern "C" {

    float GetEngineGameTime()
    {
#ifdef VALVE_DLL
        return ServerGetEngineGameTime();
#elif defined(CLIENT_DLL)
        return ClientGetEngineGameTime();
#else
        MS_ANGEL_DEBUG("GetEngineGameTime: No implementation available");
        return 0.0f;
#endif
    }

    const char* GetEngineCvarString(const char* name)
    {
#ifdef VALVE_DLL
        return ServerGetEngineCvarString(name);
#elif defined(CLIENT_DLL)
        return ClientGetEngineCvarString(name);
#else
        MS_ANGEL_DEBUG("GetEngineCvarString: No implementation available for '%s'", name ? name : "NULL");
        return "";
#endif
    }

    const char* GetEngineMapName()
    {
#ifdef VALVE_DLL
        return ServerGetEngineMapName();
#elif defined(CLIENT_DLL)
        return ClientGetEngineMapName();
#else
        MS_ANGEL_DEBUG("GetEngineMapName: No implementation available");
        return "unknown_map";
#endif
    }

    void* CreateEngineEntity(const char* classname)
    {
#ifdef VALVE_DLL
        return ServerCreateEngineEntity(classname);
#elif defined(CLIENT_DLL)
        return ClientCreateEngineEntity(classname);
#else
        MS_ANGEL_DEBUG("CreateEngineEntity: No implementation available for '%s'", classname ? classname : "NULL");
        return nullptr;
#endif
    }

    void SetEngineEntityOrigin(void* entity, float x, float y, float z)
    {
#ifdef VALVE_DLL
        ServerSetEngineEntityOrigin(entity, x, y, z);
#elif defined(CLIENT_DLL)
        ClientSetEngineEntityOrigin(entity, x, y, z);
#else
        MS_ANGEL_DEBUG("SetEngineEntityOrigin: No implementation available (%f, %f, %f)", x, y, z);
#endif
    }

    void SetEngineEntityName(void* entity, const char* name)
    {
#ifdef VALVE_DLL
        ServerSetEngineEntityName(entity, name);
#elif defined(CLIENT_DLL)
        ClientSetEngineEntityName(entity, name);
#else
        MS_ANGEL_DEBUG("SetEngineEntityName: No implementation available '%s'", name ? name : "NULL");
#endif
    }

    void SetEngineEntityTargetName(void* entity, const char* targetname)
    {
#ifdef VALVE_DLL
        ServerSetEngineEntityTargetName(entity, targetname);
#elif defined(CLIENT_DLL)
        ClientSetEngineEntityTargetName(entity, targetname);
#else
        MS_ANGEL_DEBUG("SetEngineEntityTargetName: No implementation available '%s'", targetname ? targetname : "NULL");
#endif
    }

    void SetEngineEntityHealth(void* entity, float health)
    {
#ifdef VALVE_DLL
        ServerSetEngineEntityHealth(entity, health);
#elif defined(CLIENT_DLL)
        ClientSetEngineEntityHealth(entity, health);
#else
        MS_ANGEL_DEBUG("SetEngineEntityHealth: No implementation available %f", health);
#endif
    }

    float GetEngineEntityHealth(void* entity)
    {
#ifdef VALVE_DLL
        return ServerGetEngineEntityHealth(entity);
#elif defined(CLIENT_DLL)
        return ClientGetEngineEntityHealth(entity);
#else
        MS_ANGEL_DEBUG("GetEngineEntityHealth: No implementation available");
        return 100.0f;
#endif
    }

    int GetEngineEntityDeadFlag(void* entity)
    {
#ifdef VALVE_DLL
        return ServerGetEngineEntityDeadFlag(entity);
#elif defined(CLIENT_DLL)
        return ClientGetEngineEntityDeadFlag(entity);
#else
        MS_ANGEL_DEBUG("GetEngineEntityDeadFlag: No implementation available");
        return 0; // DEAD_NO
#endif
    }

    const char* GetEngineEntityClassName(void* entity)
    {
#ifdef VALVE_DLL
        return ServerGetEngineEntityClassName(entity);
#elif defined(CLIENT_DLL)
        return ClientGetEngineEntityClassName(entity);
#else
        MS_ANGEL_DEBUG("GetEngineEntityClassName: No implementation available");
        return "unknown_entity";
#endif
    }

    void GetEngineEntityOrigin(void* entity, float* x, float* y, float* z)
    {
#ifdef VALVE_DLL
        ServerGetEngineEntityOrigin(entity, x, y, z);
#elif defined(CLIENT_DLL)
        ClientGetEngineEntityOrigin(entity, x, y, z);
#else
        MS_ANGEL_DEBUG("GetEngineEntityOrigin: No implementation available");
        if (x) *x = 0.0f;
        if (y) *y = 0.0f;
        if (z) *z = 0.0f;
#endif
    }
}