//==========================================================================
// ASEngineInterface.h
// 
// Template-based AngelScript engine interface system
// Provides clean separation between client/server implementations
// Uses CRTP for compile-time polymorphism and type safety
//==========================================================================

#pragma once

#include <string>

// Forward declarations
typedef float vec_t;
#include "hl/vector.h"

//==========================================================================
// Core Engine Interface Template
// Use CRTP (Curiously Recurring Template Pattern) for compile-time dispatch
//==========================================================================
template<typename Derived>
class ASEngineInterface
{
public:
    // Game state functions
    float GetGameTime() const 
    {
        return static_cast<const Derived*>(this)->GetGameTimeImpl();
    }
    
    std::string GetCvarString(const std::string& name) const
    {
        return static_cast<const Derived*>(this)->GetCvarStringImpl(name);
    }
    
    std::string GetMapName() const
    {
        return static_cast<const Derived*>(this)->GetMapNameImpl();
    }
    
    // Entity management functions
    void* CreateEntity(const std::string& classname)
    {
        return static_cast<Derived*>(this)->CreateEntityImpl(classname);
    }
    
    void SetEntityOrigin(void* entity, const Vector& origin)
    {
        static_cast<Derived*>(this)->SetEntityOriginImpl(entity, origin);
    }
    
    void SetEntityName(void* entity, const std::string& name)
    {
        static_cast<Derived*>(this)->SetEntityNameImpl(entity, name);
    }
    
    void SetEntityTargetName(void* entity, const std::string& targetname)
    {
        static_cast<Derived*>(this)->SetEntityTargetNameImpl(entity, targetname);
    }
    
    void SetEntityHealth(void* entity, float health)
    {
        static_cast<Derived*>(this)->SetEntityHealthImpl(entity, health);
    }
    
    Vector GetEntityOrigin(void* entity) const
    {
        return static_cast<const Derived*>(this)->GetEntityOriginImpl(entity);
    }
    
    float GetEntityHealth(void* entity) const
    {
        return static_cast<const Derived*>(this)->GetEntityHealthImpl(entity);
    }
    
    int GetEntityDeadFlag(void* entity) const
    {
        return static_cast<const Derived*>(this)->GetEntityDeadFlagImpl(entity);
    }
    
    std::string GetEntityClassName(void* entity) const
    {
        return static_cast<const Derived*>(this)->GetEntityClassNameImpl(entity);
    }
    
    // Player-specific functions
    std::string GetPlayerAuthId(void* player) const
    {
        return static_cast<const Derived*>(this)->GetPlayerAuthIdImpl(player);
    }
    
    std::string GetPlayerDisplayName(void* player) const
    {
        return static_cast<const Derived*>(this)->GetPlayerDisplayNameImpl(player);
    }
    
    std::string GetPlayerClientAddress(void* player) const
    {
        return static_cast<const Derived*>(this)->GetPlayerClientAddressImpl(player);
    }
    
    int GetPlayerEntIndex(void* player) const
    {
        return static_cast<const Derived*>(this)->GetPlayerEntIndexImpl(player);
    }
    
    bool IsValidPlayer(void* player) const
    {
        return static_cast<const Derived*>(this)->IsValidPlayerImpl(player);
    }
    
    void* PlayerByIndex(int index)
    {
        return static_cast<Derived*>(this)->PlayerByIndexImpl(index);
    }
    
    int GetMaxClients() const
    {
        return static_cast<const Derived*>(this)->GetMaxClientsImpl();
    }
    
    void SendInfoMsg(void* player, const std::string& message)
    {
        static_cast<Derived*>(this)->SendInfoMsgImpl(player, message);
    }
    
    // Communication functions for GameMaster
    void SendPlayerMessage(const std::string& playerID, const std::string& message)
    {
        static_cast<Derived*>(this)->SendPlayerMessageImpl(playerID, message);
    }
    
    void SendConsoleMessage(const std::string& playerID, const std::string& message)
    {
        static_cast<Derived*>(this)->SendConsoleMessageImpl(playerID, message);
    }
    
    void SendMessageToAllPlayers(const std::string& color, const std::string& message)
    {
        static_cast<Derived*>(this)->SendMessageToAllPlayersImpl(color, message);
    }
    
    void SendInfoMessageToAll(const std::string& title, const std::string& message)
    {
        static_cast<Derived*>(this)->SendInfoMessageToAllImpl(title, message);
    }
    
    void EmitSound(void* entity, int channel, const std::string& sound, float volume, float attn, int flags, int pitch)
    {
        static_cast<Derived*>(this)->EmitSoundImpl(entity, channel, sound, volume, attn, flags, pitch);
    }
    
    // Player management functions
    int GetPlayerCount() const
    {
        return static_cast<const Derived*>(this)->GetPlayerCountImpl();
    }
    
    bool IsPlayerConnected(void* player) const
    {
        return static_cast<const Derived*>(this)->IsPlayerConnectedImpl(player);
    }
    
    bool IsPlayerAdmin(void* player) const
    {
        return static_cast<const Derived*>(this)->IsPlayerAdminImpl(player);
    }
    
    std::string GetPlayerQuestData(const std::string& playerID, const std::string& key) const
    {
        return static_cast<const Derived*>(this)->GetPlayerQuestDataImpl(playerID, key);
    }
    
    void SetPlayerQuestData(const std::string& playerID, const std::string& key, const std::string& value)
    {
        static_cast<Derived*>(this)->SetPlayerQuestDataImpl(playerID, key, value);
    }
    
    // Server management functions
    std::string GetServerCVar(const std::string& name, const std::string& defaultValue) const
    {
        return static_cast<const Derived*>(this)->GetServerCVarImpl(name, defaultValue);
    }
    
    bool IsServerLocked() const
    {
        return static_cast<const Derived*>(this)->IsServerLockedImpl();
    }
    
    std::string GetServerPassword() const
    {
        return static_cast<const Derived*>(this)->GetServerPasswordImpl();
    }
    
    bool IsServerPvpEnabled() const
    {
        return static_cast<const Derived*>(this)->IsServerPvpEnabledImpl();
    }
    
    bool IsOnFNServer() const
    {
        return static_cast<const Derived*>(this)->IsOnFNServerImpl();
    }
    
    bool IsDeveloperMode() const
    {
        return static_cast<const Derived*>(this)->IsDeveloperModeImpl();
    }
    
    void ExecuteServerCommand(const std::string& command)
    {
        static_cast<Derived*>(this)->ExecuteServerCommandImpl(command);
    }
    
    void SetGlobalVariable(const std::string& name, const std::string& value)
    {
        static_cast<Derived*>(this)->SetGlobalVariableImpl(name, value);
    }
    
    bool EngineMapExists(const std::string& mapName) const
    {
        return static_cast<const Derived*>(this)->EngineMapExistsImpl(mapName);
    }
    
    bool MapExists(const std::string& mapName) const
    {
        return static_cast<const Derived*>(this)->MapExistsImpl(mapName);
    }
    
    void ScheduleDelayedEvent(float delay, const std::string& eventName)
    {
        static_cast<Derived*>(this)->ScheduleDelayedEventImpl(delay, eventName);
    }

protected:
    // Default implementations that derived classes can override
    virtual float GetGameTimeImpl() const = 0;
    virtual std::string GetCvarStringImpl(const std::string& name) const = 0;
    virtual std::string GetMapNameImpl() const = 0;
    virtual void* CreateEntityImpl(const std::string& classname) = 0;
    virtual void SetEntityOriginImpl(void* entity, const Vector& origin) = 0;
    virtual void SetEntityNameImpl(void* entity, const std::string& name) = 0;
    virtual void SetEntityTargetNameImpl(void* entity, const std::string& targetname) = 0;
    virtual void SetEntityHealthImpl(void* entity, float health) = 0;
    virtual Vector GetEntityOriginImpl(void* entity) const = 0;
    virtual float GetEntityHealthImpl(void* entity) const = 0;
    virtual int GetEntityDeadFlagImpl(void* entity) const = 0;
    virtual std::string GetEntityClassNameImpl(void* entity) const = 0;
    virtual std::string GetPlayerAuthIdImpl(void* player) const = 0;
    virtual std::string GetPlayerDisplayNameImpl(void* player) const = 0;
    virtual std::string GetPlayerClientAddressImpl(void* player) const = 0;
    virtual int GetPlayerEntIndexImpl(void* player) const = 0;
    virtual bool IsValidPlayerImpl(void* player) const = 0;
    virtual void* PlayerByIndexImpl(int index) = 0;
    virtual int GetMaxClientsImpl() const = 0;
    virtual void SendInfoMsgImpl(void* player, const std::string& message) = 0;
    virtual void EmitSoundImpl(void* entity, int channel, const std::string& sound, float volume, float attn, int flags, int pitch) = 0;
    
    // Player management function implementations
    virtual int GetPlayerCountImpl() const = 0;
    virtual bool IsPlayerConnectedImpl(void* player) const = 0;
    virtual bool IsPlayerAdminImpl(void* player) const = 0;
    virtual std::string GetPlayerQuestDataImpl(const std::string& playerID, const std::string& key) const = 0;
    virtual void SetPlayerQuestDataImpl(const std::string& playerID, const std::string& key, const std::string& value) = 0;
    
    // Server management function implementations
    virtual std::string GetServerCVarImpl(const std::string& name, const std::string& defaultValue) const = 0;
    virtual bool IsServerLockedImpl() const = 0;
    virtual std::string GetServerPasswordImpl() const = 0;
    virtual bool IsServerPvpEnabledImpl() const = 0;
    virtual bool IsOnFNServerImpl() const = 0;
    virtual bool IsDeveloperModeImpl() const = 0;
    virtual void ExecuteServerCommandImpl(const std::string& command) = 0;
    virtual void SetGlobalVariableImpl(const std::string& name, const std::string& value) = 0;
    virtual bool EngineMapExistsImpl(const std::string& mapName) const = 0;
    virtual bool MapExistsImpl(const std::string& mapName) const = 0;
    virtual void ScheduleDelayedEventImpl(float delay, const std::string& eventName) = 0;
    
    // Communication function implementations
    virtual void SendPlayerMessageImpl(const std::string& playerID, const std::string& message) = 0;
    virtual void SendConsoleMessageImpl(const std::string& playerID, const std::string& message) = 0;
    virtual void SendMessageToAllPlayersImpl(const std::string& color, const std::string& message) = 0;
    virtual void SendInfoMessageToAllImpl(const std::string& title, const std::string& message) = 0;
};

//==========================================================================
// Global Engine Instance Access
// Platform-specific implementations will provide the actual instance
//==========================================================================
class ASEngineProvider
{
public:
    // Type-safe wrapper functions that route to the correct implementation
    static float GetGameTime();
    static std::string GetCvarString(const std::string& name);
    static std::string GetMapName();
    static void* CreateEntity(const std::string& classname);
    static void SetEntityOrigin(void* entity, const Vector& origin);
    static void SetEntityName(void* entity, const std::string& name);
    static void SetEntityTargetName(void* entity, const std::string& targetname);
    static void SetEntityHealth(void* entity, float health);
    static Vector GetEntityOrigin(void* entity);
    static float GetEntityHealth(void* entity);
    static int GetEntityDeadFlag(void* entity);
    static std::string GetEntityClassName(void* entity);
    static std::string GetPlayerAuthId(void* player);
    static std::string GetPlayerDisplayName(void* player);
    static std::string GetPlayerClientAddress(void* player);
    static int GetPlayerEntIndex(void* player);
    static bool IsValidPlayer(void* player);
    static void* PlayerByIndex(int index);
    static int GetMaxClients();
    static void SendInfoMsg(void* player, const std::string& message);
    static void EmitSound(void* entity, int channel, const std::string& sound, float volume, float attn, int flags, int pitch);
    
    // Player management functions
    static int GetPlayerCount();
    static bool IsPlayerConnected(void* player);
    static bool IsPlayerAdmin(void* player);
    static std::string GetPlayerQuestData(const std::string& playerID, const std::string& key);
    static void SetPlayerQuestData(const std::string& playerID, const std::string& key, const std::string& value);
    
    // Server management functions
    static std::string GetServerCVar(const std::string& name, const std::string& defaultValue);
    static bool IsServerLocked();
    static std::string GetServerPassword();
    static bool IsServerPvpEnabled();
    static bool IsOnFNServer();
    static bool IsDeveloperMode();
    static void ExecuteServerCommand(const std::string& command);
    static void SetGlobalVariable(const std::string& name, const std::string& value);
    static bool EngineMapExists(const std::string& mapName);
    static bool MapExists(const std::string& mapName);
    static void ScheduleDelayedEvent(float delay, const std::string& eventName);
    
    // Communication functions
    static void SendPlayerMessage(const std::string& playerID, const std::string& message);
    static void SendConsoleMessage(const std::string& playerID, const std::string& message);
    static void SendMessageToAllPlayers(const std::string& color, const std::string& message);
    static void SendInfoMessageToAll(const std::string& title, const std::string& message);
};

//==========================================================================
// Convenience macros for easy function declaration
//==========================================================================
#define DECLARE_AS_ENGINE_FUNCTION_0(RetType, FuncName) \
    RetType FuncName() const { return ASEngineProvider::FuncName(); }

#define DECLARE_AS_ENGINE_FUNCTION_1(RetType, FuncName, Arg1Type) \
    RetType FuncName(Arg1Type arg1) const { return ASEngineProvider::FuncName(arg1); }

#define DECLARE_AS_ENGINE_FUNCTION_2(RetType, FuncName, Arg1Type, Arg2Type) \
    RetType FuncName(Arg1Type arg1, Arg2Type arg2) const { return ASEngineProvider::FuncName(arg1, arg2); }

#define DECLARE_AS_ENGINE_FUNCTION_VOID_1(FuncName, Arg1Type) \
    void FuncName(Arg1Type arg1) { ASEngineProvider::FuncName(arg1); }

#define DECLARE_AS_ENGINE_FUNCTION_VOID_2(FuncName, Arg1Type, Arg2Type) \
    void FuncName(Arg1Type arg1, Arg2Type arg2) { ASEngineProvider::FuncName(arg1, arg2); }