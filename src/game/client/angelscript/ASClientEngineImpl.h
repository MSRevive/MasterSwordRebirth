//==========================================================================
// ASClientEngineImpl.h
// 
// Client-side implementation of the AngelScript engine interface
// Provides client-side game integration for all engine functions
//==========================================================================

#pragma once

#include "ms/angelscript/ASEngineInterface.h"
#include <string>

//==========================================================================
// Client Engine Implementation
// Connects AngelScript calls to actual game client functionality
//==========================================================================
class ASClientEngine : public ASEngineInterface<ASClientEngine>
{
public:
    // Singleton access
    static ASClientEngine& GetInstance();
    
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

private:
    ASClientEngine() = default;
    ~ASClientEngine() = default;
    ASClientEngine(const ASClientEngine&) = delete;
    ASClientEngine& operator=(const ASClientEngine&) = delete;
    
    // Helper functions for client-side validation
    void* ValidateClientEntity(void* entity) const;
};

// Note: C-style wrapper functions have been removed.
// The new template system uses ASEngineProvider directly through asbind20.