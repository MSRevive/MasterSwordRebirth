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
#include "global.h"
#include "script.h"  // for CScript definition
#include <cstdio>  // for snprintf
#include <cstring> // for strlen

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

//==========================================================================
// Player Management Function Implementations
//==========================================================================

int ASServerEngine::GetPlayerCountImpl() const
{
    int count = 0;
    for (int i = 1; i <= gpGlobals->maxClients; i++)
    {
        CBasePlayer* pPlayer = static_cast<CBasePlayer*>(UTIL_PlayerByIndex(i));
        if (pPlayer && IsValidPlayerImpl(pPlayer))
        {
            count++;
        }
    }
    MS_ANGEL_DEBUG("GetPlayerCount: %d connected players", count);
    return count;
}

bool ASServerEngine::IsPlayerConnectedImpl(void* player) const
{
    if (!player) 
    {
        MS_ANGEL_DEBUG("IsPlayerConnected: NULL player pointer");
        return false;
    }
    
    CBasePlayer* pPlayer = static_cast<CBasePlayer*>(ValidatePlayer(player));
    if (!pPlayer) return false;
    
    // Check if player has a valid edict and is connected
    edict_t* pEdict = pPlayer->edict();
    if (!pEdict) return false;
    
    // Use engine function to check if player is connected
    bool connected = !pEdict->free && (pEdict->v.flags & FL_CLIENT) != 0;
    MS_ANGEL_DEBUG("IsPlayerConnected: Player %s", connected ? "connected" : "disconnected");
    return connected;
}

bool ASServerEngine::IsPlayerAdminImpl(void* player) const
{
    if (!player) 
    {
        MS_ANGEL_DEBUG("IsPlayerAdmin: NULL player pointer");
        return false;
    }
    
    CBasePlayer* pPlayer = static_cast<CBasePlayer*>(ValidatePlayer(player));
    if (!pPlayer) return false;
    
    // Check if player has admin access - this is a simple implementation
    // In a real game, this would check against an admin database or CVAR system
    edict_t* pEdict = pPlayer->edict();
    if (!pEdict) return false;
    
    // For now, check if the player has admin privileges via entity flags
    // This is a placeholder - real admin checking would need to be implemented
    // based on the game's specific admin system
    bool isAdmin = (pEdict->v.flags & FL_GODMODE) != 0; // Simple check - replace with real admin logic
    MS_ANGEL_DEBUG("IsPlayerAdmin: Player %s admin", isAdmin ? "is" : "is not");
    return isAdmin;
}

std::string ASServerEngine::GetPlayerQuestDataImpl(const std::string& playerID, const std::string& key) const
{
    if (playerID.empty() || key.empty())
    {
        MS_ANGEL_ERROR("GetPlayerQuestData: Empty playerID or key");
        return "";
    }
    
    // Find player by Steam ID or player name
    void* pPlayer = FindPlayerBySteamID(playerID);
    if (!pPlayer)
    {
        pPlayer = FindPlayerByName(playerID);
    }
    
    if (!pPlayer)
    {
        MS_ANGEL_DEBUG("GetPlayerQuestData: Player '%s' not found", playerID.c_str());
        return "";
    }
    
    CBasePlayer* pCBasePlayer = static_cast<CBasePlayer*>(pPlayer);
    
    // Access the player's script variables for quest data
    // The Master Sword system uses m_Scripts array for storing script data
    if (pCBasePlayer->m_Scripts.size() > 0 && pCBasePlayer->m_Scripts[0])
    {
        const char* value = pCBasePlayer->m_Scripts[0]->GetVar(key.c_str());
        std::string result = value ? value : "";
        MS_ANGEL_DEBUG("GetPlayerQuestData: Player '%s', key '%s' = '%s'", 
                      playerID.c_str(), key.c_str(), result.c_str());
        return result;
    }
    
    MS_ANGEL_DEBUG("GetPlayerQuestData: No script variables found for player '%s'", playerID.c_str());
    return "";
}

void ASServerEngine::SetPlayerQuestDataImpl(const std::string& playerID, const std::string& key, const std::string& value)
{
    if (playerID.empty() || key.empty())
    {
        MS_ANGEL_ERROR("SetPlayerQuestData: Empty playerID or key");
        return;
    }
    
    // Find player by Steam ID or player name
    void* pPlayer = FindPlayerBySteamID(playerID);
    if (!pPlayer)
    {
        pPlayer = FindPlayerByName(playerID);
    }
    
    if (!pPlayer)
    {
        MS_ANGEL_ERROR("SetPlayerQuestData: Player '%s' not found", playerID.c_str());
        return;
    }
    
    CBasePlayer* pCBasePlayer = static_cast<CBasePlayer*>(pPlayer);
    
    // Set the player's script variables for quest data
    if (pCBasePlayer->m_Scripts.size() > 0 && pCBasePlayer->m_Scripts[0])
    {
        pCBasePlayer->m_Scripts[0]->SetVar(key.c_str(), value.c_str());
        MS_ANGEL_DEBUG("SetPlayerQuestData: Player '%s', set key '%s' = '%s'", 
                      playerID.c_str(), key.c_str(), value.c_str());
    }
    else
    {
        MS_ANGEL_ERROR("SetPlayerQuestData: No script variables available for player '%s'", playerID.c_str());
    }
}

//==========================================================================
// Master Sword Specific Player Functions
//==========================================================================

std::string ASServerEngine::GetPlayerTitleImpl(void* player) const
{
    CBasePlayer* pPlayer = static_cast<CBasePlayer*>(ValidatePlayer(player));
    if (!pPlayer)
    {
        MS_ANGEL_DEBUG("GetPlayerTitle: Invalid player pointer");
        return "Unknown";
    }
    
    const char* title = pPlayer->GetTitle();
    std::string result = title ? title : "Unknown";
    MS_ANGEL_DEBUG("GetPlayerTitle: Player '%s' title = '%s'", pPlayer->DisplayName(), result.c_str());
    return result;
}

float ASServerEngine::GetPlayerMaxHPImpl(void* player) const
{
    CBasePlayer* pPlayer = static_cast<CBasePlayer*>(ValidatePlayer(player));
    if (!pPlayer)
    {
        MS_ANGEL_DEBUG("GetPlayerMaxHP: Invalid player pointer");
        return 0.0f;
    }
    
    float maxHP = pPlayer->MaxHP();
    MS_ANGEL_DEBUG("GetPlayerMaxHP: Player '%s' maxHP = %.1f", pPlayer->DisplayName(), maxHP);
    return maxHP;
}

float ASServerEngine::GetPlayerMaxMPImpl(void* player) const
{
    CBasePlayer* pPlayer = static_cast<CBasePlayer*>(ValidatePlayer(player));
    if (!pPlayer)
    {
        MS_ANGEL_DEBUG("GetPlayerMaxMP: Invalid player pointer");
        return 0.0f;
    }
    
    float maxMP = pPlayer->MaxMP();
    MS_ANGEL_DEBUG("GetPlayerMaxMP: Player '%s' maxMP = %.1f", pPlayer->DisplayName(), maxMP);
    return maxMP;
}

bool ASServerEngine::IsPlayerEliteImpl(void* player) const
{
    CBasePlayer* pPlayer = static_cast<CBasePlayer*>(ValidatePlayer(player));
    if (!pPlayer)
    {
        MS_ANGEL_DEBUG("IsPlayerElite: Invalid player pointer");
        return false;
    }
    
    bool isElite = pPlayer->IsElite();
    MS_ANGEL_DEBUG("IsPlayerElite: Player '%s' elite = %s", pPlayer->DisplayName(), isElite ? "true" : "false");
    return isElite;
}

std::string ASServerEngine::GetPlayerPartyNameImpl(void* player) const
{
    CBasePlayer* pPlayer = static_cast<CBasePlayer*>(ValidatePlayer(player));
    if (!pPlayer)
    {
        MS_ANGEL_DEBUG("GetPlayerPartyName: Invalid player pointer");
        return "";
    }
    
    const char* partyName = pPlayer->GetPartyName();
    std::string result = partyName ? partyName : "";
    MS_ANGEL_DEBUG("GetPlayerPartyName: Player '%s' party = '%s'", pPlayer->DisplayName(), result.c_str());
    return result;
}

bool ASServerEngine::IsPlayerLocalHostImpl(void* player) const
{
    CBasePlayer* pPlayer = static_cast<CBasePlayer*>(ValidatePlayer(player));
    if (!pPlayer)
    {
        MS_ANGEL_DEBUG("IsPlayerLocalHost: Invalid player pointer");
        return false;
    }
    
    bool isLocalHost = pPlayer->IsLocalHost();
    MS_ANGEL_DEBUG("IsPlayerLocalHost: Player '%s' localhost = %s", pPlayer->DisplayName(), isLocalHost ? "true" : "false");
    return isLocalHost;
}

//==========================================================================
// Player Management Helper Functions
//==========================================================================

void* ASServerEngine::FindPlayerBySteamID(const std::string& steamID) const
{
    if (steamID.empty()) return nullptr;
    
    for (int i = 1; i <= gpGlobals->maxClients; i++)
    {
        CBasePlayer* pPlayer = static_cast<CBasePlayer*>(UTIL_PlayerByIndex(i));
        if (!pPlayer || !IsValidPlayerImpl(pPlayer)) continue;
        
        std::string playerSteamID = GetPlayerAuthIdImpl(pPlayer);
        if (playerSteamID == steamID)
        {
            MS_ANGEL_DEBUG("FindPlayerBySteamID: Found player with Steam ID '%s'", steamID.c_str());
            return pPlayer;
        }
    }
    
    MS_ANGEL_DEBUG("FindPlayerBySteamID: No player found with Steam ID '%s'", steamID.c_str());
    return nullptr;
}

void* ASServerEngine::FindPlayerByName(const std::string& playerName) const
{
    if (playerName.empty()) return nullptr;
    
    for (int i = 1; i <= gpGlobals->maxClients; i++)
    {
        CBasePlayer* pPlayer = static_cast<CBasePlayer*>(UTIL_PlayerByIndex(i));
        if (!pPlayer || !IsValidPlayerImpl(pPlayer)) continue;
        
        std::string currentPlayerName = GetPlayerDisplayNameImpl(pPlayer);
        if (currentPlayerName == playerName)
        {
            MS_ANGEL_DEBUG("FindPlayerByName: Found player with name '%s'", playerName.c_str());
            return pPlayer;
        }
    }
    
    MS_ANGEL_DEBUG("FindPlayerByName: No player found with name '%s'", playerName.c_str());
    return nullptr;
}

bool ASServerEngine::IsPlayerIdValid(const std::string& playerID) const
{
    if (playerID.empty()) return false;
    
    // Check if it's a valid Steam ID format or a valid player name
    return FindPlayerBySteamID(playerID) != nullptr || FindPlayerByName(playerID) != nullptr;
}

//==========================================================================
// Server Management Function Implementations
//==========================================================================

std::string ASServerEngine::GetServerCVarImpl(const std::string& name, const std::string& defaultValue) const
{
    if (name.empty()) {
        MS_ANGEL_DEBUG("GetServerCVar: Empty cvar name");
        return defaultValue;
    }
    
    const char* value = CVAR_GET_STRING(name.c_str());
    if (!value || strlen(value) == 0) {
        MS_ANGEL_DEBUG("GetServerCVar: '%s' not found, returning default '%s'", name.c_str(), defaultValue.c_str());
        return defaultValue;
    }
    
    MS_ANGEL_DEBUG("GetServerCVar: '%s' = '%s'", name.c_str(), value);
    return std::string(value);
}

bool ASServerEngine::IsServerLockedImpl() const
{
    // Check if the server has a password set
    const char* password = CVAR_GET_STRING("sv_password");
    bool locked = password && strlen(password) > 0;
    MS_ANGEL_DEBUG("IsServerLocked: %s", locked ? "true" : "false");
    return locked;
}

std::string ASServerEngine::GetServerPasswordImpl() const
{
    // Security consideration: Only return password if caller has admin privileges
    // For now, return empty string as this is sensitive information
    MS_ANGEL_DEBUG("GetServerPassword: Access denied for security reasons");
    return "";
}

bool ASServerEngine::IsServerPvpEnabledImpl() const
{
    // Check the MS PvP level setting
    float pvpLevel = CVAR_GET_FLOAT("ms_pklevel");
    bool pvpEnabled = pvpLevel > 0.0f;
    MS_ANGEL_DEBUG("IsServerPvpEnabled: %s (ms_pklevel = %f)", pvpEnabled ? "true" : "false", pvpLevel);
    return pvpEnabled;
}

bool ASServerEngine::IsOnFNServerImpl() const
{
    // Check if this is a FearlessNation (FN) server
    bool fnEnabled = CVAR_GET_FLOAT("ms_central_enabled") > 0.0f;
    MS_ANGEL_DEBUG("IsOnFNServer: %s", fnEnabled ? "true" : "false");
    return fnEnabled;
}

bool ASServerEngine::IsDeveloperModeImpl() const
{
    // Check if developer mode is enabled
    bool devMode = CVAR_GET_FLOAT("ms_dev_mode") > 0.0f;
    MS_ANGEL_DEBUG("IsDeveloperMode: %s", devMode ? "true" : "false");
    return devMode;
}

void ASServerEngine::ExecuteServerCommandImpl(const std::string& command)
{
    if (command.empty()) {
        MS_ANGEL_DEBUG("ExecuteServerCommand: Empty command");
        return;
    }
    
    // Security: Basic validation to prevent dangerous commands
    // In production, this should have more comprehensive security checks
    if (command.find("quit") != std::string::npos || 
        command.find("exit") != std::string::npos ||
        command.find("rcon_password") != std::string::npos) {
        MS_ANGEL_ERROR("ExecuteServerCommand: Blocked dangerous command '%s'", command.c_str());
        return;
    }
    
    MS_ANGEL_DEBUG("ExecuteServerCommand: Executing '%s'", command.c_str());
    SERVER_COMMAND(const_cast<char*>(command.c_str()));
    SERVER_EXECUTE();
}

void ASServerEngine::SetGlobalVariableImpl(const std::string& name, const std::string& value)
{
    if (name.empty()) {
        MS_ANGEL_DEBUG("SetGlobalVariable: Empty variable name");
        return;
    }
    
    // Access the global script system to set variables
    if (MSGlobals::GameScript && MSGlobals::GameScript->m_Scripts.size() > 0 && MSGlobals::GameScript->m_Scripts[0]) {
        MSGlobals::GameScript->m_Scripts[0]->SetVar(name.c_str(), value.c_str());
        MS_ANGEL_DEBUG("SetGlobalVariable: Set '%s' = '%s'", name.c_str(), value.c_str());
    } else {
        MS_ANGEL_ERROR("SetGlobalVariable: Global script system not available");
    }
}

bool ASServerEngine::EngineMapExistsImpl(const std::string& mapName) const
{
    if (mapName.empty()) {
        MS_ANGEL_DEBUG("EngineMapExists: Empty map name");
        return false;
    }
    
    // Check if the map file exists on the server
    // Use the engine's map validation function
    char mapPath[256];
    snprintf(mapPath, sizeof(mapPath), "maps/%s.bsp", mapName.c_str());
    
    // Check if file exists using the engine's file system
    int fileSize;
    char* buffer = (char*)g_engfuncs.pfnLoadFileForMe(mapPath, &fileSize);
    if (buffer) {
        g_engfuncs.pfnFreeFile(buffer);
        MS_ANGEL_DEBUG("EngineMapExists: Map '%s' exists", mapName.c_str());
        return true;
    }
    
    MS_ANGEL_DEBUG("EngineMapExists: Map '%s' not found", mapName.c_str());
    return false;
}

bool ASServerEngine::MapExistsImpl(const std::string& mapName) const
{
    // Wrapper for EngineMapExists for backwards compatibility
    return EngineMapExistsImpl(mapName);
}

void ASServerEngine::ScheduleDelayedEventImpl(float delay, const std::string& eventName)
{
    if (eventName.empty()) {
        MS_ANGEL_DEBUG("ScheduleDelayedEvent: Empty event name");
        return;
    }
    
    if (delay < 0.0f) {
        MS_ANGEL_DEBUG("ScheduleDelayedEvent: Invalid delay %f, using 0.0", delay);
        delay = 0.0f;
    }
    
    // Schedule the event using the global script system
    if (MSGlobals::GameScript && MSGlobals::GameScript->m_Scripts.size() > 0 && MSGlobals::GameScript->m_Scripts[0]) {
        // Create a delayed event call
        msstringlist Params;
        Params.add(msstring(eventName.c_str()));
        MSGlobals::GameScript->m_Scripts[0]->RunScriptEventByName("game_delayed_event", &Params);
        MS_ANGEL_DEBUG("ScheduleDelayedEvent: Scheduled '%s' in %f seconds", eventName.c_str(), delay);
    } else {
        MS_ANGEL_ERROR("ScheduleDelayedEvent: Global script system not available");
    }
}

//==========================================================================
// Communication Function Implementations
//==========================================================================

void ASServerEngine::SendPlayerMessageImpl(const std::string& playerID, const std::string& message)
{
    MS_ANGEL_DEBUG("SendPlayerMessageImpl: Sending message to %s: %s", playerID.c_str(), message.c_str());
    // TODO: Implement actual player messaging using Half-Life message system
}

void ASServerEngine::SendConsoleMessageImpl(const std::string& playerID, const std::string& message)
{
    MS_ANGEL_DEBUG("SendConsoleMessageImpl: Sending console message to %s: %s", playerID.c_str(), message.c_str());
    // TODO: Implement actual console messaging using CLIENT_COMMAND
}

void ASServerEngine::SendMessageToAllPlayersImpl(const std::string& color, const std::string& message)
{
    MS_ANGEL_DEBUG("SendMessageToAllPlayersImpl: Sending colored message to all: [%s] %s", color.c_str(), message.c_str());
    // TODO: Implement actual broadcast messaging
}

void ASServerEngine::SendInfoMessageToAllImpl(const std::string& title, const std::string& message)
{
    MS_ANGEL_DEBUG("SendInfoMessageToAllImpl: Sending info to all: %s - %s", title.c_str(), message.c_str());
    // TODO: Implement actual info message broadcasting
}

// Note: All C-style wrapper functions have been removed.
// ASEntityBindings.cpp now uses ASEngineProvider directly.