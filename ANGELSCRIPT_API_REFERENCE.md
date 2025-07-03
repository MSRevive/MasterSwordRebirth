# AngelScript API Reference

This document describes the custom AngelScript API for Master Sword Rebirth.

## Access Scope Legend
- **Server**: Available only in server-side scripts
- **Client**: Available only in client-side scripts  
- **Shared**: Available in both server and client scripts

---

## Core Types

### Vector3
**Scope**: Shared

```angelscript
// Constructors
Vector3()
Vector3(float x, float y, float z)

// Properties
float x
float y
float z

// Methods
float Length() const
float Length2D() const
Vector3 Normalize() const
float DotProduct(const Vector3 &in) const
Vector3 CrossProduct(const Vector3 &in) const
float Distance(const Vector3 &in) const

// Operators
Vector3 opAdd(const Vector3 &in) const
Vector3 opSub(const Vector3 &in) const
Vector3 opMul(float) const
Vector3 opDiv(float) const
bool opEquals(const Vector3 &in) const
```

### Color
**Scope**: Shared

```angelscript
// Constructor
Color(int r, int g, int b, int a = 255)

// Properties
int r
int g
int b
int a
```

### EntityHandle
**Scope**: Shared

```angelscript
// Constructor
EntityHandle(int value = 0)

// Properties
int value

// Methods
bool IsValid() const
CBaseEntity@ GetEntity() const
void SetEntity(CBaseEntity@)
```

---

## Entity Classes

### CBaseEntity
**Scope**: Server

```angelscript
// Methods
Vector3 GetOrigin()
string GetClassName()
void SetOrigin(const Vector3 &in)
float GetHealth()
void SetHealth(float)
bool IsAlive()
string DisplayName()
Vector3 Center()
float Volume()
float Weight()
bool IsPlayer()
```

### CBasePlayer
**Scope**: Server
*Inherits from CBaseEntity*

```angelscript
// Methods
string DisplayName() const
string GetName() const              // Alias for DisplayName
string GETPLAYERAUTHID() const      // Returns Steam ID
int GetEntIndex() const             // Entity index
bool IsConnected() const
bool IsAlive() const
bool IsAdmin() const
Vector3 GetOrigin() const
float GetHealth() const
string GetTitle() const             // Player's in-game title
float MaxHP() const
float MaxMP() const
bool IsElite() const
string GetPartyName() const
bool IsLocalHost() const
string GetClassName() const
void SetOrigin(const Vector3 &in)
void SetHealth(float)
Vector3 Center() const
float Volume() const
float Weight() const
bool IsPlayer() const
void PlaySound(const string &in)
void SendInfoMsg(const string &in)
void SendEventMsg(const string &in)
bool opEquals(const CBasePlayer@+ other) const
```

---

## Global Functions

### String Manipulation
**Scope**: Shared

```angelscript
string Left(const string &in str, int count)
string Right(const string &in str, int count)
string Mid(const string &in str, int start, int count)
int Length(const string &in str)
string ToUpper(const string &in str)
string ToLower(const string &in str)
string Replace(const string &in str, const string &in find, const string &in replace)

// String utilities (requires array support)
array<string>@ split(const string &in) const    // Method on string type
string join(const array<string> &in arr, const string &in delimiter)
```

### Math Functions
**Scope**: Shared

```angelscript
float sin(float)
float cos(float)
float sqrt(float)
float abs(float)
float min(float, float)
float max(float, float)
```

### Game System Functions
**Scope**: Varies

```angelscript
// Time and game state
float GetGameTime()                 // Shared
string GetTimestamp()               // Shared
string GetMapName()                 // Shared
string GetPlayerCurrentMap()        // Shared

// Console variables
string GetCvar(const string &in)    // Shared

// Random number generation
float Random(float min, float max)  // Shared

// Logging
void LogMessage(const string &in)   // Shared
void ChatLog(const string &in)      // Server
void MS_ANGEL_INFO(const string &in)    // Shared
void MS_ANGEL_DEBUG(const string &in)   // Shared
void MS_ANGEL_ERROR(const string &in)   // Shared
```

### Player Management Functions
**Scope**: Server

```angelscript
// Player queries
array<CBasePlayer@>@ GetAllPlayers()
int GetPlayerCount()
CBasePlayer@ PlayerByIndex(int index)
int GetCurrentPlayerID()

// Player information
bool IsConnected(CBasePlayer@)
string GetDisplayName(CBasePlayer@)
string GetSteamID(CBasePlayer@)
string GetPlayerSteamID(CBasePlayer@)    // Alias for GetSteamID
bool IsAdmin(CBasePlayer@)
string GetClientAddress(CBasePlayer@)

// Player communication
void SendPlayerMessage(const string &in playerName, const string &in title, const string &in message)
```

### Entity Management Functions
**Scope**: Server

```angelscript
// Entity creation and manipulation
EntityHandle CreateEntity(const string &in scriptName)
void SetEntityName(const EntityHandle &in, const string &in)
void SetEntityTargetName(const EntityHandle &in, const string &in)
void SetEntityHealth(const EntityHandle &in, float)
bool IsEntityDead(const EntityHandle &in)

// Type casting
CBaseEntity@ ToEntity(CBasePlayer@)
CBasePlayer@ ToPlayer(CBaseEntity@)
```

### Quest Data Functions
**Scope**: Server

```angelscript
string GetPlayerQuestData(const string &in playerID, const string &in key)
void SetPlayerQuestData(const string &in playerID, const string &in key, const string &in value)
```

---

## Script Classes

### CGameScript
**Scope**: Shared

Base class for script objects with variable storage system.

```angelscript
// Variable management methods
void SetVar(const string &in key, const string &in value)
string GetVar(const string &in key)
// Additional methods depend on implementation
```

---

## Coroutine System
**Scope**: Shared

```angelscript
int StartCoroutine(const string &in functionName)
void DelaySeconds(float seconds)
// Additional coroutine functions depend on implementation
```

---

## Module System
**Scope**: Shared

The module system provides comprehensive module management with dependency resolution, import/export functionality, and automatic discovery.

### Module Management Functions

```angelscript
// Core module operations
bool LoadModule(const string &in filename)              // Load a module from file
bool UnloadModule(const string &in name)                // Unload a module by name
bool ReloadModule(const string &in name)                // Reload an existing module
bool HasModule(const string &in name)                   // Check if module is loaded

// Import functionality
bool ImportModule(const string &in moduleName)          // Import module into current namespace
bool ImportModule(const string &in moduleName, const string &in asNamespace)  // Import with namespace
```

### Module Declaration Syntax

Modules are declared in `.as` files using the `module` keyword. The module itself becomes a class, and its constructor is automatically called upon loading:

```angelscript
module MyModule {
    // Module properties
    private int m_counter;
    private string m_name;
    
    // Constructor - automatically called when module loads
    MyModule() {
        m_counter = 0;
        m_name = "MyModule";
        // Initialization code here
    }
    
    // Module methods
    void myFunction() {
        // Function implementation
    }
    
    int getCounter() {
        return m_counter;
    }
}
```

### Module Features

1. **Automatic Discovery**: The system can automatically discover modules in PAK files by scanning for `module ModuleName {` declarations

2. **Dependency Management**: Modules can declare dependencies that are automatically resolved during loading

3. **Import/Export System**: Modules can export functions and types for use by other modules

4. **Module Preprocessing**: The `module` keyword is transformed to `class` syntax internally. The module becomes a class instance.

5. **Lifecycle Management**: The system automatically generates initialization code:
   - `ModuleName_Initialize()` - Creates a global instance `g_ModuleName` and calls the module's constructor
   - `ModuleName_Shutdown()` - Cleans up the global instance
   - The module constructor is executed automatically when the module loads

### Module Load Options

When loading modules programmatically, the following options are available:
- **compileOnly**: Only compile without building
- **allowOverwrite**: Allow replacing existing modules
- **resolveDependencies**: Automatically load required dependencies
- **validateExports**: Check that declared exports exist

### Module Search Paths

The module system searches for modules in configured paths, typically:
- PAK file locations
- Script directories
- Custom paths added via configuration

---

## Engine Event System
**Scope**: Shared

The engine event system allows scripts to respond to game events. Specific event registration functions depend on implementation.

---

## Memory Optimization
**Scope**: Shared

Memory optimization functions are available but typically managed automatically by the engine.

---

## Notes

1. **Entity Lifetime**: Entity references (CBaseEntity@, CBasePlayer@) are managed by the game engine. Scripts should not attempt to delete entities directly.

2. **Array Types**: The `array<T>` template type is available for creating dynamic arrays of any registered type.

3. **Dictionary Type**: A dictionary type is available for key-value storage.

4. **String Type**: Strings in AngelScript use the `string` type with full UTF-8 support.

5. **Reference Counting**: Reference types (marked with @) use automatic reference counting. The engine manages the lifetime of game objects.

6. **Null Checking**: Always check entity references for null before use:
   ```angelscript
   CBasePlayer@ player = PlayerByIndex(1);
   if (player !is null) {
       // Safe to use player
   }
   ```

7. **Type Safety**: AngelScript is strongly typed. Use appropriate casting functions when converting between types.

---

## Version History
- **1.0**: Initial documentation based on ASBindings.cpp and ASEntityBindings.cpp analysis
- **1.1**: Added comprehensive module system documentation from ASModuleSystem.h and scriptmodule.h