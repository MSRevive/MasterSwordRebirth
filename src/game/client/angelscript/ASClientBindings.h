//==========================================================================
// ASClientBindings.h - Client-side AngelScript bindings
//
// Client-side AngelScript API for Master Sword Rebirth
// Provides OOP interface for client effects, entities, environment, etc.
//==========================================================================

#ifndef ASCLIENTBINDINGS_H
#define ASCLIENTBINDINGS_H
#pragma once

#include <angelscript.h>

namespace ASClientBindings
{
    // Main registration function - called from ASBindings::RegisterAll()
    void RegisterAll(asIScriptEngine* pEngine);

#ifndef VALVE_DLL  // Client-only functions
    // Individual registration functions
    void RegisterClientEffect(asIScriptEngine* pEngine);  // Base class for client effects
    void RegisterLocalPlayer(asIScriptEngine* pEngine);
    void RegisterClientEntity(asIScriptEngine* pEngine);
    void RegisterTempEntity(asIScriptEngine* pEngine);
    void RegisterDynamicLight(asIScriptEngine* pEngine);
    void RegisterBeam(asIScriptEngine* pEngine);
    void RegisterEnvironment(asIScriptEngine* pEngine);
    void RegisterScreenFade(asIScriptEngine* pEngine);
    void RegisterClientSound(asIScriptEngine* pEngine);
    void RegisterUtilityFunctions(asIScriptEngine* pEngine);
    void RegisterVGUI(asIScriptEngine* pEngine);
#endif // !VALVE_DLL
}

#endif // ASCLIENTBINDINGS_H

