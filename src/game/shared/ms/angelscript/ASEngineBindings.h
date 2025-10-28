//==========================================================================
// ASEngineBindings.h
// 
// asbind20-compatible helper for registering engine functions
// Provides template-based registration for clean, type-safe bindings
//==========================================================================

#pragma once

#include <asbind20/asbind.hpp>
#include "ASEngineInterface.h"
#include "ASCoreTypes.h"
#include "mslogger.h"

namespace ASEngineBindings
{
    //==========================================================================
    // Engine Function Registration
    // Uses asbind20 templates for clean, type-safe registration
    //==========================================================================
    
    void RegisterCoreFunctions(asIScriptEngine* pEngine)
    {
        if (!pEngine) {
            MS_ANGEL_ERROR("RegisterCoreFunctions: NULL engine pointer");
            return;
        }
        
        MS_ANGEL_INFO("[ASEngineBindings] Registering core engine functions...");
        
        try {
            // Register engine functions using asbind20
            asbind20::global(pEngine)
                // Game state functions
                .function("float GetGameTime()", &ASEngineProvider::GetGameTime)
                .function("string GetCvar(const string &in)", &ASEngineProvider::GetCvarString)
                .function("string GetMapName()", &ASEngineProvider::GetMapName)
                .function("int GetMaxClients()", &ASEngineProvider::GetMaxClients)
                
                // Entity management functions
                .function("CBaseEntity@ CreateEntity(const string &in)", 
                    +[](const std::string& classname) -> CBaseEntity* {
                        void* entity = ASEngineProvider::CreateEntity(classname);
                        return static_cast<CBaseEntity*>(entity);
                    })
                .function("void SetEntityOrigin(CBaseEntity@, const Vector3 &in)", 
                    +[](CBaseEntity* entity, const Vector& origin) {
                        ASEngineProvider::SetEntityOrigin((void*)entity, origin);
                    })
                .function("void SetEntityName(CBaseEntity@, const string &in)", 
                    +[](CBaseEntity* entity, const std::string& name) {
                        ASEngineProvider::SetEntityName((void*)entity, name);
                    })
                .function("void SetEntityTargetName(CBaseEntity@, const string &in)", 
                    +[](CBaseEntity* entity, const std::string& targetname) {
                        ASEngineProvider::SetEntityTargetName((void*)entity, targetname);
                    })
                .function("void SetEntityHealth(CBaseEntity@, float)", 
                    +[](CBaseEntity* entity, float health) {
                        ASEngineProvider::SetEntityHealth((void*)entity, health);
                    })
                .function("Vector3 GetEntityOrigin(CBaseEntity@)", 
                    +[](CBaseEntity* entity) -> Vector {
                        return ASEngineProvider::GetEntityOrigin((void*)entity);
                    })
                .function("float GetEntityHealth(CBaseEntity@)", 
                    +[](CBaseEntity* entity) -> float {
                        return ASEngineProvider::GetEntityHealth((void*)entity);
                    })
                .function("int GetEntityDeadFlag(CBaseEntity@)", 
                    +[](CBaseEntity* entity) -> int {
                        return ASEngineProvider::GetEntityDeadFlag((void*)entity);
                    })
                .function("string GetEntityClassName(CBaseEntity@)", 
                    +[](CBaseEntity* entity) -> std::string {
                        return ASEngineProvider::GetEntityClassName((void*)entity);
                    })
                .function("bool IsEntityDead(CBaseEntity@)", 
                    +[](CBaseEntity* entity) -> bool {
                        return ASEngineProvider::GetEntityDeadFlag((void*)entity) != 0;
                    })
                .function("bool IsEntityAlive(CBaseEntity@)", 
                    +[](CBaseEntity* entity) -> bool {
                        return ASEngineProvider::GetEntityDeadFlag((void*)entity) == 0 && 
                               ASEngineProvider::GetEntityHealth((void*)entity) > 0;
                    })
                
                // Player-specific functions  
                .function("string GetPlayerAuthId(CBasePlayer@)", 
                    +[](CBasePlayer* player) -> std::string {
                        return ASEngineProvider::GetPlayerAuthId((void*)player);
                    })
                .function("string GetPlayerDisplayName(CBasePlayer@)", 
                    +[](CBasePlayer* player) -> std::string {
                        return ASEngineProvider::GetPlayerDisplayName((void*)player);
                    })
                .function("string GetPlayerClientAddress(CBasePlayer@)", 
                    +[](CBasePlayer* player) -> std::string {
                        return ASEngineProvider::GetPlayerClientAddress((void*)player);
                    })
                .function("int GetPlayerEntIndex(CBasePlayer@)", 
                    +[](CBasePlayer* player) -> int {
                        return ASEngineProvider::GetPlayerEntIndex((void*)player);
                    })
                .function("bool IsValidPlayer(CBasePlayer@)", 
                    +[](CBasePlayer* player) -> bool {
                        return ASEngineProvider::IsValidPlayer((void*)player);
                    })
                .function("void SendPlayerMessage(CBasePlayer@, const string &in)", 
                    +[](CBasePlayer* player, const std::string& message) {
                        ASEngineProvider::SendInfoMsg((void*)player, message);
                    })
                
                // Sound functions
                .function("void EmitSound(CBaseEntity@, int, const string &in, float, float, int, int)", 
                    +[](CBaseEntity* entity, int channel, const std::string& sound, 
                        float volume, float attn, int flags, int pitch) {
                        ASEngineProvider::EmitSound((void*)entity, channel, sound, volume, attn, flags, pitch);
                    })
                .function("void EmitSound(CBaseEntity@, const string &in)", 
                    +[](CBaseEntity* entity, const std::string& sound) {
                        ASEngineProvider::EmitSound((void*)entity, 0, sound, 1.0f, 0.8f, 0, 100);
                    })
                .function("void EmitSound(CBaseEntity@, const string &in, float)", 
                    +[](CBaseEntity* entity, const std::string& sound, float volume) {
                        ASEngineProvider::EmitSound((void*)entity, 0, sound, volume, 0.8f, 0, 100);
                    });
            
            MS_ANGEL_INFO("[ASEngineBindings] Core engine functions registered successfully");
            
        } catch (const std::exception& e) {
            MS_ANGEL_ERROR("[ASEngineBindings] Failed to register core functions: %s", e.what());
        }
    }
    
    void RegisterTypeDefinitions(asIScriptEngine* pEngine)
    {
        if (!pEngine) {
            MS_ANGEL_ERROR("RegisterTypeDefinitions: NULL engine pointer");
            return;
        }
        
        MS_ANGEL_INFO("[ASEngineBindings] Registering engine type definitions...");
        
        try {
            // Note: All types are registered directly in their respective modules
            // No typedef registrations needed here
            
            MS_ANGEL_INFO("[ASEngineBindings] Engine type definitions registered successfully");
            
        } catch (const std::exception& e) {
            MS_ANGEL_ERROR("[ASEngineBindings] Failed to register type definitions: %s", e.what());
        }
    }
    
    void RegisterConvenienceFunctions(asIScriptEngine* pEngine)
    {
        if (!pEngine) {
            MS_ANGEL_ERROR("RegisterConvenienceFunctions: NULL engine pointer");
            return;
        }
        
        MS_ANGEL_INFO("[ASEngineBindings] Registering convenience functions...");
        
        try {
            // Additional convenience functions that combine multiple engine calls
            asbind20::global(pEngine)
                .function("bool TeleportEntity(CBaseEntity@, const Vector3 &in)", 
                    +[](CBaseEntity* entity, const Vector& origin) -> bool {
                        if (!entity) return false;
                        ASEngineProvider::SetEntityOrigin((void*)entity, origin);
                        return true;
                    })
                .function("void HealEntity(CBaseEntity@, float)", 
                    +[](CBaseEntity* entity, float amount) {
                        if (!entity) return;
                        float currentHealth = ASEngineProvider::GetEntityHealth((void*)entity);
                        ASEngineProvider::SetEntityHealth((void*)entity, currentHealth + amount);
                    })
                .function("void DamageEntity(CBaseEntity@, float)", 
                    +[](CBaseEntity* entity, float amount) {
                        if (!entity) return;
                        float currentHealth = ASEngineProvider::GetEntityHealth((void*)entity);
                        ASEngineProvider::SetEntityHealth((void*)entity, currentHealth - amount);
                    })
                .function("void KillEntity(CBaseEntity@)", 
                    +[](CBaseEntity* entity) {
                        if (!entity) return;
                        ASEngineProvider::SetEntityHealth((void*)entity, 0.0f);
                    })
                .function("Vector3 GetPlayerPosition(CBasePlayer@)", 
                    +[](CBasePlayer* player) -> Vector {
                        return ASEngineProvider::GetEntityOrigin((void*)player);
                    })
                .function("bool TeleportPlayer(CBasePlayer@, const Vector3 &in)", 
                    +[](CBasePlayer* player, const Vector& origin) -> bool {
                        if (!ASEngineProvider::IsValidPlayer((void*)player)) return false;
                        ASEngineProvider::SetEntityOrigin((void*)player, origin);
                        return true;
                    });
            
            MS_ANGEL_INFO("[ASEngineBindings] Convenience functions registered successfully");
            
        } catch (const std::exception& e) {
            MS_ANGEL_ERROR("[ASEngineBindings] Failed to register convenience functions: %s", e.what());
        }
    }
    
    void RegisterAll(asIScriptEngine* pEngine)
    {
        if (!pEngine) {
            MS_ANGEL_ERROR("RegisterAll: NULL engine pointer");
            return;
        }
        
        MS_ANGEL_INFO("[ASEngineBindings] Starting engine bindings registration...");
        
        RegisterTypeDefinitions(pEngine);
        RegisterCoreFunctions(pEngine);
        RegisterConvenienceFunctions(pEngine);
        
        MS_ANGEL_INFO("[ASEngineBindings] Engine bindings registration complete");
    }
}

//==========================================================================
// Template helpers for custom engine function registration
// These can be used by other modules to easily register their own functions
//==========================================================================

#define REGISTER_ENGINE_FUNCTION_0(engine, retType, funcName, implFunc) \
    asbind20::global(engine).function(#retType " " #funcName "()", &implFunc)

#define REGISTER_ENGINE_FUNCTION_1(engine, retType, funcName, arg1Type, implFunc) \
    asbind20::global(engine).function(#retType " " #funcName "(" #arg1Type ")", &implFunc)

#define REGISTER_ENGINE_FUNCTION_2(engine, retType, funcName, arg1Type, arg2Type, implFunc) \
    asbind20::global(engine).function(#retType " " #funcName "(" #arg1Type ", " #arg2Type ")", &implFunc)

#define REGISTER_ENGINE_LAMBDA_0(engine, retType, funcName, lambda) \
    asbind20::global(engine).function(#retType " " #funcName "()", +lambda)

#define REGISTER_ENGINE_LAMBDA_1(engine, retType, funcName, arg1Type, lambda) \
    asbind20::global(engine).function(#retType " " #funcName "(" #arg1Type ")", +lambda)

#define REGISTER_ENGINE_LAMBDA_2(engine, retType, funcName, arg1Type, arg2Type, lambda) \
    asbind20::global(engine).function(#retType " " #funcName "(" #arg1Type ", " #arg2Type ")", +lambda)