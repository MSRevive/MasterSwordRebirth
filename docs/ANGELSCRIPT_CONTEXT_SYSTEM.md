# AngelScript Context System

## Overview

The AngelScript Context System provides a robust way to distinguish between client-side, server-side, and shared scripts. This ensures that scripts only run in appropriate execution environments and helps prevent bugs from calling wrong-side functions.

## Script Context Types

### Server-Only (`SERVER`)
- Scripts that run exclusively on the server
- Used for: Player management, entity spawning, game logic, quest data
- Cannot run in client builds
- Has access to server-only functions

### Client-Only (`CLIENT`)
- Scripts that run exclusively on the client
- Used for: UI, rendering, input handling, visual effects
- Cannot run in server builds
- Has access to client-only functions

### Shared (`SHARED`)
- Scripts that run on both client and server
- Used for: Math utilities, data structures, common algorithms
- Can run in any build
- Only has access to shared functions

## How to Specify Script Context

The system provides **three ways** to specify script context (in order of precedence):

### 1. Pragma Directive (Highest Priority)
Add a pragma at the top of your script file:

```angelscript
#pragma context server

// This is a server-side script
void OnPlayerConnect(CBasePlayer@ player) {
    // Server-side logic
}
```

Supported pragmas:
- `#pragma context server`
- `#pragma context client`
- `#pragma context shared`

### 2. Comment Marker
Use a comment marker (useful for backwards compatibility):

```angelscript
// @context client

// This is a client-side script
void UpdateUI() {
    // Client UI logic
}
```

### 3. Directory Structure (Automatic Detection)
Organize scripts in directories:

```
scripts/angelscript/
├── server/          # All scripts here are SERVER-only
│   ├── player_management.as
│   └── quest_system.as
├── client/          # All scripts here are CLIENT-only
│   ├── ui_manager.as
│   └── effects.as
└── shared/          # All scripts here are SHARED
    ├── math_utils.as
    └── data_structures.as
```

### 4. File Naming Convention (Automatic Detection)
Use naming patterns:

```
server_*.as        # Server-only scripts
*.server.as        # Server-only scripts
client_*.as        # Client-only scripts
*.client.as        # Client-only scripts
shared_*.as        # Shared scripts
*.shared.as        # Shared scripts
```

Examples:
- `server_player_manager.as` → SERVER context
- `quest_system.server.as` → SERVER context
- `client_hud.as` → CLIENT context
- `ui_manager.client.as` → CLIENT context
- `math_utils.shared.as` → SHARED context

## Function Context Registration

When registering C++ functions with AngelScript, use the context-aware macros:

### Server-Only Functions
```cpp
// In your bindings file
AS_REGISTER_SERVER_FUNC(engine, 
    "void SpawnEntity(const string &in)", 
    AS_SpawnEntity);
```

### Client-Only Functions
```cpp
AS_REGISTER_CLIENT_FUNC(engine, 
    "void ShowNotification(const string &in)", 
    AS_ShowNotification);
```

### Shared Functions
```cpp
AS_REGISTER_SHARED_FUNC(engine, 
    "float Random(float, float)", 
    AS_Random);
```

## Runtime Context Checking

The system automatically validates that:
1. Scripts can only load if compatible with the current build
2. Functions are only registered in appropriate builds
3. Module dependencies respect context boundaries

### Manual Context Checking (Advanced)
In C++ code, you can manually check contexts:

```cpp
// Check if we're in the right context
AS_CHECK_CONTEXT(ScriptContext::SERVER_ONLY);

// Check with return value
AS_CHECK_CONTEXT_RET(ScriptContext::CLIENT_ONLY, false);
```

## Build-Time Distinction

The system uses compile-time checks to distinguish client from server:

```cpp
#if defined(CLIENT_DLL)
    // Client-side code
#else
    // Server-side code
#endif
```

You can also use the utility functions:

```cpp
if (ASScriptContextUtil::IsClientBuild()) {
    // Client-specific initialization
}

if (ASScriptContextUtil::IsServerBuild()) {
    // Server-specific initialization
}
```

## Example Scripts

### Server-Side Script Example
```angelscript
#pragma context server

// Server-side player management
module PlayerManager {
    PlayerManager() {
        LogMessage("PlayerManager initialized on SERVER");
    }
    
    void OnPlayerJoin(CBasePlayer@ player) {
        // This can only run on server
        string steamID = player.GETPLAYERAUTHID();
        SetPlayerQuestData(steamID, "last_login", GetTimestamp());
        
        SendInfoMessageToAll("Player Joined", 
            player.DisplayName() + " has joined the game!");
    }
}
```

### Client-Side Script Example
```angelscript
#pragma context client

// Client-side UI manager
module UIManager {
    UIManager() {
        LogMessage("UIManager initialized on CLIENT");
    }
    
    void ShowWelcomeMessage() {
        // This can only run on client
        // Show UI elements, play sounds, etc.
    }
}
```

### Shared Script Example
```angelscript
#pragma context shared

// Shared math utilities
module MathUtils {
    float Clamp(float value, float minVal, float maxVal) {
        return max(minVal, min(maxVal, value));
    }
    
    Vector3 LerpVector(const Vector3 &in a, const Vector3 &in b, float t) {
        t = Clamp(t, 0.0f, 1.0f);
        return a + (b - a) * t;
    }
}
```

## Context Compatibility Rules

### Script Loading
- **SERVER build**: Can load SERVER and SHARED scripts
- **CLIENT build**: Can load CLIENT and SHARED scripts
- Attempting to load incompatible scripts will fail with an error

### Function Calls
- **SERVER scripts**: Can call SERVER and SHARED functions
- **CLIENT scripts**: Can call CLIENT and SHARED functions
- **SHARED scripts**: Can call any SHARED functions
- Calling incompatible functions will result in runtime errors

### Module Dependencies
Modules can only depend on modules that are compatible:
- SERVER modules can depend on SERVER or SHARED modules
- CLIENT modules can depend on CLIENT or SHARED modules
- SHARED modules can only depend on other SHARED modules

## Debugging and Diagnostics

### Enable Context Logging
The system automatically logs context information:

```
ASModuleSystem: Loading module 'PlayerManager' with context: SERVER
ASScriptContextManager initialized for SERVER build
Set script context: PlayerManager = SERVER
```

### Check Context at Runtime
```cpp
ASScriptContextManager* pMgr = ASScriptContextManager::Instance();
pMgr->LogContextInfo();
```

Output:
```
=== Script Context Manager Status ===
Current Build: SERVER
Registered Scripts: 3
  PlayerManager: SERVER
  UIManager: CLIENT (would fail to load)
  MathUtils: SHARED
Registered Functions: 42
  Server-only: 15
  Client-only: 8
  Shared: 19
=====================================
```

## Best Practices

1. **Always specify context explicitly** using `#pragma context` for clarity

2. **Organize by directory** to make it easy to see which scripts belong where

3. **Use SHARED for utilities** - Keep math, data structures, and algorithms in shared scripts

4. **Test in both builds** - Ensure your scripts load correctly in client and server builds

5. **Document context requirements** - Add comments explaining why a script needs a specific context

6. **Avoid context mixing** - Don't try to work around the system; it's there to protect you from bugs

## Migration Guide

### For Existing Scripts

1. **Identify current usage**: Determine if scripts are used on client, server, or both

2. **Add pragmas**: Add `#pragma context` to each script

3. **Reorganize if needed**: Move scripts to appropriate directories

4. **Test thoroughly**: Ensure scripts load and run correctly after migration

### Example Migration

Before:
```angelscript
// some_script.as
void MyFunction() {
    // What context is this?
}
```

After:
```angelscript
// server/player_system.as
#pragma context server

void MyFunction() {
    // Clearly a server function
}
```

## Troubleshooting

### "Module context is not compatible with current build"
**Cause**: Trying to load a server script in a client build (or vice versa)
**Solution**: Verify the script's context matches where you're trying to run it

### "Function called in wrong context"
**Cause**: Attempting to call a server-only function from a client script
**Solution**: Move the logic to the appropriate context or use shared functions

### "Context defaulting to SHARED"
**Cause**: No context was specified for the script
**Solution**: Add an explicit `#pragma context` directive

## API Reference

### ASScriptContextUtil Functions

```cpp
// Convert context to string
const char* ContextToString(ScriptContext context);

// Convert string to context
ScriptContext StringToContext(const std::string& str);

// Check if script can run in current context
bool CanRunInCurrentContext(ScriptContext scriptContext);

// Get current build context
ScriptContext GetCurrentBuildContext();

// Check build type
bool IsClientBuild();
bool IsServerBuild();
```

### ASScriptContextManager Functions

```cpp
// Set/get script context
void SetScriptContext(const std::string& scriptName, ScriptContext context);
ScriptContext GetScriptContext(const std::string& scriptName) const;

// Context determination
ScriptContext DetermineContextFromPath(const std::string& path) const;
ScriptContext DetermineContextFromContent(const std::string& content) const;

// Validation
bool ValidateFunctionCall(const std::string& functionName, ScriptContext requiredContext) const;
bool IsContextCompatible(ScriptContext scriptContext, ScriptContext functionContext) const;

// Function registration
void RegisterFunctionContext(const std::string& functionName, ScriptContext context);

// Debugging
void LogContextInfo() const;
```

## See Also

- [AngelScript API Reference](../ANGELSCRIPT_API_REFERENCE.md)
- [Module System Documentation](ASModuleSystem.h)
- [Build and Debug Guide](../BUILD_AND_DEBUG_GUIDE.md)

