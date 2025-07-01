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
    
    void EmitSound(void* entity, int channel, const std::string& sound, float volume, float attn, int flags, int pitch)
    {
        static_cast<Derived*>(this)->EmitSoundImpl(entity, channel, sound, volume, attn, flags, pitch);
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