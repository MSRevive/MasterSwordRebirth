//==========================================================================
// ASScriptContext.h
// 
// Script execution context management for AngelScript
// Distinguishes between client-side, server-side, and shared scripts
//==========================================================================

#pragma once

#include <string>
#include <set>
#include <map>

//==========================================================================
// Script Execution Context
//==========================================================================
enum class ScriptContext
{
    SERVER_ONLY,    // Server-side only scripts (player management, entity spawning)
    CLIENT_ONLY,    // Client-side only scripts (UI, rendering, input)
    SHARED,         // Scripts that run on both client and server (math, utilities)
    UNKNOWN         // Context not yet determined
};

//==========================================================================
// Build Context - Compile-time determination of client vs server
//==========================================================================
enum class BuildContext
{
#if defined(ISCLIENT)
    CLIENT,
#else
    SERVER,
#endif
    CURRENT = 
#if defined(ISCLIENT)
        CLIENT
#else
        SERVER
#endif
};

//==========================================================================
// Script Context Utilities
//==========================================================================
class ASScriptContextUtil
{
public:
    // Convert context to string
    static const char* ContextToString(ScriptContext context)
    {
        switch (context)
        {
            case ScriptContext::SERVER_ONLY: return "SERVER";
            case ScriptContext::CLIENT_ONLY: return "CLIENT";
            case ScriptContext::SHARED:      return "SHARED";
            case ScriptContext::UNKNOWN:     return "UNKNOWN";
            default:                         return "INVALID";
        }
    }
    
    // Convert string to context
    static ScriptContext StringToContext(const std::string& str)
    {
        if (str == "SERVER" || str == "server")
            return ScriptContext::SERVER_ONLY;
        if (str == "CLIENT" || str == "client")
            return ScriptContext::CLIENT_ONLY;
        if (str == "SHARED" || str == "shared")
            return ScriptContext::SHARED;
        return ScriptContext::UNKNOWN;
    }
    
    // Check if a script can run in the current build context
    static bool CanRunInCurrentContext(ScriptContext scriptContext)
    {
#if defined(ISCLIENT)
        // Client build can run CLIENT and SHARED scripts
        return scriptContext == ScriptContext::CLIENT_ONLY || 
               scriptContext == ScriptContext::SHARED;
#else
        // Server build can run SERVER and SHARED scripts
        return scriptContext == ScriptContext::SERVER_ONLY || 
               scriptContext == ScriptContext::SHARED;
#endif
    }
    
    // Get current build context as script context
    static ScriptContext GetCurrentBuildContext()
    {
#if defined(ISCLIENT)
        return ScriptContext::CLIENT_ONLY;
#else
        return ScriptContext::SERVER_ONLY;
#endif
    }
    
    // Check if we're currently building client or server
    static bool IsClientBuild()
    {
#if defined(ISCLIENT)
        return true;
#else
        return false;
#endif
    }
    
    static bool IsServerBuild()
    {
#if defined(ISCLIENT)
        return false;
#else
        return true;
#endif
    }
};

//==========================================================================
// Script Context Manager
// Tracks context of loaded scripts and validates function calls
//==========================================================================
class ASScriptContextManager
{
public:
    static ASScriptContextManager* Instance();
    static void Shutdown();
    
    // Set context for a script/module
    void SetScriptContext(const std::string& scriptName, ScriptContext context);
    
    // Get context for a script/module
    ScriptContext GetScriptContext(const std::string& scriptName) const;
    
    // Determine context from script path/name
    ScriptContext DetermineContextFromPath(const std::string& path) const;
    
    // Determine context from script content (looks for #pragma directives)
    ScriptContext DetermineContextFromContent(const std::string& content) const;
    
    // Validate that a function can be called in the current context
    bool ValidateFunctionCall(const std::string& functionName, ScriptContext requiredContext) const;
    
    // Register a function with its required context
    void RegisterFunctionContext(const std::string& functionName, ScriptContext context);
    
    // Get current execution context (build-time)
    ScriptContext GetCurrentExecutionContext() const;
    
    // Check if context is compatible
    bool IsContextCompatible(ScriptContext scriptContext, ScriptContext functionContext) const;
    
    // Debug logging
    void LogContextInfo() const;
    
private:
    ASScriptContextManager();
    ~ASScriptContextManager();
    
    static ASScriptContextManager* s_pInstance;
    
    // Map of script names to their contexts
    std::map<std::string, ScriptContext> m_ScriptContexts;
    
    // Map of function names to their required contexts
    std::map<std::string, ScriptContext> m_FunctionContexts;
};

//==========================================================================
// Convenience Macros
//==========================================================================

// Mark a function as server-only at registration
#define AS_REGISTER_SERVER_FUNC(engine, decl, func) \
    do { \
        if (ASScriptContextUtil::IsServerBuild()) { \
            engine->RegisterGlobalFunction(decl, asFUNCTION(func), asCALL_CDECL); \
            ASScriptContextManager::Instance()->RegisterFunctionContext(#func, ScriptContext::SERVER_ONLY); \
        } \
    } while(0)

// Mark a function as client-only at registration
#define AS_REGISTER_CLIENT_FUNC(engine, decl, func) \
    do { \
        if (ASScriptContextUtil::IsClientBuild()) { \
            engine->RegisterGlobalFunction(decl, asFUNCTION(func), asCALL_CDECL); \
            ASScriptContextManager::Instance()->RegisterFunctionContext(#func, ScriptContext::CLIENT_ONLY); \
        } \
    } while(0)

// Mark a function as shared (available on both)
#define AS_REGISTER_SHARED_FUNC(engine, decl, func) \
    do { \
        engine->RegisterGlobalFunction(decl, asFUNCTION(func), asCALL_CDECL); \
        ASScriptContextManager::Instance()->RegisterFunctionContext(#func, ScriptContext::SHARED); \
    } while(0)

// Check at runtime if we're in the correct context
#define AS_CHECK_CONTEXT(required) \
    do { \
        if (!ASScriptContextUtil::CanRunInCurrentContext(required)) { \
            MS_ANGEL_ERROR("Function called in wrong context! Required: %s, Current: %s", \
                ASScriptContextUtil::ContextToString(required), \
                ASScriptContextUtil::ContextToString(ASScriptContextUtil::GetCurrentBuildContext())); \
            return; \
        } \
    } while(0)

#define AS_CHECK_CONTEXT_RET(required, retval) \
    do { \
        if (!ASScriptContextUtil::CanRunInCurrentContext(required)) { \
            MS_ANGEL_ERROR("Function called in wrong context! Required: %s, Current: %s", \
                ASScriptContextUtil::ContextToString(required), \
                ASScriptContextUtil::ContextToString(ASScriptContextUtil::GetCurrentBuildContext())); \
            return retval; \
        } \
    } while(0)

