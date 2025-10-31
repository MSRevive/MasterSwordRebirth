//==========================================================================
// ASClientBindings.h - Client-side AngelScript bindings
//
// Client-side AngelScript API for Master Sword Rebirth
// Provides OOP interface for client effects, entities, environment, etc.
//==========================================================================

#ifndef ASCLIENTBINDINGS_H
#define ASCLIENTBINDINGS_H
#pragma once

#ifdef CLIENT_DLL

#include <angelscript.h>

namespace ASClientBindings
{
    // Main registration function - called from ASBindings::RegisterAll()
    void RegisterAll(asIScriptEngine* pEngine);
    
    // Individual registration functions
    void RegisterLocalPlayer(asIScriptEngine* pEngine);
    void RegisterClientEntity(asIScriptEngine* pEngine);
    void RegisterTempEntity(asIScriptEngine* pEngine);
    void RegisterDynamicLight(asIScriptEngine* pEngine);
    void RegisterBeam(asIScriptEngine* pEngine);
    void RegisterEnvironment(asIScriptEngine* pEngine);
    void RegisterScreenFade(asIScriptEngine* pEngine);
    void RegisterClientSound(asIScriptEngine* pEngine);
    void RegisterUtilityFunctions(asIScriptEngine* pEngine);
}

#endif // CLIENT_DLL

#endif // ASCLIENTBINDINGS_H

