//==========================================================================
// ASEngineProvider.cpp
// 
// Platform-agnostic provider that routes to correct engine implementation
// Uses compile-time detection to route to server or client implementations
//==========================================================================

#include "ASEngineInterface.h"
#include "mslogger.h"

// Platform-specific includes
#ifdef VALVE_DLL
    // Server-side implementation
    #include "../../server/angelscript/ASServerEngineImpl.h"
    #define ENGINE_IMPL ASServerEngine::GetInstance()
#elif defined(CLIENT_DLL)
    // Client-side implementation
    #include "../../client/angelscript/ASClientEngineImpl.h"
    #define ENGINE_IMPL ASClientEngine::GetInstance()
#else
    // Fallback for tools/shared builds
    #define ENGINE_IMPL (*static_cast<ASEngineInterface<int>*>(nullptr))
    #define NO_ENGINE_IMPL
#endif

//==========================================================================
// ASEngineProvider Implementation
//==========================================================================

float ASEngineProvider::GetGameTime()
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.GetGameTime();
#else
    MS_ANGEL_DEBUG("GetGameTime: No engine implementation available");
    return 0.0f;
#endif
}

std::string ASEngineProvider::GetCvarString(const std::string& name)
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.GetCvarString(name);
#else
    MS_ANGEL_DEBUG("GetCvarString: No engine implementation available for '%s'", name.c_str());
    return "";
#endif
}

std::string ASEngineProvider::GetMapName()
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.GetMapName();
#else
    MS_ANGEL_DEBUG("GetMapName: No engine implementation available");
    return "unknown_map";
#endif
}

void* ASEngineProvider::CreateEntity(const std::string& classname)
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.CreateEntity(classname);
#else
    MS_ANGEL_DEBUG("CreateEntity: No engine implementation available for '%s'", classname.c_str());
    return nullptr;
#endif
}

void ASEngineProvider::SetEntityOrigin(void* entity, const Vector& origin)
{
#ifndef NO_ENGINE_IMPL
    ENGINE_IMPL.SetEntityOrigin(entity, origin);
#else
    MS_ANGEL_DEBUG("SetEntityOrigin: No engine implementation available");
#endif
}

void ASEngineProvider::SetEntityName(void* entity, const std::string& name)
{
#ifndef NO_ENGINE_IMPL
    ENGINE_IMPL.SetEntityName(entity, name);
#else
    MS_ANGEL_DEBUG("SetEntityName: No engine implementation available");
#endif
}

void ASEngineProvider::SetEntityTargetName(void* entity, const std::string& targetname)
{
#ifndef NO_ENGINE_IMPL
    ENGINE_IMPL.SetEntityTargetName(entity, targetname);
#else
    MS_ANGEL_DEBUG("SetEntityTargetName: No engine implementation available");
#endif
}

void ASEngineProvider::SetEntityHealth(void* entity, float health)
{
#ifndef NO_ENGINE_IMPL
    ENGINE_IMPL.SetEntityHealth(entity, health);
#else
    MS_ANGEL_DEBUG("SetEntityHealth: No engine implementation available");
#endif
}

Vector ASEngineProvider::GetEntityOrigin(void* entity)
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.GetEntityOrigin(entity);
#else
    MS_ANGEL_DEBUG("GetEntityOrigin: No engine implementation available");
    return Vector(0, 0, 0);
#endif
}

float ASEngineProvider::GetEntityHealth(void* entity)
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.GetEntityHealth(entity);
#else
    MS_ANGEL_DEBUG("GetEntityHealth: No engine implementation available");
    return 100.0f;
#endif
}

int ASEngineProvider::GetEntityDeadFlag(void* entity)
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.GetEntityDeadFlag(entity);
#else
    MS_ANGEL_DEBUG("GetEntityDeadFlag: No engine implementation available");
    return 0; // DEAD_NO
#endif
}

std::string ASEngineProvider::GetEntityClassName(void* entity)
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.GetEntityClassName(entity);
#else
    MS_ANGEL_DEBUG("GetEntityClassName: No engine implementation available");
    return "unknown_entity";
#endif
}

std::string ASEngineProvider::GetPlayerAuthId(void* player)
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.GetPlayerAuthId(player);
#else
    MS_ANGEL_DEBUG("GetPlayerAuthId: No engine implementation available");
    return "STEAM_ID_INVALID";
#endif
}

std::string ASEngineProvider::GetPlayerDisplayName(void* player)
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.GetPlayerDisplayName(player);
#else
    MS_ANGEL_DEBUG("GetPlayerDisplayName: No engine implementation available");
    return "Unknown Player";
#endif
}

std::string ASEngineProvider::GetPlayerClientAddress(void* player)
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.GetPlayerClientAddress(player);
#else
    MS_ANGEL_DEBUG("GetPlayerClientAddress: No engine implementation available");
    return "0.0.0.0";
#endif
}

int ASEngineProvider::GetPlayerEntIndex(void* player)
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.GetPlayerEntIndex(player);
#else
    MS_ANGEL_DEBUG("GetPlayerEntIndex: No engine implementation available");
    return -1;
#endif
}

bool ASEngineProvider::IsValidPlayer(void* player)
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.IsValidPlayer(player);
#else
    MS_ANGEL_DEBUG("IsValidPlayer: No engine implementation available");
    return false;
#endif
}

void* ASEngineProvider::PlayerByIndex(int index)
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.PlayerByIndex(index);
#else
    MS_ANGEL_DEBUG("PlayerByIndex: No engine implementation available");
    return nullptr;
#endif
}

int ASEngineProvider::GetMaxClients()
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.GetMaxClients();
#else
    MS_ANGEL_DEBUG("GetMaxClients: No engine implementation available");
    return 0;
#endif
}

void ASEngineProvider::SendInfoMsg(void* player, const std::string& message)
{
#ifndef NO_ENGINE_IMPL
    ENGINE_IMPL.SendInfoMsg(player, message);
#else
    MS_ANGEL_DEBUG("SendInfoMsg: No engine implementation available");
#endif
}

void ASEngineProvider::EmitSound(void* entity, int channel, const std::string& sound, float volume, float attn, int flags, int pitch)
{
#ifndef NO_ENGINE_IMPL
    ENGINE_IMPL.EmitSound(entity, channel, sound, volume, attn, flags, pitch);
#else
    MS_ANGEL_DEBUG("EmitSound: No engine implementation available");
#endif
}

// Note: All C-style wrapper functions have been removed.
// The new template system uses ASEngineProvider directly through asbind20.
// If legacy C linkage is needed, it should be minimal and specific.