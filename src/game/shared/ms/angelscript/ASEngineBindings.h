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
                .function("EntityHandle CreateEntity(const string &in)", 
                    +[](const std::string& classname) -> EntityHandle {
                        void* entity = ASEngineProvider::CreateEntity(classname);
                        return EntityHandle(reinterpret_cast<int>(entity));
                    })
                .function("void SetEntityOrigin(EntityHandle, const Vector3 &in)", 
                    +[](const EntityHandle& handle, const Vector& origin) {
                        void* entity = reinterpret_cast<void*>(handle.value);
                        ASEngineProvider::SetEntityOrigin(entity, origin);
                    })
                .function("void SetEntityName(EntityHandle, const string &in)", 
                    +[](const EntityHandle& handle, const std::string& name) {
                        void* entity = reinterpret_cast<void*>(handle.value);
                        ASEngineProvider::SetEntityName(entity, name);
                    })
                .function("void SetEntityTargetName(EntityHandle, const string &in)", 
                    +[](const EntityHandle& handle, const std::string& targetname) {
                        void* entity = reinterpret_cast<void*>(handle.value);
                        ASEngineProvider::SetEntityTargetName(entity, targetname);
                    })
                .function("void SetEntityHealth(EntityHandle, float)", 
                    +[](const EntityHandle& handle, float health) {
                        void* entity = reinterpret_cast<void*>(handle.value);
                        ASEngineProvider::SetEntityHealth(entity, health);
                    })
                .function("Vector3 GetEntityOrigin(EntityHandle)", 
                    +[](const EntityHandle& handle) -> Vector {
                        void* entity = reinterpret_cast<void*>(handle.value);
                        return ASEngineProvider::GetEntityOrigin(entity);
                    })
                .function("float GetEntityHealth(EntityHandle)", 
                    +[](const EntityHandle& handle) -> float {
                        void* entity = reinterpret_cast<void*>(handle.value);
                        return ASEngineProvider::GetEntityHealth(entity);
                    })
                .function("int GetEntityDeadFlag(EntityHandle)", 
                    +[](const EntityHandle& handle) -> int {
                        void* entity = reinterpret_cast<void*>(handle.value);
                        return ASEngineProvider::GetEntityDeadFlag(entity);
                    })
                .function("string GetEntityClassName(EntityHandle)", 
                    +[](const EntityHandle& handle) -> std::string {
                        void* entity = reinterpret_cast<void*>(handle.value);
                        return ASEngineProvider::GetEntityClassName(entity);
                    })
                .function("bool IsEntityDead(EntityHandle)", 
                    +[](const EntityHandle& handle) -> bool {
                        void* entity = reinterpret_cast<void*>(handle.value);
                        return ASEngineProvider::GetEntityDeadFlag(entity) != 0;
                    })
                .function("bool IsEntityAlive(EntityHandle)", 
                    +[](const EntityHandle& handle) -> bool {
                        void* entity = reinterpret_cast<void*>(handle.value);
                        return ASEngineProvider::GetEntityDeadFlag(entity) == 0 && 
                               ASEngineProvider::GetEntityHealth(entity) > 0;
                    })
                
                // Player-specific functions  
                .function("string GetPlayerAuthId(PlayerHandle)", 
                    +[](const PlayerHandle& handle) -> std::string {
                        void* player = reinterpret_cast<void*>(handle.value);
                        return ASEngineProvider::GetPlayerAuthId(player);
                    })
                .function("string GetPlayerDisplayName(PlayerHandle)", 
                    +[](const PlayerHandle& handle) -> std::string {
                        void* player = reinterpret_cast<void*>(handle.value);
                        return ASEngineProvider::GetPlayerDisplayName(player);
                    })
                .function("string GetPlayerClientAddress(PlayerHandle)", 
                    +[](const PlayerHandle& handle) -> std::string {
                        void* player = reinterpret_cast<void*>(handle.value);
                        return ASEngineProvider::GetPlayerClientAddress(player);
                    })
                .function("int GetPlayerEntIndex(PlayerHandle)", 
                    +[](const PlayerHandle& handle) -> int {
                        void* player = reinterpret_cast<void*>(handle.value);
                        return ASEngineProvider::GetPlayerEntIndex(player);
                    })
                .function("bool IsValidPlayer(PlayerHandle)", 
                    +[](const PlayerHandle& handle) -> bool {
                        void* player = reinterpret_cast<void*>(handle.value);
                        return ASEngineProvider::IsValidPlayer(player);
                    })
                .function("PlayerHandle PlayerByIndex(int)", 
                    +[](int index) -> PlayerHandle {
                        void* player = ASEngineProvider::PlayerByIndex(index);
                        return PlayerHandle(reinterpret_cast<int>(player));
                    })
                .function("void SendPlayerMessage(PlayerHandle, const string &in)", 
                    +[](const PlayerHandle& handle, const std::string& message) {
                        void* player = reinterpret_cast<void*>(handle.value);
                        ASEngineProvider::SendInfoMsg(player, message);
                    })
                
                // Sound functions
                .function("void EmitSound(EntityHandle, int, const string &in, float, float, int, int)", 
                    +[](void* entity, int channel, const std::string& sound, 
                        float volume, float attn, int flags, int pitch) {
                        ASEngineProvider::EmitSound(entity, channel, sound, volume, attn, flags, pitch);
                    })
                .function("void EmitSound(EntityHandle, const string &in)", 
                    +[](void* entity, const std::string& sound) {
                        ASEngineProvider::EmitSound(entity, 0, sound, 1.0f, 0.8f, 0, 100);
                    })
                .function("void EmitSound(EntityHandle, const string &in, float)", 
                    +[](void* entity, const std::string& sound, float volume) {
                        ASEngineProvider::EmitSound(entity, 0, sound, volume, 0.8f, 0, 100);
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
            // Note: Both EntityHandle and PlayerHandle are now registered as proper value types in ASCoreTypes
            // No typedef registrations needed here
            
            MS_ANGEL_INFO("[ASEngineBindings] Engine type definitions registered successfully (types handled by ASCoreTypes)");
            
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
                .function("bool TeleportEntity(EntityHandle, const Vector3 &in)", 
                    +[](const EntityHandle& handle, const Vector& origin) -> bool {
                        void* entity = reinterpret_cast<void*>(handle.value);
                        if (!entity) return false;
                        ASEngineProvider::SetEntityOrigin(entity, origin);
                        return true;
                    })
                .function("void HealEntity(EntityHandle, float)", 
                    +[](const EntityHandle& handle, float amount) {
                        void* entity = reinterpret_cast<void*>(handle.value);
                        if (!entity) return;
                        float currentHealth = ASEngineProvider::GetEntityHealth(entity);
                        ASEngineProvider::SetEntityHealth(entity, currentHealth + amount);
                    })
                .function("void DamageEntity(EntityHandle, float)", 
                    +[](const EntityHandle& handle, float amount) {
                        void* entity = reinterpret_cast<void*>(handle.value);
                        if (!entity) return;
                        float currentHealth = ASEngineProvider::GetEntityHealth(entity);
                        ASEngineProvider::SetEntityHealth(entity, currentHealth - amount);
                    })
                .function("void KillEntity(EntityHandle)", 
                    +[](const EntityHandle& handle) {
                        void* entity = reinterpret_cast<void*>(handle.value);
                        if (!entity) return;
                        ASEngineProvider::SetEntityHealth(entity, 0.0f);
                    })
                .function("Vector3 GetPlayerPosition(PlayerHandle)", 
                    +[](const PlayerHandle& handle) -> Vector {
                        void* player = reinterpret_cast<void*>(handle.value);
                        return ASEngineProvider::GetEntityOrigin(player);
                    })
                .function("bool TeleportPlayer(PlayerHandle, const Vector3 &in)", 
                    +[](const PlayerHandle& handle, const Vector& origin) -> bool {
                        void* player = reinterpret_cast<void*>(handle.value);
                        if (!ASEngineProvider::IsValidPlayer(player)) return false;
                        ASEngineProvider::SetEntityOrigin(player, origin);
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