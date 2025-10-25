//==========================================================================
// ASBindExtensions.h
// 
// Extensions to asbind20 that add context-aware function registration
// Provides clean API for registering functions with script contexts
//==========================================================================

#pragma once

#include <asbind20/asbind.hpp>
#include "ASScriptContext.h"

namespace asbind20
{
//==========================================================================
// Context-aware global function binder
// Wraps asbind20::global and adds context tracking
//==========================================================================
template <bool ForceGeneric = false>
class global_with_context final
{
private:
    asbind20::global<ForceGeneric> m_global;
    asIScriptEngine* m_engine;
    ASScriptContextManager* m_contextMgr;
    
public:
    global_with_context() = delete;
    global_with_context(const global_with_context&) = default;
    
    global_with_context(asIScriptEngine* engine)
        : m_global(engine)
        , m_engine(engine)
        , m_contextMgr(ASScriptContextManager::Instance())
    {
    }
    
    // Function registration with context
    template <typename... Args>
    global_with_context& function(
        const char* decl,
        auto&& func,
        ScriptContext context,
        Args&&... args
    )
    {
        // Register the function with asbind20
        m_global.function(decl, std::forward<decltype(func)>(func), std::forward<Args>(args)...);
        
        // Register the context
        if (m_contextMgr && m_engine)
        {
            asIScriptFunction* asFunc = m_engine->GetGlobalFunctionByDecl(decl);
            if (asFunc)
            {
                m_contextMgr->RegisterFunctionContext(asFunc->GetName(), context);
            }
        }
        
        return *this;
    }
    
    // Fallback for functions without context (defaults to SHARED)
    template <typename... Args>
    global_with_context& function(
        const char* decl,
        auto&& func,
        Args&&... args
    ) requires (!std::is_same_v<std::remove_cvref_t<decltype(args)>, ScriptContext> && ...)
    {
        return function(decl, std::forward<decltype(func)>(func), ScriptContext::SHARED, std::forward<Args>(args)...);
    }
    
    // Pass-through to asbind20::global for other methods
    template <typename... Args>
    global_with_context& property(Args&&... args)
    {
        m_global.property(std::forward<Args>(args)...);
        return *this;
    }
    
    template <typename... Args>
    global_with_context& enum_(Args&&... args)
    {
        m_global.enum_(std::forward<Args>(args)...);
        return *this;
    }
    
    // Get the underlying engine
    asIScriptEngine* engine() const noexcept
    {
        return m_engine;
    }
};

// Helper function to create context-aware global binder
inline global_with_context<false> global_ctx(asIScriptEngine* engine)
{
    return global_with_context<false>(engine);
}

// Convenience constants for cleaner syntax
namespace ctx
{
    constexpr ScriptContext server = ScriptContext::SERVER_ONLY;
    constexpr ScriptContext client = ScriptContext::CLIENT_ONLY;
    constexpr ScriptContext shared = ScriptContext::SHARED;
}

} // namespace asbind20

//==========================================================================
// Convenience macros for context-aware registration
//==========================================================================

// Register a global function with explicit context
#define AS_GLOBAL_FUNC(engine, decl, func, context) \
    asbind20::global_ctx(engine).function(decl, func, context)

// Register multiple functions fluently with context
#define AS_GLOBAL_BEGIN(engine) \
    asbind20::global_ctx(engine)

// Usage example:
// AS_GLOBAL_BEGIN(engine)
//     .function("void ServerFunc()", &MyServerFunc, asbind20::ctx::server)
//     .function("void ClientFunc()", &MyClientFunc, asbind20::ctx::client)
//     .function("float MathFunc(float)", &MyMathFunc, asbind20::ctx::shared);

