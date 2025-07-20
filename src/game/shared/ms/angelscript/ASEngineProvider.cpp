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

//==========================================================================
// Player Management Functions
//==========================================================================

int ASEngineProvider::GetPlayerCount()
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.GetPlayerCount();
#else
    MS_ANGEL_DEBUG("GetPlayerCount: No engine implementation available");
    return 0;
#endif
}

bool ASEngineProvider::IsPlayerConnected(void* player)
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.IsPlayerConnected(player);
#else
    MS_ANGEL_DEBUG("IsPlayerConnected: No engine implementation available");
    return false;
#endif
}

bool ASEngineProvider::IsPlayerAdmin(void* player)
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.IsPlayerAdmin(player);
#else
    MS_ANGEL_DEBUG("IsPlayerAdmin: No engine implementation available");
    return false;
#endif
}

//==========================================================================
// Master Sword Specific Player Functions
//==========================================================================

std::string ASEngineProvider::GetPlayerTitle(void* player)
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.GetPlayerTitle(player);
#else
    MS_ANGEL_DEBUG("GetPlayerTitle: No engine implementation available");
    return "Unknown";
#endif
}

float ASEngineProvider::GetPlayerMaxHP(void* player)
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.GetPlayerMaxHP(player);
#else
    MS_ANGEL_DEBUG("GetPlayerMaxHP: No engine implementation available");
    return 0.0f;
#endif
}

float ASEngineProvider::GetPlayerMaxMP(void* player)
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.GetPlayerMaxMP(player);
#else
    MS_ANGEL_DEBUG("GetPlayerMaxMP: No engine implementation available");
    return 0.0f;
#endif
}

bool ASEngineProvider::IsPlayerElite(void* player)
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.IsPlayerElite(player);
#else
    MS_ANGEL_DEBUG("IsPlayerElite: No engine implementation available");
    return false;
#endif
}

std::string ASEngineProvider::GetPlayerPartyName(void* player)
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.GetPlayerPartyName(player);
#else
    MS_ANGEL_DEBUG("GetPlayerPartyName: No engine implementation available");
    return "";
#endif
}

bool ASEngineProvider::IsPlayerLocalHost(void* player)
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.IsPlayerLocalHost(player);
#else
    MS_ANGEL_DEBUG("IsPlayerLocalHost: No engine implementation available");
    return false;
#endif
}

std::string ASEngineProvider::GetPlayerQuestData(const std::string& playerID, const std::string& key)
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.GetPlayerQuestData(playerID, key);
#else
    MS_ANGEL_DEBUG("GetPlayerQuestData: No engine implementation available");
    return "";
#endif
}

void ASEngineProvider::SetPlayerQuestData(const std::string& playerID, const std::string& key, const std::string& value)
{
#ifndef NO_ENGINE_IMPL
    ENGINE_IMPL.SetPlayerQuestData(playerID, key, value);
#else
    MS_ANGEL_DEBUG("SetPlayerQuestData: No engine implementation available");
#endif
}

//==========================================================================
// Server Management Functions
//==========================================================================

std::string ASEngineProvider::GetServerCVar(const std::string& name, const std::string& defaultValue)
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.GetServerCVar(name, defaultValue);
#else
    MS_ANGEL_DEBUG("GetServerCVar: No engine implementation available");
    return defaultValue;
#endif
}

bool ASEngineProvider::IsServerLocked()
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.IsServerLocked();
#else
    MS_ANGEL_DEBUG("IsServerLocked: No engine implementation available");
    return false;
#endif
}

std::string ASEngineProvider::GetServerPassword()
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.GetServerPassword();
#else
    MS_ANGEL_DEBUG("GetServerPassword: No engine implementation available");
    return "";
#endif
}

bool ASEngineProvider::IsServerPvpEnabled()
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.IsServerPvpEnabled();
#else
    MS_ANGEL_DEBUG("IsServerPvpEnabled: No engine implementation available");
    return false;
#endif
}

bool ASEngineProvider::IsOnFNServer()
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.IsOnFNServer();
#else
    MS_ANGEL_DEBUG("IsOnFNServer: No engine implementation available");
    return false;
#endif
}

bool ASEngineProvider::IsDeveloperMode()
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.IsDeveloperMode();
#else
    MS_ANGEL_DEBUG("IsDeveloperMode: No engine implementation available");
    return false;
#endif
}

void ASEngineProvider::ExecuteServerCommand(const std::string& command)
{
#ifndef NO_ENGINE_IMPL
    ENGINE_IMPL.ExecuteServerCommand(command);
#else
    MS_ANGEL_DEBUG("ExecuteServerCommand: No engine implementation available");
#endif
}

void ASEngineProvider::SetGlobalVariable(const std::string& name, const std::string& value)
{
#ifndef NO_ENGINE_IMPL
    ENGINE_IMPL.SetGlobalVariable(name, value);
#else
    MS_ANGEL_DEBUG("SetGlobalVariable: No engine implementation available");
#endif
}

bool ASEngineProvider::EngineMapExists(const std::string& mapName)
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.EngineMapExists(mapName);
#else
    MS_ANGEL_DEBUG("EngineMapExists: No engine implementation available");
    return false;
#endif
}

bool ASEngineProvider::MapExists(const std::string& mapName)
{
#ifndef NO_ENGINE_IMPL
    return ENGINE_IMPL.MapExists(mapName);
#else
    MS_ANGEL_DEBUG("MapExists: No engine implementation available");
    return false;
#endif
}

void ASEngineProvider::ScheduleDelayedEvent(float delay, const std::string& eventName)
{
#ifndef NO_ENGINE_IMPL
    ENGINE_IMPL.ScheduleDelayedEvent(delay, eventName);
#else
    MS_ANGEL_DEBUG("ScheduleDelayedEvent: No engine implementation available");
#endif
}

//==========================================================================
// Communication Functions
//==========================================================================

void ASEngineProvider::SendPlayerMessage(const std::string& playerID, const std::string& message)
{
#ifndef NO_ENGINE_IMPL
    ENGINE_IMPL.SendPlayerMessage(playerID, message);
#else
    MS_ANGEL_DEBUG("SendPlayerMessage: No engine implementation available");
#endif
}

void ASEngineProvider::SendConsoleMessage(const std::string& playerID, const std::string& message)
{
#ifndef NO_ENGINE_IMPL
    ENGINE_IMPL.SendConsoleMessage(playerID, message);
#else
    MS_ANGEL_DEBUG("SendConsoleMessage: No engine implementation available");
#endif
}

void ASEngineProvider::SendMessageToAllPlayers(const std::string& color, const std::string& message)
{
#ifndef NO_ENGINE_IMPL
    ENGINE_IMPL.SendMessageToAllPlayers(color, message);
#else
    MS_ANGEL_DEBUG("SendMessageToAllPlayers: No engine implementation available");
#endif
}

void ASEngineProvider::SendInfoMessageToAll(const std::string& title, const std::string& message)
{
#ifndef NO_ENGINE_IMPL
    ENGINE_IMPL.SendInfoMessageToAll(title, message);
#else
    MS_ANGEL_DEBUG("SendInfoMessageToAll: No engine implementation available");
#endif
}

// Note: All C-style wrapper functions have been removed.
// The new template system uses ASEngineProvider directly through asbind20.
// If legacy C linkage is needed, it should be minimal and specific.