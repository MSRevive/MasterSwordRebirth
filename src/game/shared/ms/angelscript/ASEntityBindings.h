//==========================================================================
// ASEntityBindings.h
// 
// Entity-related type bindings for AngelScript integration
//==========================================================================

#pragma once

class asIScriptEngine;

namespace ASEntityBindings
{
    void RegisterAll(asIScriptEngine* pEngine);
    void RegisterEntityTypes(asIScriptEngine* pEngine);
    void RegisterGlobalFunctions(asIScriptEngine* pEngine);
}