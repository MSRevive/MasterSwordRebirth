# Context-Aware Function Binding Guide

## Overview

We've extended asbind20 with a context-aware wrapper that automatically tracks function contexts during registration. This provides a clean API for registering functions with their execution contexts.

## Quick Start

### Include the Header

```cpp
#include "ASBindExtensions.h"
```

### Basic Usage

Instead of using `asbind20::global()`, use `asbind20::global_ctx()`:

```cpp
// Old way (still works, but no context tracking)
asbind20::global(engine)
    .function("float Random(float, float)", &AS_Random);

// New way with automatic context tracking
asbind20::global_ctx(engine)
    .function("float Random(float, float)", &AS_Random, asbind20::ctx::shared);
```

## Context Constants

Use these convenience constants for cleaner code:

```cpp
using namespace asbind20::ctx;

asbind20::global_ctx(engine)
    .function("void ServerFunc()", &MyServerFunc, server)   // SERVER_ONLY
    .function("void ClientFunc()", &MyClientFunc, client)   // CLIENT_ONLY
    .function("float MathFunc()", &MyMathFunc, shared);     // SHARED
```

##Real-World Examples

### Example 1: Math Functions (SHARED)

```cpp
void RegisterMathFunctions(asIScriptEngine* pEngine)
{
    using namespace asbind20::ctx;
    
    asbind20::global_ctx(pEngine)
        .function("int abs(int)", &AS_AbsInt, shared)
        .function("int min(int, int)", &AS_MinInt, shared)
        .function("int max(int, int)", &AS_MaxInt, shared);
}
```

### Example 2: Vector Utilities (SHARED)

```cpp
void RegisterVectorFunctions(asIScriptEngine* pEngine)
{
    using namespace asbind20::ctx;
    
    asbind20::global_ctx(pEngine)
        .function("Vector3 CreateVector(float, float, float)", &AS_CreateVector, shared)
        .function("float Distance(const Vector3 &in, const Vector3 &in)", &AS_Distance, shared)
        .function("float DotProduct(const Vector3 &in, const Vector3 &in)", &AS_DotProduct, shared);
}
```

### Example 3: Game Utilities (SHARED)

```cpp
void RegisterGameUtilities(asIScriptEngine* pEngine)
{
    using namespace asbind20::ctx;
    
    asbind20::global_ctx(pEngine)
        // String utilities - SHARED
        .function("string formatFloat(float, const string &in, int, int)", &AS_FormatFloat, shared)
        // Random functions - SHARED
        .function("float Random(float, float)", &AS_Random, shared)
        .function("int RandomInt(int, int)", &AS_RandomInt, shared)
        // Logging - SHARED
        .function("void LogMessage(const string &in)", &AS_LogMessage, shared)
        // Angle utilities - SHARED
        .function("Vector3 CreateAngles(float, float, float)", &AS_CreateAngles, shared);
}
```

### Example 4: Server-Only Functions

For functions that can't use asbind20 (like arrays), use the helper approach:

```cpp
void RegisterServerFunctions(asIScriptEngine* pEngine)
{
    using namespace asbind20::ctx;
    ASScriptContextManager* pContextMgr = ASScriptContextManager::Instance();
    
    // Helper lambda for SERVER_ONLY registration
    auto regServerFunc = [&](const char* decl, auto func) {
        pEngine->RegisterGlobalFunction(decl, asFUNCTION(func), asCALL_CDECL);
        if (pContextMgr) {
            asIScriptFunction* asFunc = pEngine->GetGlobalFunctionByDecl(decl);
            if (asFunc) pContextMgr->RegisterFunctionContext(asFunc->GetName(), ScriptContext::SERVER_ONLY);
        }
    };
    
    // Register all server functions
    regServerFunc("void SendPlayerMessage(const string &in, const string &in)", AS_SendPlayerMessage);
    regServerFunc("void SendInfoMessageToAll(const string &in, const string &in)", AS_SendInfoMessageToAll);
    regServerFunc("void InitializeAdvancedTriggerSystem()", AS_InitializeAdvancedTriggerSystem);
}
```

## API Reference

### `global_ctx<ForceGeneric>(asIScriptEngine* engine)`

Creates a context-aware global function binder.

**Parameters:**
- `engine`: The AngelScript engine pointer

**Returns:** `global_with_context` object for chaining

### `.function(decl, func, context, ...)`

Registers a global function with context tracking.

**Parameters:**
- `decl`: Function declaration string
- `func`: Function pointer
- `context`: Script context (server/client/shared)
- `...`: Additional asbind20 parameters (optional)

**Returns:** Reference to `global_with_context` for chaining

### Context Constants

- `asbind20::ctx::server` - SERVER_ONLY context
- `asbind20::ctx::client` - CLIENT_ONLY context
- `asbind20::ctx::shared` - SHARED context

## Benefits

### 1. **Automatic Context Tracking**

No need to manually call `RegisterFunctionContext` after every function registration:

```cpp
// Before (manual tracking)
asbind20::global(engine)
    .function("float Random(float, float)", &AS_Random);
RegisterFunctionContext(engine, "float Random(float, float)", ScriptContext::SHARED);

// After (automatic tracking)
asbind20::global_ctx(engine)
    .function("float Random(float, float)", &AS_Random, shared);
```

### 2. **Cleaner Code**

Registration and context tracking happen in one place:

```cpp
using namespace asbind20::ctx;

asbind20::global_ctx(engine)
    .function("float sin(float)", &AS_Sin, shared)
    .function("float cos(float)", &AS_Cos, shared)
    .function("float sqrt(float)", &AS_Sqrt, shared)
    .function("float abs(float)", &AS_Abs, shared);
```

### 3. **Type Safety**

Context is enforced at compile time using strongly-typed enums.

### 4. **Self-Documenting**

The context is explicit in the registration code, making it clear which functions are available where.

## Default Context

If you omit the context parameter, functions default to `SHARED`:

```cpp
// These are equivalent:
asbind20::global_ctx(engine).function("float Random(float, float)", &AS_Random);
asbind20::global_ctx(engine).function("float Random(float, float)", &AS_Random, shared);
```

## Macros (Optional)

For even more concise code, you can use the provided macros:

```cpp
// Single function registration
AS_GLOBAL_FUNC(engine, "float Random(float, float)", &AS_Random, shared);

// Multiple functions
AS_GLOBAL_BEGIN(engine)
    .function("void ServerFunc()", &MyServerFunc, server)
    .function("void ClientFunc()", &MyClientFunc, client)
    .function("float MathFunc()", &MyMathFunc, shared);
```

## Migration Guide

### Step 1: Include the New Header

```cpp
#include "ASBindExtensions.h"
```

### Step 2: Replace `global()` with `global_ctx()`

```cpp
// Before
asbind20::global(engine)

// After
asbind20::global_ctx(engine)
```

### Step 3: Add Context Parameters

```cpp
// Before
.function("float Random(float, float)", &AS_Random)

// After
.function("float Random(float, float)", &AS_Random, shared)
```

### Step 4: Remove Manual Context Registration

```cpp
// Before - delete these lines
RegisterFunctionContext(engine, "float Random(float, float)", ScriptContext::SHARED);

// After - automatic, no manual registration needed
```

## Best Practices

### 1. **Use Namespace Alias**

Always use the namespace alias for cleaner code:

```cpp
using namespace asbind20::ctx;
```

### 2. **Group by Context**

Organize registrations by context for clarity:

```cpp
using namespace asbind20::ctx;

// SHARED functions
asbind20::global_ctx(engine)
    .function("float sin(float)", &AS_Sin, shared)
    .function("float cos(float)", &AS_Cos, shared);

// SERVER_ONLY functions (if using asbind20)
asbind20::global_ctx(engine)
    .function("void ServerCmd()", &AS_ServerCmd, server);
```

### 3. **Add Comments for Complex Cases**

```cpp
using namespace asbind20::ctx;

asbind20::global_ctx(engine)
    // Math utilities - SHARED (useful on both client and server)
    .function("float Random(float, float)", &AS_Random, shared)
    // String utilities - SHARED (common operations)
    .function("string formatFloat(float, ...)", &AS_FormatFloat, shared);
```

### 4. **Use Helper Lambda for Non-asbind20 Functions**

Some functions (like those with arrays) can't use asbind20. Use a helper lambda:

```cpp
auto regFunc = [&](const char* decl, auto func, ScriptContext ctx) {
    pEngine->RegisterGlobalFunction(decl, asFUNCTION(func), asCALL_CDECL);
    if (auto* pMgr = ASScriptContextManager::Instance()) {
        if (auto* asFunc = pEngine->GetGlobalFunctionByDecl(decl)) {
            pMgr->RegisterFunctionContext(asFunc->GetName(), ctx);
        }
    }
};

regFunc("void SendMessage(const array<string>@ &in)", AS_SendMessage, ScriptContext::SERVER_ONLY);
```

## Troubleshooting

### Functions Not Showing in Context Info

**Problem:** Registered functions don't appear in context manager logs.

**Solution:** Ensure you're using `global_ctx()` instead of `global()`:

```cpp
// Wrong - no context tracking
asbind20::global(engine).function(...);

// Correct - with context tracking
asbind20::global_ctx(engine).function(..., shared);
```

### Compile Errors with Context Constants

**Problem:** `server`, `client`, `shared` not recognized.

**Solution:** Add namespace alias:

```cpp
using namespace asbind20::ctx;
```

### Old asbind20 Code Still Works

**Good News:** The extension is backward compatible. You can gradually migrate existing code without breaking anything. Functions registered with `asbind20::global()` still work, they just won't have context tracking.

## Summary

The context-aware binding extension provides:
- ✅ **Automatic context tracking** during registration
- ✅ **Cleaner, more maintainable** code
- ✅ **Self-documenting** function contexts
- ✅ **Type-safe** context specification
- ✅ **Backward compatible** with existing code
- ✅ **No manual cleanup** required

Start using it today for cleaner AngelScript bindings!

