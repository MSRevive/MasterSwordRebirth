//==========================================================================
// ASServerEngineImpl.h
// 
// Server-side implementation of the AngelScript engine interface
// Provides actual game server integration for all engine functions
//==========================================================================

#pragma once

#include "ms/angelscript/ASEngineInterface.h"
#include <string>

//==========================================================================
// Server Engine Implementation
// Connects AngelScript calls to actual game server functionality
//==========================================================================
class ASServerEngine : public ASEngineInterface<ASServerEngine>
{
public:
    // Singleton access
    static ASServerEngine& GetInstance();
    
    // ASEngineInterface implementation
    float GetGameTimeImpl() const;
    std::string GetCvarStringImpl(const std::string& name) const;
    std::string GetMapNameImpl() const;
    void* CreateEntityImpl(const std::string& classname);
    void SetEntityOriginImpl(void* entity, const Vector& origin);
    void SetEntityNameImpl(void* entity, const std::string& name);
    void SetEntityTargetNameImpl(void* entity, const std::string& targetname);
    void SetEntityHealthImpl(void* entity, float health);
    Vector GetEntityOriginImpl(void* entity) const;
    float GetEntityHealthImpl(void* entity) const;
    int GetEntityDeadFlagImpl(void* entity) const;
    std::string GetEntityClassNameImpl(void* entity) const;
    std::string GetPlayerAuthIdImpl(void* player) const;
    std::string GetPlayerDisplayNameImpl(void* player) const;
    std::string GetPlayerClientAddressImpl(void* player) const;
    int GetPlayerEntIndexImpl(void* player) const;
    bool IsValidPlayerImpl(void* player) const;
    void* PlayerByIndexImpl(int index);
    int GetMaxClientsImpl() const;
    void SendInfoMsgImpl(void* player, const std::string& message);
    void EmitSoundImpl(void* entity, int channel, const std::string& sound, float volume, float attn, int flags, int pitch);
    
    // Communication function implementations
    void SendPlayerMessageImpl(const std::string& playerID, const std::string& message);
    void SendConsoleMessageImpl(const std::string& playerID, const std::string& message);
    void SendMessageToAllPlayersImpl(const std::string& color, const std::string& message);
    void SendInfoMessageToAllImpl(const std::string& title, const std::string& message);
    
    // Player management function implementations
    int GetPlayerCountImpl() const;
    bool IsPlayerConnectedImpl(void* player) const;
    bool IsPlayerAdminImpl(void* player) const;
    std::string GetPlayerQuestDataImpl(const std::string& playerID, const std::string& key) const;
    void SetPlayerQuestDataImpl(const std::string& playerID, const std::string& key, const std::string& value);
    
    // Server management function implementations
    std::string GetServerCVarImpl(const std::string& name, const std::string& defaultValue) const;
    bool IsServerLockedImpl() const;
    std::string GetServerPasswordImpl() const;
    bool IsServerPvpEnabledImpl() const;
    bool IsOnFNServerImpl() const;
    bool IsDeveloperModeImpl() const;
    void ExecuteServerCommandImpl(const std::string& command);
    void SetGlobalVariableImpl(const std::string& name, const std::string& value);
    bool EngineMapExistsImpl(const std::string& mapName) const;
    bool MapExistsImpl(const std::string& mapName) const;
    void ScheduleDelayedEventImpl(float delay, const std::string& eventName);

private:
    ASServerEngine() = default;
    ~ASServerEngine() = default;
    ASServerEngine(const ASServerEngine&) = delete;
    ASServerEngine& operator=(const ASServerEngine&) = delete;
    
    // Helper functions for type safety
    void* ValidateEntity(void* entity) const;
    void* ValidatePlayer(void* player) const;
    void* GetEdict(void* entity) const;
    
    // Player management helper functions
    void* FindPlayerBySteamID(const std::string& steamID) const;
    void* FindPlayerByName(const std::string& playerName) const;
    bool IsPlayerIdValid(const std::string& playerID) const;
};

// Note: All C-style function declarations have been removed.
// ASEntityBindings.cpp now uses ASEngineProvider directly.