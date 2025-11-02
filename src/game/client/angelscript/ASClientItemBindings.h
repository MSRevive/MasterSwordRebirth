//==========================================================================
// ASClientItemBindings.h - Client-side item bindings for AngelScript
//
// Provides read-only access to CGenericItem properties for client-side
// UI, display, and visualization purposes. NO game logic methods exposed.
//==========================================================================

#pragma once

#include <angelscript.h>

namespace ASClientItemBindings
{
    // Register all client-side item bindings
    void RegisterAll(asIScriptEngine* pEngine);
}
