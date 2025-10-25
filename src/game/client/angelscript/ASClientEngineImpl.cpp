//==========================================================================
// ASClientEngineImpl.cpp
// 
// Client-side implementation of the AngelScript engine interface
//==========================================================================

#include "ASClientEngineImpl.h"
#include "mslogger.h"
#include "hud.h"
#include "cl_util.h"

//==========================================================================
// Singleton Implementation
//==========================================================================

ASClientEngine& ASClientEngine::GetInstance()
{
    static ASClientEngine instance;
    return instance;
}

//==========================================================================
// Core Engine Function Implementations
//==========================================================================

float ASClientEngine::GetGameTimeImpl() const
{
    return gEngfuncs.GetClientTime();
}

std::string ASClientEngine::GetCvarStringImpl(const std::string& name) const
{
    const char* value = gEngfuncs.pfnGetCvarString(name.c_str());
    return value ? std::string(value) : "";
}

std::string ASClientEngine::GetMapNameImpl() const
{
    // Client doesn't have direct access to map name in the same way
    // This would need to be tracked separately or obtained from server
    MS_ANGEL_DEBUG("GetMapNameImpl: Client-side map name not readily available");
    return "client_unknown";
}

void* ASClientEngine::CreateEntityImpl(const std::string& classname)
{
    MS_ANGEL_DEBUG("CreateEntityImpl: Clients cannot create entities");
    return nullptr;
}

void ASClientEngine::SetEntityOriginImpl(void* entity, const Vector& origin)
{
    MS_ANGEL_DEBUG("SetEntityOriginImpl: Clients cannot modify entity origins");
}

void ASClientEngine::SetEntityNameImpl(void* entity, const std::string& name)
{
    MS_ANGEL_DEBUG("SetEntityNameImpl: Clients cannot set entity names");
}

void ASClientEngine::SetEntityTargetNameImpl(void* entity, const std::string& targetname)
{
    MS_ANGEL_DEBUG("SetEntityTargetNameImpl: Clients cannot set entity targetnames");
}

void ASClientEngine::SetEntityHealthImpl(void* entity, float health)
{
    MS_ANGEL_DEBUG("SetEntityHealthImpl: Clients cannot modify entity health");
}

Vector ASClientEngine::GetEntityOriginImpl(void* entity) const
{
    MS_ANGEL_DEBUG("GetEntityOriginImpl: Client-side entity access limited");
    return Vector(0, 0, 0);
}

float ASClientEngine::GetEntityHealthImpl(void* entity) const
{
    MS_ANGEL_DEBUG("GetEntityHealthImpl: Client-side entity access limited");
    return 0.0f;
}

int ASClientEngine::GetEntityDeadFlagImpl(void* entity) const
{
    MS_ANGEL_DEBUG("GetEntityDeadFlagImpl: Client-side entity access limited");
    return 0;
}

std::string ASClientEngine::GetEntityClassNameImpl(void* entity) const
{
    MS_ANGEL_DEBUG("GetEntityClassNameImpl: Client-side entity access limited");
    return "";
}

std::string ASClientEngine::GetPlayerAuthIdImpl(void* player) const
{
    MS_ANGEL_DEBUG("GetPlayerAuthIdImpl: Client-side player info limited");
    return "";
}

std::string ASClientEngine::GetPlayerDisplayNameImpl(void* player) const
{
    MS_ANGEL_DEBUG("GetPlayerDisplayNameImpl: Client-side player info limited");
    return "";
}

std::string ASClientEngine::GetPlayerClientAddressImpl(void* player) const
{
    MS_ANGEL_DEBUG("GetPlayerClientAddressImpl: Client-side player info limited");
    return "";
}

int ASClientEngine::GetPlayerEntIndexImpl(void* player) const
{
    MS_ANGEL_DEBUG("GetPlayerEntIndexImpl: Client-side player info limited");
    return -1;
}

bool ASClientEngine::IsValidPlayerImpl(void* player) const
{
    return player != nullptr;
}

void* ASClientEngine::PlayerByIndexImpl(int index)
{
    MS_ANGEL_DEBUG("PlayerByIndexImpl: Client-side player access limited");
    return nullptr;
}

int ASClientEngine::GetMaxClientsImpl() const
{
    return gEngfuncs.GetMaxClients();
}

void ASClientEngine::SendInfoMsgImpl(void* player, const std::string& message)
{
    // Display message in client HUD
    gHUD.m_TextMessage.BufferedLocaliseTextString(message.c_str());
}

void ASClientEngine::EmitSoundImpl(void* entity, int channel, const std::string& sound, float volume, float attn, int flags, int pitch)
{
    MS_ANGEL_DEBUG("EmitSoundImpl: Client-side sound emission limited");
}

//==========================================================================
// Player Management Function Implementations
//==========================================================================

int ASClientEngine::GetPlayerCountImpl() const
{
    MS_ANGEL_DEBUG("GetPlayerCountImpl: Client doesn't track player count");
    return 0;
}

bool ASClientEngine::IsPlayerConnectedImpl(void* player) const
{
    return player != nullptr;
}

bool ASClientEngine::IsPlayerAdminImpl(void* player) const
{
    MS_ANGEL_DEBUG("IsPlayerAdminImpl: Client doesn't have admin info");
    return false;
}

//==========================================================================
// Master Sword Specific Player Function Implementations
//==========================================================================

std::string ASClientEngine::GetPlayerTitleImpl(void* player) const
{
    MS_ANGEL_DEBUG("GetPlayerTitleImpl: Client-side access limited");
    return "";
}

float ASClientEngine::GetPlayerMaxHPImpl(void* player) const
{
    MS_ANGEL_DEBUG("GetPlayerMaxHPImpl: Client-side access limited");
    return 0.0f;
}

float ASClientEngine::GetPlayerMaxMPImpl(void* player) const
{
    MS_ANGEL_DEBUG("GetPlayerMaxMPImpl: Client-side access limited");
    return 0.0f;
}

bool ASClientEngine::IsPlayerEliteImpl(void* player) const
{
    MS_ANGEL_DEBUG("IsPlayerEliteImpl: Client-side access limited");
    return false;
}

std::string ASClientEngine::GetPlayerPartyNameImpl(void* player) const
{
    MS_ANGEL_DEBUG("GetPlayerPartyNameImpl: Client-side access limited");
    return "";
}

bool ASClientEngine::IsPlayerLocalHostImpl(void* player) const
{
    MS_ANGEL_DEBUG("IsPlayerLocalHostImpl: Client-side check limited");
    return false;
}

std::string ASClientEngine::GetPlayerQuestDataImpl(const std::string& playerID, const std::string& key) const
{
    MS_ANGEL_DEBUG("GetPlayerQuestDataImpl: Client doesn't store quest data");
    return "";
}

void ASClientEngine::SetPlayerQuestDataImpl(const std::string& playerID, const std::string& key, const std::string& value)
{
    MS_ANGEL_DEBUG("SetPlayerQuestDataImpl: Client cannot modify quest data");
}

//==========================================================================
// Server Management Function Implementations
//==========================================================================

std::string ASClientEngine::GetServerCVarImpl(const std::string& name, const std::string& defaultValue) const
{
    const char* value = gEngfuncs.pfnGetCvarString(name.c_str());
    return value ? std::string(value) : defaultValue;
}

bool ASClientEngine::IsServerLockedImpl() const
{
    MS_ANGEL_DEBUG("IsServerLockedImpl: Client doesn't have server lock info");
    return false;
}

std::string ASClientEngine::GetServerPasswordImpl() const
{
    MS_ANGEL_DEBUG("GetServerPasswordImpl: Client doesn't have password info");
    return "";
}

bool ASClientEngine::IsServerPvpEnabledImpl() const
{
    MS_ANGEL_DEBUG("IsServerPvpEnabledImpl: Client doesn't have PVP info");
    return false;
}

bool ASClientEngine::IsOnFNServerImpl() const
{
    MS_ANGEL_DEBUG("IsOnFNServerImpl: Client doesn't have FN server info");
    return false;
}

bool ASClientEngine::IsDeveloperModeImpl() const
{
    const char* devValue = gEngfuncs.pfnGetCvarString("developer");
    return devValue && atoi(devValue) > 0;
}

void ASClientEngine::ExecuteServerCommandImpl(const std::string& command)
{
    MS_ANGEL_DEBUG("ExecuteServerCommandImpl: Clients cannot execute server commands");
}

void ASClientEngine::SetGlobalVariableImpl(const std::string& name, const std::string& value)
{
    MS_ANGEL_DEBUG("SetGlobalVariableImpl: Clients cannot set global variables");
}

bool ASClientEngine::EngineMapExistsImpl(const std::string& mapName) const
{
    MS_ANGEL_DEBUG("EngineMapExistsImpl: Client-side map existence check limited");
    return false;
}

bool ASClientEngine::MapExistsImpl(const std::string& mapName) const
{
    MS_ANGEL_DEBUG("MapExistsImpl: Client-side map existence check limited");
    return false;
}

void ASClientEngine::ScheduleDelayedEventImpl(float delay, const std::string& eventName)
{
    MS_ANGEL_DEBUG("ScheduleDelayedEventImpl: Client-side event scheduling limited");
}

//==========================================================================
// Helper Functions
//==========================================================================

void* ASClientEngine::ValidateClientEntity(void* entity) const
{
    // Basic validation - in a real implementation this would check if the entity is valid
    return entity;
}

//==========================================================================
// Communication Function Implementations (Client-Side Stubs)
//==========================================================================

void ASClientEngine::SendPlayerMessageImpl(const std::string& playerID, const std::string& message)
{
    // For client, if the playerID is empty or refers to the local player, send the message to self
    if (playerID.empty() || message.empty()) {
        MS_ANGEL_DEBUG("SendPlayerMessageImpl: Empty playerID or message");
        return;
    }
    
    // Send message to client console (this client can only receive messages for itself)
    ConsolePrint("[MESSAGE] ");
    ConsolePrint(message.c_str());
    ConsolePrint("\n");
    
    MS_ANGEL_DEBUG("SendPlayerMessageImpl: Sent message to client: %s", message.c_str());
}

void ASClientEngine::SendConsoleMessageImpl(const std::string& playerID, const std::string& message)
{
    MS_ANGEL_DEBUG("SendConsoleMessageImpl: Client cannot send console messages to other players");
}

void ASClientEngine::SendMessageToAllPlayersImpl(const std::string& color, const std::string& message)
{
    MS_ANGEL_DEBUG("SendMessageToAllPlayersImpl: Client cannot send messages to all players");
}

void ASClientEngine::SendInfoMessageToAllImpl(const std::string& title, const std::string& message)
{
    MS_ANGEL_DEBUG("SendInfoMessageToAllImpl: Client cannot send info messages to all players");
}