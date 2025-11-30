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
CMSMonster@ SpawnNPC(const string &in scriptName, const Vector3 &in position, const array<string>@ params = null)
CBasePlayerItem@ SpawnItem(const string &in scriptName, const Vector3 &in position, const array<string>@ params = null)
```

**SpawnNPC** creates an NPC entity at the specified position:
- `scriptName`: Script name of the NPC to spawn (e.g., "game_master", "skeleton")
- `position`: 3D coordinates where the NPC should spawn
- `params`: Optional array of additional parameters (reserved for future use)
- Returns: Pointer to the spawned CMSMonster, or null if creation failed
- Server-only function

**SpawnItem** creates an item entity at the specified position:
- `scriptName`: Script name of the item to spawn (e.g., "potion_health", "sword_short")
- `position`: 3D coordinates where the item should spawn
- `params`: Optional array of additional parameters (reserved for future use)
- Returns: Pointer to the spawned CBasePlayerItem, or null if creation failed
- Server-only function

**Example**:
```angelscript
// Spawn a skeleton at coordinates (100, 200, 0)
CMSMonster@ pSkeleton = SpawnNPC("skeleton", Vector3(100, 200, 0));
if (pSkeleton !is null) {
    LogMessage("Skeleton spawned at index " + pSkeleton.entindex());
    // Now you have direct access to CMSMonster properties
    pSkeleton.m_HP = 100.0f;
    pSkeleton.Title = "Elite Skeleton";
}

// Spawn a health potion (returns CBasePlayerItem)
CBasePlayerItem@ pPotion = SpawnItem("potion_health", Vector3(150, 250, 10));
if (pPotion !is null) {
    LogMessage("Item spawned: " + pPotion.GetItemName());
    // Access item-specific properties
    uint value = pPotion.GetValue();
    LogMessage("Item value: " + value + " gold");
}
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
    
    // Spawn game_master NPC at far coordinates (returns CMSMonster@)
    CMSMonster@ pGameMaster = SpawnNPC("game_master", Vector3(20000, -10000, -20000), null, Angel);
    
    if (pGameMaster !is null) {
        // Configure game_master properties
        pGameMaster.SetNetName("-game_master");  // Required for C++ lookup
        pGameMaster.SetHealth(1.0f);
        pGameMaster.SetRenderMode(kRenderTransTexture);
        pGameMaster.SetRenderAmount(0);  // Invisible
        pGameMaster.SetGodMode(true);
        pGameMaster.SetTakeDamage(DAMAGE_NO);
        
        // Set MS-specific properties
        pGameMaster.Title = "Game Master";
        pGameMaster.m_Menu_Autoopen = false;
        
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

## Monster Classes

**Inheritance Chain**: `CBaseEntity` → `CBaseMonster` → `CMSMonster` → `CBasePlayer`

All monster classes inherit methods and properties from their parent classes. For example, `CMSMonster` has access to all `CBaseEntity` methods like `SetNetName()`, `SetHealth()`, `GetClassName()`, etc.

### CBaseMonster
**Scope**: Server
*Base class for all monsters (inherits from CBaseEntity)*

```angelscript
// Properties (direct member access)
float m_flFieldOfView        // Monster's view cone width (dot product, e.g. 0.5)
float m_flWaitFinished       // Time when current wait expires
float m_flMoveWaitFinished   // Time when movement wait expires
int m_LastHitGroup           // Last body region that took damage
float m_flNextAttack         // Time when can attack again
int m_bloodColor             // Blood particle color value
float m_flDistTooFar         // Enemy too far distance threshold
float m_flDistLook           // Monster sight distance (default 2048)
int m_iMaxHealth             // Maximum health value
int m_afCapability           // Capability flags (bits_CAP_*)
CBaseEntity@ Enemy           // Current enemy entity (virtual property)
CBaseEntity@ TargetEnt       // Current target entity (virtual property)
Vector3 EnemyLKP             // Enemy last known position (virtual property)

// Core AI Methods
int BloodColor()                        // Get blood color value
void Look(int distance)                 // Look for targets within distance
void RunAI()                            // Execute AI tick
bool IsAlive()                          // Check if monster is alive
void MonsterThink()                     // AI think function
int IRelationship(CBaseEntity@ target) // Get relationship to target
void MonsterInit()                      // Initialize monster
void StartMonster()                     // Activate monster
CBaseEntity@ BestVisibleEnemy()        // Find best visible enemy
bool FInViewCone(CBaseEntity@ entity)  // Check if entity in view cone
bool FInViewCone(const Vector3 &in)    // Check if position in view cone
void SetState(int state)                // Set monster state
void ReportAIState()                    // Debug AI state to console
int CheckEnemy(CBaseEntity@ enemy)     // Validate enemy
void PushEnemy(CBaseEntity@ enemy, const Vector3 &in lastPos) // Save enemy
bool PopEnemy()                         // Restore previous enemy
void ClearSchedule()                    // Clear current schedule
float ChangeYaw(int speed)              // Rotate yaw toward target
bool FacingIdeal()                      // Check if facing ideal direction
Vector3 BodyTarget(const Vector3 &in)   // Get aim point on body

// Condition Management
void SetConditions(int conditions)      // Set condition flags
void ClearConditions(int conditions)    // Clear condition flags
bool HasConditions(int conditions)      // Check condition flags
bool HasAllConditions(int conditions)   // Check all condition flags

// Memory Management
void Remember(int memory)               // Set memory flag
void Forget(int memory)                 // Clear memory flag
bool HasMemory(int memory)              // Check memory flag
bool HasAllMemories(int memory)         // Check all memory flags

// Task Management
void TaskComplete()                     // Mark current task complete
void TaskFail()                         // Mark current task failed
void TaskBegin()                        // Begin current task

// Movement
bool IsMoving()                         // Check if monster is moving
void Stop()                             // Stop movement
```

**MonsterState Enum Values**:
- `MONSTERSTATE_NONE` (0) - No state
- `MONSTERSTATE_IDLE` (1) - Idle/passive
- `MONSTERSTATE_COMBAT` (2) - In combat
- `MONSTERSTATE_ALERT` (3) - Alert/investigating
- `MONSTERSTATE_HUNT` (4) - Hunting enemy
- `MONSTERSTATE_PRONE` (5) - Prone/knocked down
- `MONSTERSTATE_SCRIPT` (6) - Scripted sequence
- `MONSTERSTATE_PLAYDEAD` (7) - Playing dead
- `MONSTERSTATE_DEAD` (8) - Dead

**Example - Basic Monster AI Control**:
```angelscript
void ControlMonsterAI() {
    // Get a monster entity (example assumes you have one)
    CBaseEntity@ entity = /* get monster entity */;
    CBaseMonster@ monster = ToMonster(entity);
    
    if (monster !is null) {
        // Configure monster AI
        monster.m_flFieldOfView = 0.7f;  // Narrower view cone
        monster.m_flDistLook = 1024.0f;  // Shorter sight range
        
        // Set monster to combat state
        monster.SetState(MONSTERSTATE_COMBAT);
        
        // Find and engage enemy
        CBaseEntity@ enemy = monster.BestVisibleEnemy();
        if (enemy !is null) {
            monster.Enemy = enemy;
            MS_ANGEL_INFO("Monster engaged enemy");
        }
        
        // Check conditions
        if (monster.HasConditions(bits_COND_SEE_ENEMY)) {
            MS_ANGEL_INFO("Monster sees enemy");
        }
        
        // Remember something
        monster.Remember(bits_MEMORY_PROVOKED);
    }
}
```

### CMSMonster
**Scope**: Server
*Master Sword monster class (inherits from CBaseMonster, which inherits from CBaseEntity)*

**Note**: CMSMonster has full access to all `CBaseEntity` methods (e.g., `SetNetName()`, `GetClassName()`, `SetHealth()`, `SetRenderMode()`) and all `CBaseMonster` methods (e.g., `Look()`, `SetState()`, `BestVisibleEnemy()`).

```angelscript
// Properties (direct member access)
float m_HP                   // Current health points
float m_MP                   // Current mana points
int m_Gold                   // Gold amount carried
float m_StepSize             // Maximum step height
float m_SpeedMultiplier      // Speed modifier (1.0 = normal)
float m_TurnRate             // Turn speed ratio (0.0-1.0)
float m_Width                // Monster width for collision
float m_Height               // Monster height for collision
uint8 m_Gender               // Monster gender (Gender enum as uint8)
string Title                 // Display title/name (virtual property, msstring wrapper)
string ScriptName            // Script file name (virtual property, msstring wrapper)
int m_Lives                  // Remaining lives (respawns)
float m_SkillLevel           // Experience worth
bool m_Menu_Autoopen         // Auto-open menu on +use
float m_HITMulti             // Hit chance multiplier
float m_HPMulti              // HP multiplier
float m_DMGMulti             // Damage output multiplier
float m_HearingSensitivity   // Sound hearing sensitivity

// Lifecycle Methods
bool IsMSMonster()           // Check if this is an MSMonster
float MaxHP()                // Get effective maximum HP
float MaxMP()                // Get effective maximum MP
bool IsAlive()               // Check if alive

// Economy Methods
int GiveGold(int amount)                 // Give/take gold (returns new amount)
int GiveGold(int amount, bool verbose)   // With verbose messages
float GiveHP(float amount)               // Give/take HP (returns new HP)
float GiveMP(float amount)               // Give/take MP (returns new MP)

// Stats System
int GetNatStat(int stat)                 // Get natural stat value
int GetSkillStat(int stat)               // Get skill stat value
int GetSkillStatCount()                  // Get number of skill stats

// Movement
float WalkSpeed()                        // Get walk speed
float RunSpeed()                         // Get run speed
bool IsFlying()                          // Check if flying/swimming

// Combat
bool IsActing()                          // Check if attacking/acting
bool IsShielding()                       // Check if blocking with shield
void CancelAttack()                      // Cancel current attack

// Miscellaneous
float Weight()                           // Get total weight (gear + items)
void SetSpeed()                          // Recalculate speed
```

**Gender Enum Values**:
- `GENDER_MALE` (0) - Male
- `GENDER_FEMALE` (1) - Female
- `GENDER_UNKNOWN` (2) - Unknown/neutral

**SpeechType Enum Values**:
- `SPEECH_GLOBAL` (0) - Global chat
- `SPEECH_LOCAL` (1) - Local range (300 units)
- `SPEECH_PARTY` (2) - Party only

**Example - Monster Management**:
```angelscript
void ManageMSMonster() {
    // Get an MS monster entity
    CBaseEntity@ entity = /* get monster entity */;
    CMSMonster@ monster = ToMSMonster(entity);
    
    if (monster !is null) {
        // Set properties
        monster.Title = "Elite Guardian";
        monster.m_SpeedMultiplier = 1.5f;
        monster.m_HPMulti = 2.0f;
        monster.m_DMGMulti = 1.3f;
        
        // Give rewards
        monster.GiveHP(50.0f);   // Heal 50 HP
        monster.GiveMP(25.0f);   // Restore 25 MP
        monster.GiveGold(100);   // Add 100 gold
        
        // Check stats
        float currentHP = monster.m_HP;
        float maxHP = monster.MaxHP();
        float hpPercent = (currentHP / maxHP) * 100.0f;
        
        MS_ANGEL_INFO("Monster HP: " + currentHP + "/" + maxHP + " (" + hpPercent + "%)");
        MS_ANGEL_INFO("Gold: " + monster.m_Gold);
        
        // Check combat state
        if (monster.IsActing()) {
            MS_ANGEL_INFO("Monster is attacking");
        }
        
        if (monster.IsShielding()) {
            MS_ANGEL_INFO("Monster is blocking");
        }
        
        // Get stats
        int strength = monster.GetNatStat(STAT_STRENGTH);
        MS_ANGEL_INFO("Strength: " + strength);
    }
}
```

**Example - Monster Casting**:
```angelscript
void CastMonsterTypes() {
    CBaseEntity@ entity = /* get entity */;
    
    // Cast to CBaseMonster
    CBaseMonster@ monster = ToMonster(entity);
    if (monster !is null) {
        MS_ANGEL_INFO("Entity is a monster");
        monster.Look(1024);
    }
    
    // Cast to CMSMonster
    CMSMonster@ msMonster = ToMSMonster(entity);
    if (msMonster !is null) {
        MS_ANGEL_INFO("Entity is an MS monster");
        MS_ANGEL_INFO("HP: " + msMonster.m_HP + "/" + msMonster.MaxHP());
    }
    
    // Cast monster to entity
    CBaseEntity@ backToEntity = ToEntity(monster);
}
```

---

## Client-Side Classes

**NOTE**: All classes and functions in this section are available ONLY in client-side scripts (marked with `#scope client` in legacy scripts or loaded as client modules in AngelScript).

### Class Hierarchy

Client classes follow three main categories:

**Entity Hierarchy:**
```
CClientEntity (base)
  └── CLocalPlayer
```

**Effect Hierarchy:**
```
CClientEffect (base)
  ├── CTempEntity
  ├── CDynamicLight
  └── CBeam
```

**Item Classes:**
```
CGenericItem (standalone, read-only item data)
```

- **CLocalPlayer** inherits all entity query methods from **CClientEntity** (GetOrigin, GetAngles, GetVelocity, etc.)
- All effect classes inherit the `IsValid()` method from **CClientEffect**
- **CGenericItem** provides read-only access to item data for UI/display purposes

### CClientEffect (Base Class)
**Scope**: Client

Base class for all client-side visual effects.

```angelscript
class CClientEffect {
    bool IsValid() const;  // Check if effect is valid
}
```

This class is inherited by `CTempEntity`, `CDynamicLight`, and `CBeam`.

### CClientEntity
**Scope**: Client

Read-only wrapper for client-side entity queries. This is the base class for `CLocalPlayer`.

```angelscript
class CClientEntity {
    bool Exists() const;                   // Check if entity exists
    int GetIndex() const;                  // Get entity index
    Vector3 GetOrigin() const;             // Get position
    Vector3 GetAngles() const;             // Get angles
    Vector3 GetVelocity() const;           // Get velocity
    string GetModelName() const;           // Get model path
    int GetModelIndex() const;             // Get model index
    int GetRenderMode() const;             // Get render mode
    int GetRenderAmount() const;           // Get render amount
    Color GetRenderColor() const;          // Get render color
    bool IsPlayer() const;                 // Check if player entity
    bool IsVisible() const;                // Check if visible
    Vector3 GetBonePosition(int) const;    // Get bone position
    int GetBoneCount() const;              // Get bone count
    Vector3 GetAttachment(int) const;      // Get attachment position
    int GetSequence() const;               // Get animation sequence
    float GetFrame() const;                // Get animation frame
    int GetBody() const;                   // Get body group
    int GetSkin() const;                   // Get skin
}

// Global accessor
CClientEntity@ GetClientEntity(int index)
```

**Example**:
```angelscript
void QueryEntity(int index) {
    CClientEntity@ ent = GetClientEntity(index);
    if (ent.Exists()) {
        MS_ANGEL_INFO("Entity: " + ent.GetModelName());
        MS_ANGEL_INFO("Position: " + ent.GetOrigin().x);
        MS_ANGEL_INFO("Is Player: " + ent.IsPlayer());
    }
}
```

### CLocalPlayer
**Scope**: Client

The local player represents the player running the client. Inherits from `CClientEntity`.

```angelscript
class CLocalPlayer : CClientEntity {
    // Inherited from CClientEntity:
    // int GetIndex() const;
    // Vector3 GetOrigin() const;
    // Vector3 GetAngles() const;
    // Vector3 GetVelocity() const;
    // string GetModelName() const;
    // ... and all other CClientEntity methods
    
    // Local player specific methods:
    Vector3 GetViewAngles() const;         // Get view angles (camera)
    bool IsThirdPerson() const;            // Check if in third-person mode
    bool IsUnderwater() const;             // Check if underwater
    Vector3 GetWaterOrigin() const;        // Get water surface position
    bool CanAttack() const;                // Check if can attack
    bool CanJump() const;                  // Check if can jump
    bool CanDuck() const;                  // Check if can duck
    bool CanRun() const;                   // Check if can run
    bool CanMove() const;                  // Check if can move
    int GetWaterLevel() const;             // Get water level (0-3)
}

// Global accessor
CLocalPlayer@ GetLocalPlayer()
```

**Example**:
```angelscript
void CheckLocalPlayer() {
    CLocalPlayer@ player = GetLocalPlayer();
    
    // Can use inherited CClientEntity methods
    Vector3 pos = player.GetOrigin();
    Vector3 angles = player.GetAngles();
    string model = player.GetModelName();
    
    MS_ANGEL_INFO("Player at: " + pos.x + "," + pos.y + "," + pos.z);
    MS_ANGEL_INFO("Player model: " + model);
    
    // Can use local player specific methods
    if (player.IsUnderwater()) {
        MS_ANGEL_INFO("Player is underwater!");
    }
    
    Vector3 viewAngles = player.GetViewAngles();
    MS_ANGEL_INFO("Looking at: " + viewAngles.y + " degrees");
}
```

### CTempEntity
**Scope**: Client

Temporary entity for client-side visual effects. Inherits from `CClientEffect`.

```angelscript
class CTempEntity : CClientEffect {
    // Inherited from CClientEffect:
    // bool IsValid() const;
    
    // Transform
    void SetOrigin(const Vector3 &in);
    Vector3 GetOrigin() const;
    void SetAngles(const Vector3 &in);
    Vector3 GetAngles() const;
    void SetVelocity(const Vector3 &in);
    Vector3 GetVelocity() const;
    
    // Appearance
    void SetModel(const string &in);
    void SetSprite(const string &in);
    void SetScale(float);
    float GetScale() const;
    void SetFrame(int);
    int GetFrame() const;
    void SetFrameRate(float);
    void SetBody(int);
    void SetSkin(int);
    
    // Rendering
    void SetRenderMode(int);
    void SetRenderAmount(int);
    void SetRenderColor(const Color &in);
    void SetRenderFx(int);
    
    // Physics
    void SetGravity(float);
    void SetCollisionMode(CollisionMode);
    void SetBounceFactor(float);
    void SetMins(const Vector3 &in);
    void SetMaxs(const Vector3 &in);
    
    // Lifetime
    void SetDeathDelay(float);
    void SetFadeout(bool, float);
    void Kill();
    
    // Following
    void FollowEntity(int entityIndex, int attachment = -1);
    void StopFollowing();
    
    // Custom data
    void SetUserInt(int slot, int value);
    int GetUserInt(int slot) const;
    void SetUserFloat(int slot, float value);
    float GetUserFloat(int slot) const;
    
    // Callbacks (TODO)
    void SetUpdateCallback(const string &in);
    void SetCollisionCallback(const string &in);
    void SetWaterCallback(const string &in);
    void SetTimerCallback(float, const string &in);
}

enum CollisionMode {
    None,
    World,
    All,
    AllAndDie
}

// Factory functions
CTempEntity@ CreateTempSprite(const string &in spriteName, const Vector3 &in origin);
CTempEntity@ CreateTempModel(const string &in modelName, const Vector3 &in origin);
CTempEntity@ CreateFrameSprite(const string &in spriteName, const Vector3 &in origin);
CTempEntity@ CreateFrameModel(const string &in modelName, const Vector3 &in origin);
```

**Example**:
```angelscript
void CreateExplosionEffect() {
    Vector3 pos = Vector3(100, 200, 50);
    CTempEntity@ sprite = CreateTempSprite("sprites/fire.spr", pos);
    
    if (sprite.IsValid()) {
        sprite.SetScale(2.0);
        sprite.SetFrameRate(15.0);
        sprite.SetRenderMode(kRenderTransAdd);
        sprite.SetRenderAmount(200);
        sprite.SetDeathDelay(1.0);
        sprite.SetFadeout(true, 0.5);
    }
}
```

### CDynamicLight
**Scope**: Client

Client-side dynamic lighting. Inherits from `CClientEffect`.

```angelscript
class CDynamicLight : CClientEffect {
    // Inherited from CClientEffect:
    // bool IsValid() const;
    void SetOrigin(const Vector3 &in);
    Vector3 GetOrigin() const;
    void SetRadius(float);
    void SetColor(const Color &in);
    void SetDuration(float);
    void SetDark(bool);              // Dark light (removes light)
    void FollowEntity(int entityIndex);
    void Kill();
}

// Factory function
CDynamicLight@ CreateDynamicLight(const Vector3 &in origin, float radius, 
                                   const Color &in color, float duration = 0.0f);
```

**Example**:
```angelscript
void CreateTorchLight() {
    Vector3 pos = Vector3(100, 200, 50);
    Color orange = Color(255, 128, 0, 255);
    CDynamicLight@ light = CreateDynamicLight(pos, 256.0, orange, 5.0);
    
    if (light.IsValid()) {
        // Light will last 5 seconds
        light.SetRadius(300.0);  // Make it brighter
    }
}
```

### CBeam
**Scope**: Client

Client-side beam effects. Inherits from `CClientEffect`.

```angelscript
class CBeam : CClientEffect {
    // Inherited from CClientEffect:
    // bool IsValid() const;
    void SetStartPos(const Vector3 &in);
    void SetEndPos(const Vector3 &in);
    void SetStartEntity(int entityIndex, int attachment);
    void SetEndEntity(int entityIndex, int attachment);
    void SetSprite(const string &in);
    void SetWidth(float);
    void SetAmplitude(float);
    void SetBrightness(float);
    void SetSpeed(float);
    void SetFrameRate(float);
    void SetColor(const Color &in);
    void SetLife(float);
    void Kill();
}

// Factory functions
CBeam@ CreateBeamPoints(const Vector3 &in start, const Vector3 &in end, const string &in sprite);
CBeam@ CreateBeamEntities(int startIdx, int startAttach, int endIdx, int endAttach, const string &in sprite);
CBeam@ CreateBeamEntPoint(int startIdx, int attachment, const Vector3 &in endPos, const string &in sprite);

// Effect utilities
void CreateSpark(const Vector3 &in origin);
void CreateSparkOnModel(int entityIndex, int attachment = 0);
void CreateDecal(int decalIndex, const Vector3 &in traceStart, const Vector3 &in traceEnd);
```

**Example**:
```angelscript
void CreateLightningBeam() {
    Vector3 start = Vector3(0, 0, 100);
    Vector3 end = Vector3(100, 100, 0);
    CBeam@ beam = CreateBeamPoints(start, end, "sprites/laserbeam.spr");
    
    if (beam.IsValid()) {
        beam.SetWidth(20.0);
        beam.SetColor(Color(100, 100, 255, 255));
        beam.SetBrightness(200.0);
        beam.SetLife(2.0);
    }
}
```

### CClientEnvironment
**Scope**: Client

Client-side environment control (fog, screen tint, sky).

```angelscript
class CClientEnvironment {
    // Fog control
    void SetFogEnabled(bool);
    bool GetFogEnabled() const;
    void SetFogColor(const Color &in);
    Color GetFogColor() const;
    void SetFogDensity(float);
    void SetFogStart(float);
    void SetFogEnd(float);
    void SetFogType(int);
    
    // Screen effects
    void SetScreenTint(const Color &in);
    Color GetScreenTint() const;
    void ClearScreenTint();
    
    // Sky
    void SetSkyTexture(const string &in);
    string GetSkyName() const;
    void SetLightGamma(float);
    void SetMaxViewDistance(float);
}

// Global accessor
CClientEnvironment@ GetEnvironment()
```

**Example**:
```angelscript
void SetDarkAtmosphere() {
    CClientEnvironment@ env = GetEnvironment();
    
    // Dark red screen tint
    env.SetScreenTint(Color(50, 0, 0, 100));
    
    // Enable fog
    env.SetFogEnabled(true);
    env.SetFogColor(Color(20, 0, 0, 255));
    env.SetFogDensity(0.002);
}
```

### CClientSound
**Scope**: Client

Client-side audio playback.

```angelscript
class CClientSound {
    void PlaySound(int channel, const string &in soundPath, float volume = 1.0f);
    void SetVolume(int channel, const string &in soundPath, float volume);
    void StopSound(int channel, const string &in soundPath);
    void PlaySoundAtPosition(const Vector3 &in pos, const string &in soundPath, float volume = 1.0f);
}

// Global accessor
CClientSound@ GetClientSound()

// Sound channel constants
const int SOUND_CHANNEL_WEAPON = 0;
const int SOUND_CHANNEL_VOICE = 1;
const int SOUND_CHANNEL_STREAM = 5;
```

**Example**:
```angelscript
void PlayAmbientSound() {
    CClientSound@ sound = GetClientSound();
    sound.PlaySound(SOUND_CHANNEL_STREAM, "ambient/wind.wav", 0.5);
}
```

### Client Utility Functions
**Scope**: Client

```angelscript
// Trace/query functions
Vector3 GetGroundHeight(const Vector3 &in origin);
Vector3 GetSkyHeight(const Vector3 &in origin);
bool IsUnderSky(const Vector3 &in origin);
Vector3 TraceLine(const Vector3 &in start, const Vector3 &in end, bool worldOnly = false);
int TraceLineEntity(const Vector3 &in start, const Vector3 &in end);
int GetContents(const Vector3 &in origin);

// Client-specific accessors
float GetClientTime();
string GetCurrentMap();
bool IsClientSide();  // Always returns true on client
array<int>@ GetNearbyEntities(const Vector3 &in origin, float radius, bool playersOnly = false);
```

**Example**:
```angelscript
void CheckGround() {
    CLocalPlayer@ player = GetLocalPlayer();
    Vector3 playerPos = player.GetOrigin();
    Vector3 ground = GetGroundHeight(playerPos);
    
    float distToGround = playerPos.z - ground.z;
    MS_ANGEL_INFO("Distance to ground: " + distToGround);
    
    if (IsUnderSky(playerPos)) {
        MS_ANGEL_INFO("Player is under open sky");
    }
}
```

---

### CGenericItem
**Scope**: Client

Read-only access to item properties for client-side UI, display, and visualization. This class provides safe access to item data without exposing game logic.

**Note**: CGenericItem provides read-only methods only. All game logic (use, drop, equip, etc.) must be handled server-side.

```angelscript
class CGenericItem {
    // Basic Properties
    string GetItemName() const;        // Internal item name (e.g., "dagger")
    int GetItemID() const;             // Unique item instance ID
    int GetQuality() const;            // Current item quality/durability
    int GetMaxQuality() const;         // Maximum quality value
    int GetQuantity() const;           // Stack quantity (for groupable items)
    int GetMaxQuantity() const;        // Maximum stack size
    float GetWeight() const;           // Item weight
    uint GetValue() const;             // Gold value
    int GetLocation() const;           // Item location (ITEMPOS_* constants)
    int GetHand() const;               // Which hand (LEFT_HAND/RIGHT_HAND)

    // Visual/Rendering Properties
    string GetViewModel() const;       // View model path (1st person)
    string GetPlayerHoldModel() const; // Player hold model (3rd person)
    string GetAnimExtension() const;   // Animation extension
    string GetAnimExtensionLegs() const; // Leg animation extension
    int GetRenderMode() const;         // Render mode (kRender*)
    int GetRenderAmount() const;       // Render amount (0-255)
    int GetRenderFx() const;           // Render effects
    int GetSkin() const;               // Model skin index
    int GetViewModelAnim() const;      // Current view model animation
    float GetViewModelAnimSpeed() const; // View model animation speed

    // Item Type Checks
    bool IsWorn() const;               // Is currently worn/equipped
    bool IsArmor() const;              // Is armor piece
    bool IsWearable() const;           // Can be worn
    bool IsShield() const;             // Is a shield
    bool IsGroupable() const;          // Can stack with others
    bool IsContainer() const;          // Can hold other items
    bool IsProjectile() const;         // Is a projectile/ammo
    bool IsDrinkable() const;          // Can be consumed as drink
    bool IsPerishable() const;         // Has expiration/decay
    bool IsSpell() const;              // Is a spell scroll/tome

    // Container Methods (for container items)
    int Container_ItemCount() const;   // Number of items in container
    float Container_Weight() const;    // Total weight of contents
    bool Container_IsOpen() const;     // Is container open
    float Volume() const;              // Item volume

    // Attack Information
    int GetAttackCount() const;        // Number of attack modes
}
```

**Location Constants**:
```angelscript
const int ITEMPOS_NONE = 0;
const int ITEMPOS_HANDS = 1;
const int ITEMPOS_BODY = 2;
```

**Hand Constants**:
```angelscript
const int LEFT_HAND = 0;
const int RIGHT_HAND = 1;
```

**Example - Displaying Item Info in UI**:
```angelscript
void DisplayItemTooltip(CGenericItem@ pItem) {
    if (pItem is null) return;

    string tooltip = "=== " + pItem.GetItemName() + " ===\n";
    tooltip += "Value: " + pItem.GetValue() + " gold\n";
    tooltip += "Weight: " + pItem.GetWeight() + " kg\n";
    tooltip += "Quality: " + pItem.GetQuality() + "/" + pItem.GetMaxQuality() + "\n";

    // Check item type
    if (pItem.IsWearable()) {
        tooltip += "Type: Wearable\n";
        tooltip += "Equipped: " + (pItem.IsWorn() ? "Yes" : "No") + "\n";
    }
    if (pItem.IsContainer()) {
        tooltip += "Container: " + pItem.Container_ItemCount() + " items\n";
        tooltip += "Total Weight: " + pItem.Container_Weight() + " kg\n";
    }
    if (pItem.IsGroupable()) {
        tooltip += "Quantity: " + pItem.GetQuantity() + "/" + pItem.GetMaxQuantity() + "\n";
    }

    MS_ANGEL_INFO(tooltip);
}
```

**Example - Filtering Items by Type**:
```angelscript
void ShowWeapons(array<CGenericItem@>@ items) {
    for (uint i = 0; i < items.length(); i++) {
        CGenericItem@ pItem = items[i];

        // Check if item has attack capability
        if (pItem.GetAttackCount() > 0) {
            MS_ANGEL_INFO("Weapon: " + pItem.GetItemName());
            MS_ANGEL_INFO("  Attacks: " + pItem.GetAttackCount());
            MS_ANGEL_INFO("  Hand: " + (pItem.GetHand() == LEFT_HAND ? "Left" : "Right"));
        }
    }
}
```

**Example - Rendering Information**:
```angelscript
void CheckItemVisuals(CGenericItem@ pItem) {
    if (pItem is null) return;

    MS_ANGEL_INFO("View Model: " + pItem.GetViewModel());
    MS_ANGEL_INFO("World Model: " + pItem.GetPlayerHoldModel());
    MS_ANGEL_INFO("Render Mode: " + pItem.GetRenderMode());
    MS_ANGEL_INFO("Skin: " + pItem.GetSkin());

    // Check transparency
    if (pItem.GetRenderMode() == kRenderTransAlpha) {
        int alpha = pItem.GetRenderAmount();
        MS_ANGEL_INFO("Item is " + (alpha < 128 ? "mostly transparent" : "mostly opaque"));
    }
}
```

---

## VGUI System (Client UI)
**Scope**: Client

The VGUI (Valve GUI) system allows client-side scripts to create custom user interfaces with interactive elements like buttons, labels, text fields, and frames.

### Class Hierarchy

```
VGUIPanel (base class)
  ├── VGUIFrame (windowed panels with title bars)
  ├── VGUIButton (clickable buttons)
  ├── VGUILabel (text display)
  ├── VGUITextEntry (text input fields)
  └── VGUIImagePanel (image display)
```

### VGUIPanel (Base Class)
**Scope**: Client

Base class for all VGUI panels. Provides core functionality for positioning, sizing, visibility, and hierarchy.

```angelscript
class VGUIPanel {
    // Validity
    bool IsValid();

    // Position and sizing
    void SetPos(int x, int y);
    void SetSize(int wide, int tall);
    void SetBounds(int x, int y, int wide, int tall);

    // Visibility
    void SetVisible(bool state);
    bool IsVisible();

    // Hierarchy
    void SetParent(VGUIPanel@ parent);
    void AddChild(VGUIPanel@ child);
    void RemoveChild(VGUIPanel@ child);

    // Appearance
    void SetFgColor(int r, int g, int b, int a);
    void SetBgColor(int r, int g, int b, int a);

    // Rendering
    void Repaint();
    void SetPaintEnabled(bool enabled);

    // Input
    bool IsMouseOver();

    // Cleanup
    void Remove();  // Remove from parent
}
```

**SetPos** / **SetSize** / **SetBounds**: Position and size the panel
- Coordinates are in screen space (pixels)
- Position (0,0) is top-left corner of screen

**SetVisible**: Show or hide the panel and all its children

**SetParent** / **AddChild** / **RemoveChild**: Manage panel hierarchy
- Child panels move with their parent
- Child visibility is affected by parent visibility

**SetFgColor** / **SetBgColor**: Set foreground and background colors
- Parameters: red (0-255), green (0-255), blue (0-255), alpha (0-255)
- Alpha 0 = fully transparent, 255 = fully opaque

**IsMouseOver**: Check if mouse cursor is over this panel

**Remove**: Remove panel from its parent (panel remains valid until released)

### VGUIFrame
**Scope**: Client

Windowed panels with title bars, dragging support, and optional close/minimize buttons.

```angelscript
class VGUIFrame : VGUIPanel {
    // All VGUIPanel methods, plus:

    void SetTitle(const string& in);
    void SetMoveable(bool);
    void SetSizeable(bool);
    void SetCloseButton(bool);
    void SetMinimizeButton(bool);
    void Center();  // Center on screen
}
```

**SetTitle**: Set the title bar text

**SetMoveable**: Allow/disallow dragging the frame by its title bar

**SetSizeable**: Allow/disallow resizing the frame by dragging edges

**SetCloseButton** / **SetMinimizeButton**: Show/hide window control buttons

**Center**: Position the frame at the center of the screen

**Example**:
```angelscript
VGUIFrame@ frame = CreateFrame(100, 100, 400, 300, "My Window");
frame.SetMoveable(true);
frame.SetSizeable(false);
frame.SetCloseButton(true);
frame.Center();
frame.SetVisible(true);
```

### VGUIButton
**Scope**: Client

Clickable buttons with callback support.

```angelscript
class VGUIButton : VGUIPanel {
    // All VGUIPanel methods, plus:

    void SetText(const string& in);
    void SetCallback(const string& in);  // Function name to call when clicked
    void SetEnabled(bool);
    void SetArmedColor(int r, int g, int b, int a);  // Color when mouse over
    void SetDepressedColor(int r, int g, int b, int a);  // Color when clicked
}
```

**SetText**: Set the button's label text

**SetCallback**: Set the AngelScript function to call when button is clicked
- Function must exist in the global scope
- Function takes no parameters and returns void

**SetEnabled**: Enable/disable the button (disabled buttons can't be clicked)

**SetArmedColor**: Color when mouse hovers over button

**SetDepressedColor**: Color when button is being clicked

**Example**:
```angelscript
void OnMyButtonClick() {
    MS_ANGEL_INFO("Button was clicked!");
}

VGUIButton@ btn = CreateButton("Click Me", 50, 50, 120, 30);
btn.SetCallback("OnMyButtonClick");
btn.SetArmedColor(255, 200, 100, 255);  // Orange when hovering
btn.SetVisible(true);
```

### VGUILabel
**Scope**: Client

Static text display with formatting options.

```angelscript
class VGUILabel : VGUIPanel {
    // All VGUIPanel methods, plus:

    void SetText(const string& in);
    void SetTextColor(int r, int g, int b, int a);
    void SetContentAlignment(int alignment);  // 0=left, 1=center, 2=right
}
```

**SetText**: Set the label text

**SetTextColor**: Set the color of the text

**SetContentAlignment**: Align text within the label
- 0 = left-aligned
- 1 = center-aligned
- 2 = right-aligned

**Example**:
```angelscript
VGUILabel@ label = CreateLabel("Player Health: 100", 10, 10, 200, 30);
label.SetTextColor(0, 255, 0, 255);  // Green text
label.SetContentAlignment(0);  // Left-aligned
label.SetVisible(true);
```

### VGUITextEntry
**Scope**: Client

Text input fields for user text entry.

```angelscript
class VGUITextEntry : VGUIPanel {
    // All VGUIPanel methods, plus:

    void SetText(const string& in);
    string GetText();
    void SetEditable(bool);
    void SetMaxLength(int);    // Note: Not fully supported in base VGUI
    void SetMultiline(bool);   // Note: Use TextPanel for multiline
}
```

**SetText** / **GetText**: Set or retrieve the current text

**SetEditable**: Allow/disallow editing

**Example**:
```angelscript
VGUITextEntry@ entry = CreateTextEntry(10, 50, 200, 30);
entry.SetText("Enter your name");
entry.SetEditable(true);
entry.SetVisible(true);

// Later, retrieve the text:
string playerName = entry.GetText();
```

### VGUIImagePanel
**Scope**: Client

Display images (sprites or textures).

```angelscript
class VGUIImagePanel : VGUIPanel {
    // All VGUIPanel methods, plus:

    void SetImage(const string& in);  // Set image path
    void SetScaleImage(bool);         // Scale image to fit panel size
}
```

**SetImage**: Set the image to display
- Path relative to game directory
- Supports TGA format

**SetScaleImage**: Whether to scale the image to fit the panel bounds

**Example**:
```angelscript
VGUIImagePanel@ img = CreateImagePanel("gfx/vgui/logo.tga", 100, 100, 256, 256);
img.SetScaleImage(true);
img.SetVisible(true);
```

---

## VGUI Factory Functions
**Scope**: Client

Factory functions for creating VGUI panels.

```angelscript
// Create windowed frame
VGUIFrame@ CreateFrame(int x, int y, int wide, int tall, const string& in title);

// Create button
VGUIButton@ CreateButton(const string& in text, int x, int y, int wide, int tall);

// Create label
VGUILabel@ CreateLabel(const string& in text, int x, int y, int wide, int tall);

// Create text entry field
VGUITextEntry@ CreateTextEntry(int x, int y, int wide, int tall);

// Create image panel
VGUIImagePanel@ CreateImagePanel(const string& in imagePath, int x, int y, int wide, int tall);

// Create generic panel (container)
VGUIPanel@ CreatePanel(int x, int y, int wide, int tall);
```

---

## VGUI Utility Functions
**Scope**: Client

```angelscript
// Get screen dimensions
void GetScreenSize(int& out wide, int& out tall);

// Check if panel is valid
bool IsPanelValid(VGUIPanel@ panel);
```

**GetScreenSize**: Retrieve the current screen resolution
- Use for positioning panels relative to screen size

**IsPanelValid**: Check if a panel pointer is valid and not null

---

## VGUI Complete Example
**Scope**: Client

Here's a complete example showing how to create a custom dialog:

```angelscript
// Global variables to keep panels alive
VGUIFrame@ g_CustomDialog;
VGUILabel@ g_MessageLabel;
VGUIButton@ g_OkButton;
VGUIButton@ g_CancelButton;

void ShowCustomDialog(const string& in message) {
    // Create frame
    g_CustomDialog = CreateFrame(0, 0, 400, 200, "Notification");
    g_CustomDialog.Center();
    g_CustomDialog.SetMoveable(true);
    g_CustomDialog.SetCloseButton(true);

    // Create message label
    g_MessageLabel = CreateLabel(message, 20, 50, 360, 60);
    g_MessageLabel.SetTextColor(255, 255, 255, 255);
    g_MessageLabel.SetContentAlignment(1);  // Center
    g_CustomDialog.AddChild(g_MessageLabel);

    // Create OK button
    g_OkButton = CreateButton("OK", 100, 140, 80, 30);
    g_OkButton.SetCallback("OnDialogOK");
    g_OkButton.SetArmedColor(100, 200, 100, 255);
    g_CustomDialog.AddChild(g_OkButton);

    // Create Cancel button
    g_CancelButton = CreateButton("Cancel", 220, 140, 80, 30);
    g_CancelButton.SetCallback("OnDialogCancel");
    g_CancelButton.SetArmedColor(200, 100, 100, 255);
    g_CustomDialog.AddChild(g_CancelButton);

    // Show dialog
    g_CustomDialog.SetVisible(true);
}

void OnDialogOK() {
    MS_ANGEL_INFO("User clicked OK");
    HideCustomDialog();
}

void OnDialogCancel() {
    MS_ANGEL_INFO("User clicked Cancel");
    HideCustomDialog();
}

void HideCustomDialog() {
    if (g_CustomDialog !is null && g_CustomDialog.IsValid()) {
        g_CustomDialog.SetVisible(false);
        @g_CustomDialog = null;  // Release references
        @g_MessageLabel = null;
        @g_OkButton = null;
        @g_CancelButton = null;
    }
}
```

---

## Notes

1. **Entity Lifetime**: Entity references (CBaseEntity@, CBasePlayer@, CBaseMonster@, CMSMonster@) are managed by the game engine. Scripts should not attempt to delete entities directly.

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
- **1.2**: Added comprehensive CBaseMonster and CMSMonster bindings with full property access and AI control methods
- **1.3**: Added complete client-side AngelScript API documentation including CLocalPlayer, CClientEntity, CTempEntity, CDynamicLight, CBeam, CClientEnvironment, CClientSound, and client utility functions
- **1.4**: Refactored client-side classes to use inheritance hierarchies using asbind20's `.base<>()` feature:
  - CClientEffect base class for effects (CTempEntity, CDynamicLight, CBeam)
  - CLocalPlayer now inherits from CClientEntity, gaining all entity query methods
- **1.5**: Added comprehensive VGUI (Valve GUI) system for client-side custom UI creation:
  - VGUIPanel base class for all UI elements
  - VGUIFrame for windowed interfaces
  - VGUIButton with callback support
  - VGUILabel for text display
  - VGUITextEntry for user input
  - VGUIImagePanel for image display
  - Factory functions and complete usage examples