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

---

## Engine Constants
**Scope**: Shared

### Render Mode Constants
```angelscript
const int kRenderNormal = 0          // Normal rendering
const int kRenderTransColor = 1      // Transparent color
const int kRenderTransTexture = 2    // Transparent texture
const int kRenderGlow = 3            // Glow effect
const int kRenderTransAlpha = 4      // Transparent alpha
const int kRenderTransAdd = 5        // Additive transparency
```

### Damage Type Constants
```angelscript
const int DAMAGE_NO = 0   // Entity takes no damage
const int DAMAGE_YES = 1  // Entity takes normal damage
const int DAMAGE_AIM = 2  // Entity takes aim-based damage
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

// Entity configuration methods
void SetNetName(const string &in)
string GetNetName()
void SetRenderMode(int)
void SetRenderAmount(int)
void SetTakeDamage(int)
void SetGodMode(bool)
```

### CBasePlayer
**Scope**: Server
*Inherits from CBaseEntity*

```angelscript
// Methods
string DisplayName() const
string GetName() const              // Alias for DisplayName
string GetSteamID() const           // Returns Steam ID
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

// Enhanced messaging methods with color support
void SendColoredMessage(MessageColor color, const string &in message)
void SendHUDInfoMessage(const string &in title, const string &in message)

// Map transition methods
void SetTransitionFields(const string &in localSpawn, const string &in destMap, const string &in destSpawn)
string GetOldTransition() const
string GetNextMap() const
string GetNextTransition() const
int GetJoinType() const
void SetJoinType(int joinType)

// Spawn management methods
bool MoveToSpawnSpot()
void SetSpawnTransition(const string &in transName)
string GetSpawnTransition() const

// Inventory/Item management methods
CBasePlayerItem@ GetItemBySlot(int slot) const
CBasePlayerWeapon@ GetActiveWeapon() const
array<CBasePlayerItem@>@ GetInventory()
bool HasItem(const string &in itemName) const

// Comparison
bool opEquals(const CBasePlayer@+ other) const
```

**SetTransitionFields** sets the player's C++ transition fields which control where they spawn after map changes:
- `localSpawn`: Current spawn point name on the current map (sets `m_OldTransition` and `m_SpawnTransition`)
- `destMap`: Destination map name (sets `m_NextMap`)
- `destSpawn`: Spawn point name on the destination map (sets `m_NextTransition`)
- Automatically calls `SaveChar()` to persist the changes

**GetOldTransition**, **GetNextMap**, **GetNextTransition**: Retrieve the current transition state

**GetJoinType** / **SetJoinType**: Get or set how the player joined the server
- JoinType constants: `JN_NOTALLOWED` (0), `JN_TRAVEL` (1), `JN_STARTMAP` (2), `JN_VISITED` (3), `JN_ELITE` (4)
- Changing JoinType affects spawn point selection logic
- Useful for ensuring transitions take priority over start map status

**MoveToSpawnSpot** moves the player to their designated spawn point:
- Returns `true` if successful, `false` otherwise
- Uses the player's current spawn transition to find the appropriate spawn point
- Automatically sets player position and angles to match the spawn point

**SetSpawnTransition** / **GetSpawnTransition**: Set or retrieve the player's current spawn transition name:
- The spawn transition determines which `ms_player_spawn` entity the player will spawn at
- Maximum length: 32 characters
- Used by map transition system and respawn logic

**GetItemBySlot** / **GetActiveWeapon** / **GetInventory** / **HasItem**: Inventory management methods:
- See the "Item/Weapon Management Methods" section below for detailed documentation
- These methods provide access to player's items, weapons, and inventory state

**Example**:
```angelscript
CBasePlayer@ player = PlayerBySteamID("STEAM_0:1:12345");
if (player !is null) {
    player.SetTransitionFields("start", "thornlands", "entrance");
    
    // Adjust JoinType to ensure transition spawn is used
    if (player.GetJoinType() == 2) {  // JN_STARTMAP
        player.SetJoinType(1);  // JN_TRAVEL
    }
}
```

**SendColoredMessage** sends a colored text message to the player's HUD event console:
- `color`: MessageColor enum value determining the message color
- `message`: Message text to display (max 140 characters, auto-truncated with `*` suffix)
- Automatically adds newline if not present
- Replaces legacy script commands: `playermessage`, `rplayermessage`, `gplayermessage`, `bplayermessage`, `yplayermessage`, `dplayermessage`

**SendHUDInfoMessage** sends an info box message to the player's top-left HUD:
- `title`: Title text for the info box (max 120 characters, auto-truncated with `*\n` suffix)
- `message`: Body text for the info box (max 120 characters, auto-truncated with `*\n` suffix)
- Replaces legacy script command: `infomsg <player> <title> <message>`

**Example - Messaging**:
```angelscript
CBasePlayer@ player = PlayerByIndex(1);
if (player !is null) {
    // Send colored messages
    player.SendColoredMessage(MessageColor::Green, "You found a rare item!");
    player.SendColoredMessage(MessageColor::Red, "You took damage!");
    player.SendColoredMessage(MessageColor::Yellow, "Attack dealt 50 damage");
    player.SendColoredMessage(MessageColor::Blue, "Mana restored");
    player.SendColoredMessage(MessageColor::Gray, "Cannot equip this item");
    player.SendColoredMessage(MessageColor::White, "Normal information message");
    
    // Send HUD info box
    player.SendHUDInfoMessage("Quest Complete", "You completed the tutorial quest!");
}
```

### MessageColor Enum
**Scope**: Server

```angelscript
enum MessageColor {
    White = 0,   // Normal color (off-white) - HUDEVENT_NORMAL
    Gray = 1,    // Dark gray - unable/disabled actions - HUDEVENT_UNABLE
    Yellow = 2,  // Yellowish - attack results - HUDEVENT_ATTACK
    Red = 3,     // Red - damage received - HUDEVENT_ATTACKED
    Green = 4,   // Green - positive events - HUDEVENT_GREEN
    Blue = 5     // Blue - info/mana events - HUDEVENT_BLUE
}
```

The MessageColor enum is used with `SendColoredMessage()` to specify HUD message colors:
- **White**: Standard informational messages, general notifications
- **Gray**: Disabled actions (can't pickup, can't equip, etc.)
- **Yellow**: Combat feedback (your attack damage)
- **Red**: Taking damage, negative combat events
- **Green**: Positive events (item pickup, quest complete, healing)
- **Blue**: Special information, mana-related events

---

## Item and Weapon Classes

### CBaseAnimating
**Scope**: Server
*Base class for animated entities*

```angelscript
// Animation methods
int LookupSequence(const string &in label)
void ResetSequenceInfo()
void SetBodygroup(int group, int value)
int GetBodygroup(int group)

// Properties (read-only from script)
float GetFrameRate() const
bool IsSequenceFinished() const
bool IsSequenceLooping() const
```

**LookupSequence** finds a sequence by name:
- Returns sequence index or -1 if not found
- Used for setting specific animations on models

**ResetSequenceInfo** resets the current sequence information:
- Call when switching sequences or models
- Resets timing and frame information

**SetBodygroup** / **GetBodygroup** manage model body groups:
- Body groups allow switching between model variations
- Common for armor, weapons, or character customization

### CBasePlayerItem
**Scope**: Server
*Base class for inventory items*

```angelscript
// Properties
string GetItemName() const
string GetWorldModel() const
string GetHandSpriteName() const
string GetTradeSpriteName() const
CBasePlayer@ GetOwnerPlayer() const
int GetItemID() const
uint GetValue() const
bool IsWielded() const
bool IsUseable() const

// Methods
bool IsMSItem() const
bool CanDrop() const
bool Deploy()
void Holster()
void Materialize()
```

**GetItemName** returns the internal item name (e.g., "dagger", "broadsword"):
- This is the script name used to identify the item type

**GetWorldModel** returns the world model path:
- Used when the item is dropped or placed in the world

**GetOwnerPlayer** returns the player who owns this item:
- Returns `null` if not owned by a player
- Use to check item ownership

**GetValue** returns the item's gold value:
- Used for trading, selling, and economy systems

**IsWielded** checks if the item is currently being held/used:
- Returns `true` if the item is actively equipped

**Deploy** / **Holster** control item equipping:
- `Deploy()` equips the item (makes it active)
- `Holster()` puts the item away

**Example**:
```angelscript
CBasePlayer@ pPlayer = PlayerByIndex(1);
if (pPlayer !is null) {
    CBasePlayerItem@ pItem = pPlayer.GetItemBySlot(0);
    if (pItem !is null) {
        MS_ANGEL_INFO("Item: " + pItem.GetItemName());
        MS_ANGEL_INFO("Value: " + pItem.GetValue() + " gold");
        MS_ANGEL_INFO("Wielded: " + (pItem.IsWielded() ? "Yes" : "No"));
    }
}
```

### CBasePlayerWeapon
**Scope**: Server
*Weapons (extends CBasePlayerItem)*

```angelscript
// Ammo properties
int GetClip() const
void SetClip(int amount)
int GetPrimaryAmmoType() const
int GetSecondaryAmmoType() const

// Attack timing
float GetNextPrimaryAttack() const
void SetNextPrimaryAttack(float time)
float GetNextSecondaryAttack() const
void SetNextSecondaryAttack(float time)
float GetTimeWeaponIdle() const

// State properties
bool IsInReload() const

// Methods
bool CanDeploy()
bool IsUseable()
void SendWeaponAnim(int anim, int skiplocal)
int PrimaryAmmoIndex()
int SecondaryAmmoIndex()
string pszAmmo1()
string pszAmmo2()
```

**GetClip** / **SetClip** manage weapon ammunition:
- `GetClip()` returns current ammunition in the weapon's clip
- `SetClip(amount)` sets the clip ammunition count
- Use for reloading or ammo manipulation

**GetNextPrimaryAttack** / **SetNextPrimaryAttack** control attack timing:
- Time value when the weapon can fire again
- Based on `gpGlobals->time` for server timing

**IsInReload** checks if the weapon is currently reloading:
- Returns `true` during reload animation
- Use to prevent actions during reload

**SendWeaponAnim** plays a weapon animation:
- `anim` is the animation sequence index
- `skiplocal` skips animation for local player (client prediction)

**Example**:
```angelscript
CBasePlayer@ pPlayer = PlayerByIndex(1);
if (pPlayer !is null) {
    CBasePlayerWeapon@ pWeapon = pPlayer.GetActiveWeapon();
    if (pWeapon !is null) {
        MS_ANGEL_INFO("Clip: " + pWeapon.GetClip());
        MS_ANGEL_INFO("In Reload: " + (pWeapon.IsInReload() ? "Yes" : "No"));
        
        // Refill clip
        pWeapon.SetClip(30);
    }
}
```

---

## Item/Weapon Management Methods (CBasePlayer)
**Scope**: Server

The following inventory management methods are available on `CBasePlayer`:

```angelscript
// Get item from specific inventory slot
CBasePlayerItem@ GetItemBySlot(int slot) const

// Get player's currently active weapon
CBasePlayerWeapon@ GetActiveWeapon() const

// Get all items in player's inventory
array<CBasePlayerItem@>@ GetInventory()

// Check if player has specific item by name
bool HasItem(const string &in itemName) const
```

**GetItemBySlot** retrieves an item from a specific inventory slot:
- `slot`: Inventory slot index (0 to MAX_ITEM_TYPES-1)
- Returns `null` if slot is empty
- Use to access specific inventory positions

**GetActiveWeapon** gets the player's currently equipped weapon:
- Returns `null` if no weapon is active or item is not a weapon
- Casts to CBasePlayerWeapon for weapon-specific access

**GetInventory** returns all items the player owns:
- Returns an array of all CBasePlayerItem@ references
- Includes items from all inventory slots
- Empty slots are not included in the array

**HasItem** checks if player owns a specific item by name:
- `itemName`: The item's script name (e.g., "dagger")
- Returns `true` if found in any inventory slot
- Case-sensitive comparison

**Example**:
```angelscript
CBasePlayer@ pPlayer = PlayerByIndex(1);
if (pPlayer !is null) {
    // Check if player has a specific item
    if (pPlayer.HasItem("health_potion")) {
        pPlayer.SendInfoMsg("You have a health potion!");
    }
    
    // Iterate through all player items
    array<CBasePlayerItem@>@ items = pPlayer.GetInventory();
    for (uint i = 0; i < items.length(); i++) {
        CBasePlayerItem@ pItem = items[i];
        MS_ANGEL_INFO("Item " + i + ": " + pItem.GetItemName());
    }
    
    // Get active weapon and check ammo
    CBasePlayerWeapon@ pWeapon = pPlayer.GetActiveWeapon();
    if (pWeapon !is null && pWeapon.GetClip() < 5) {
        pPlayer.SendColoredMessage(MessageColor::Red, "Low ammo!");
    }
    
    // Get item from specific slot
    CBasePlayerItem@ pSlotItem = pPlayer.GetItemBySlot(0);
    if (pSlotItem !is null) {
        MS_ANGEL_INFO("Slot 0 item: " + pSlotItem.GetItemName());
    }
}
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
CBasePlayer@ PlayerBySteamID(const string &in steamID)
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

// Menu system
void OpenVoteMenu(CBasePlayer@ player, const string &in title, const array<string> &in options)

// Spawn management
bool MovePlayerToRandomSpawn(CBasePlayer@ player, float maxDistance)
```

**MovePlayerToRandomSpawn** moves a player to a random spawn point within a specified distance:
- `player`: The player to move
- `maxDistance`: Maximum distance in units to search for spawn points (e.g., 256.0)
- Returns `true` if a spawn point was found and the player was moved
- Returns `false` if no spawn points were found within the distance
- Searches for `ms_player_spawn` entities near the player's current position
- Useful for `/stuck` commands or respawn systems
- Converted from legacy `torandomspawn` script command

**Example**:
```angelscript
// Move player to a random nearby spawn (within 256 units)
if (MovePlayerToRandomSpawn(pPlayer, 256.0f)) {
    pPlayer.SendInfoMsg("Teleported to nearby spawn point");
} else {
    pPlayer.SendInfoMsg("No spawn points found nearby");
}
```

### Server Management Functions
**Scope**: Server

```angelscript
// Execute server console commands
void ExecuteServerCommand(const string &in command)
```

**ExecuteServerCommand** allows scripts to execute server console commands such as:
- `changelevel <mapname>` - Change to a different map
- `kick #<userid>` - Kick a player by user ID
- `banid <minutes> <steamid> kick` - Ban a player by Steam ID
- `sv_password <password>` - Set server password
- `ms_pklevel <level>` - Set PvP level (0=disabled, 1+=enabled)

**Security Notes:**
- Commands like `quit`, `exit`, and `rcon_password` are blocked for security
- Only use trusted input when building command strings
- This function is SERVER_ONLY and cannot be called from client scripts

**Implementation Details:**
- Exposed via `AS_ExecuteServerCommand` in `ASBuiltinFunctions.cpp`
- Uses `SERVER_COMMAND()` and `SERVER_EXECUTE()` internally
- Security validation performed before execution

### GameMaster Communication Functions
**Scope**: Server

```angelscript
// External function calls
void CallPlayerExternal(const string &in playerID, const string &in function, const array<string>@ &in args)
void CallGameMasterExternal(const string &in function, const array<string>@ &in args)

// Player messaging
void SendMessageToAllPlayers(const string &in color, const string &in message)
void SendInfoMessageToAll(const string &in title, const string &in message)
void SendPlayerMessage(const string &in playerName, const string &in title, const string &in message)
void SendConsoleMessage(const string &in playerID, const string &in message)
```

**CallPlayerExternal** calls external functions on player scripts:
- `playerID`: Player's Steam ID
- `function`: Name of the function to call (e.g., "ext_set_map", "ext_setspawn", "ext_changelevel_prep")
- `args`: Array of string arguments to pass to the function

**CallGameMasterExternal** calls external functions on GameMaster scripts:
- `function`: Name of the GameMaster function to call (e.g., "gm_create_vote", "gm_set_global_var")
- `args`: Array of string arguments to pass to the function

**SendMessageToAllPlayers** sends colored chat messages to all players:
- `color`: Color name (red, green, blue, yellow, etc.)
- `message`: Message text to display

**SendInfoMessageToAll** sends info box messages to all players:
- `title`: Title text for the info box
- `message`: Body text for the info box

### Map Validation Functions
**Scope**: Server

```angelscript
// Check if a map exists on the server
bool EngineMapExists(const string &in mapName)
```

**EngineMapExists** checks if a map file exists on the server:
- `mapName`: Map name without .bsp extension (e.g., "crossroads", "thornlands")
- Returns `true` if the map file exists in the maps/ directory
- Returns `false` if the map doesn't exist or the name is invalid
- Uses engine file system to validate actual map file presence
- Server-only function for map transition validation

**Example Usage**:
```angelscript
if (EngineMapExists("crossroads")) {
    ExecuteServerCommand("changelevel crossroads");
} else {
    SendMessageToAllPlayers("red", "Map 'crossroads' not found on server");
}
```

### Entity Management Functions
**Scope**: Server

```angelscript
// Entity creation and manipulation
CBaseEntity@ CreateEntity(const string &in scriptName)
void SetEntityName(CBaseEntity@, const string &in)
void SetEntityTargetName(CBaseEntity@, const string &in)
void SetEntityHealth(CBaseEntity@, float)
bool IsEntityDead(CBaseEntity@)

// Type casting
CBaseEntity@ ToEntity(CBasePlayer@)
CBasePlayer@ ToPlayer(CBaseEntity@)

// Entity string conversion
CBaseEntity@ StringToEntity(const string &in)
CBasePlayer@ StringToPlayer(const string &in)

// Entity spawning functions
CBaseEntity@ SpawnNPC(const string &in scriptName, const Vector3 &in position, const array<string>@ params = null)
CBaseEntity@ SpawnItem(const string &in scriptName, const Vector3 &in position, const array<string>@ params = null)
```

**SpawnNPC** creates an NPC entity at the specified position:
- `scriptName`: Script name of the NPC to spawn (e.g., "game_master", "skeleton")
- `position`: 3D coordinates where the NPC should spawn
- `params`: Optional array of additional parameters (reserved for future use)
- Returns: Pointer to the spawned CBaseEntity, or null if creation failed
- Server-only function

**SpawnItem** creates an item entity at the specified position:
- `scriptName`: Script name of the item to spawn (e.g., "potion_health", "sword_short")
- `position`: 3D coordinates where the item should spawn
- `params`: Optional array of additional parameters (reserved for future use)
- Returns: Pointer to the spawned CBaseEntity, or null if creation failed
- Server-only function

**Example**:
```angelscript
// Spawn a skeleton at coordinates (100, 200, 0)
CBaseEntity@ pSkeleton = SpawnNPC("skeleton", Vector3(100, 200, 0));
if (pSkeleton !is null) {
    LogMessage("Skeleton spawned at index " + pSkeleton.GetEntIndex());
}

// Spawn a health potion
CBaseEntity@ pPotion = SpawnItem("potion_health", Vector3(150, 250, 10));
```

**StringToEntity** converts an entity string to a CBaseEntity pointer:
- Entity strings are in the format `"PentP(index,address)"` (e.g., `"PentP(1,12345678)"`)
- Returns `null` if the string is invalid or the entity no longer exists
- Validates both the entity index and memory address for safety
- Used when C++ passes entity references as strings to scripts

**StringToPlayer** converts an entity string to a CBasePlayer pointer:
- Uses `StringToEntity` internally and casts to player
- Returns `null` if the entity is not a valid player
- Convenient for converting player entity strings from C++ events

**Example**:
```angelscript
// Event handler receiving entity string from C++
void GamePlayerPutInWorld(const string &in entityString) {
    CBasePlayer@ pPlayer = StringToPlayer(entityString);
    if (pPlayer !is null) {
        LogMessage("Player spawned: " + pPlayer.GetName());
    }
}
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

The engine event system allows scripts to respond to game events by implementing global functions with specific names. The C++ engine will call these functions at appropriate times.

### Player Lifecycle Events
**Scope**: Server

```angelscript
// Called when player spawns in world (after character selection)
void GamePlayerPutInWorld()
void GamePlayerPutInWorld(CBasePlayer@ pPlayer)
void GamePlayerPutInWorld(const string &in playerIdentifier)

// Called when player respawns after death
void GameRespawn(CBasePlayer@ pPlayer)

// Called when the server activates (map load complete)
void ServerActivate()
```

**ServerActivate** is called after all map entities have been spawned and activated:
- Called once per map load, after entity activation completes
- Ideal for initializing game systems, spawning dynamic entities
- GameMaster scripts should implement this to spawn the game_master NPC
- Called from C++ `client.cpp:ServerActivate()` via `CallGlobalFunctionWithParams`
- Replaces the legacy C++ game_master spawning code

**Example - GameMaster Implementation**:
```angelscript
void ServerActivate() {
    LogMessage("Server activated, spawning game_master...");
    
    // Spawn game_master NPC at far coordinates
    CBaseEntity@ pGameMaster = SpawnNPC("game_master", Vector3(20000, -10000, -20000));
    
    if (pGameMaster !is null) {
        // Configure game_master properties
        pGameMaster.SetNetName("-game_master");  // Required for C++ lookup
        pGameMaster.SetHealth(1.0f);
        pGameMaster.SetRenderMode(kRenderTransTexture);
        pGameMaster.SetRenderAmount(0);  // Invisible
        pGameMaster.SetGodMode(true);
        pGameMaster.SetTakeDamage(DAMAGE_NO);
        
        LogMessage("Game master spawned and configured successfully");
    } else {
        LogMessage("ERROR: Failed to spawn game_master");
    }
}
```

**GamePlayerPutInWorld** is called when a player enters the world after character selection (not during respawn):
- Called from C++ `player.cpp:2730` via `CallScriptEvent`
- The entity string version receives `"PentP(index,address)"` format from `EntToString(this)`
- The version with `CBasePlayer@` can be called directly with a player object  
- The version with string identifier can look up players by Steam ID or index
- Implement this function globally in your AngelScript module to handle player spawn events
- See `PlayerEvents.as` for the reference implementation

**GameRespawn** is called when a player respawns after death:
- Implement this function globally to handle post-death respawn logic
- Different from `GamePlayerPutInWorld` which is for initial world entry

**Reference Implementation** (`PlayerEvents.as`):
The official implementation handles:
- **Christmas Mode**: Seasonal events on specific maps (edana, deralia, helena)
- **Random Spawn**: Moves player to random nearby spawn point for anti-stuck
- **Transition Data**: Restores spawn points from quest data (key: "d")
- **Home Position**: Saves player's home coordinates to quest data
- **First Join**: Initializes new players (sets `PLR_IN_WORLD` quest flag)
- **Dark Level**: Restores dark level from quest data (key: "dl")
- **Meta Perks**: Donator/dev halos and trollcano checks

**Example**:
```angelscript
void GamePlayerPutInWorld(const string &in entityString) {
    // Convert entity string to player
    CBasePlayer@ pPlayer = StringToPlayer(entityString);
    if (pPlayer is null) return;
    
    string steamID = pPlayer.GetSteamID();
    
    // Handle player spawn logic
    LogMessage("Player " + pPlayer.GetName() + " entered the world");
    
    // Set transition data from quest data
    string trans = GetPlayerQuestData(steamID, "d");
    if (trans.length() > 0) {
        pPlayer.SetTransitionFields(trans, GetMapName(), trans);
    }
    
    // Set home position
    Vector3 pos = pPlayer.GetOrigin();
    string posStr = pos.x + "," + pos.y + "," + pos.z;
    SetPlayerQuestData(steamID, "MY_HOME", posStr);
}
```

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