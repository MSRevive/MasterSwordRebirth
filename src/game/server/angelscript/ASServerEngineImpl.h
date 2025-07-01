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

private:
    ASServerEngine() = default;
    ~ASServerEngine() = default;
    ASServerEngine(const ASServerEngine&) = delete;
    ASServerEngine& operator=(const ASServerEngine&) = delete;
    
    // Helper functions for type safety
    void* ValidateEntity(void* entity) const;
    void* ValidatePlayer(void* player) const;
    void* GetEdict(void* entity) const;
};

// Note: All C-style function declarations have been removed.
// ASEntityBindings.cpp now uses ASEngineProvider directly.