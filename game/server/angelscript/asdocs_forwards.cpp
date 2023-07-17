// This CPP file is only meant for including the forwards, nothing else.

// Homepage for the API Generator
/**MARKDOWN
Introduction
============

Welcome to the Contagion Angelscript API documentation!

This will help you creating Angelscript plugins for Contagion for use in servers as well as maps.

We provide some code examples in the dark area to the right whenever we feel it's needed.

Keep in mind that this is an **API documentation, not an Angelscript scripting manual/tutorial**. If you are not familiar with Angelscript and/or you need to brush up your knowledge, please consult it's [scripting manual](http://www.angelcode.com/angelscript/sdk/docs/manual/doc_script.html) first before proceeding.

Like always, we are open to comments, suggestions and potential fixes we can make to this API documentation. The best place to do this would be on our `#contagion-modding` channel on our [Discord server](https://discord.gg/monochrome).

Have fun scripting!

- - -

## Available Angelscript commands

To access these commands, make sure the player has the admin access rights of [LEVEL_ADMIN][AdminAccessLevel_t] or higher.
Command                 | Description                                                                      | Dedicated Server Only
----------------------- | ---------------------------------------------------------------------------------|-----------------------------
cg_angelscript_debug    | Enable angelscript debug?                                                        | **Yes**
as_log                  | Logs angelscript related errors an issues (will save the logs to logs/angelscript.log)       | **No**
as_showcommands         | Show all available angelscript console commands.                                 | **No**
as_listplugins          | List all server side Angelscript plugins.                                        | **No**
as_reloadplugin         | Reload a specific server side Angelscript plugin.                                | **No**
as_loadplugin           | Load a specific server side Angelscript plugin.                                  | **No**
as_unloadplugin         | Unload a specific server side Angelscript plugin.                                | **No**
as_reloadallplugins     | Reload all server side Angelscript plugins.                                      | **No**
as_unloadallplugins     | Unload all server side Angelscript plugins.                                      | **No**
as_processpluginevents  | Processes server side Angelscript plugins's events.                              | **No**

- - -

## Global Properties
```cpp
int TEAM_ANY;
int TEAM_INVALID;				// Same as TEAM_ANY
int TEAM_SPECTATOR;
int TEAM_SURVIVOR;
int TEAM_ZOMBIE;
int TEAM_L4D1_SURVIVOR;			// Unused

int FCAP_IMPULSE_USE;			// can be used by the player
int FCAP_CONTINUOUS_USE;		// can be used by the player
int FCAP_ONOFF_USE;				// can be used by the player
int FCAP_DIRECTIONAL_USE;		// Player sends +/- 1 when using (currently only tracktrains)
int FCAP_USE_ONGROUND;			// NOTE: Normally +USE only works in direct line of sight.  Add these caps for additional searches
int FCAP_USE_IN_RADIUS;			// NOTE: Normally +USE only works in direct line of sight.  Add these caps for additional searches
int FCAP_USE_HELPING_HAND;		// L4D helping hand
int FCAP_USEABLE_BY_INFECTED;	// can be used by infected
int FCAP_ENABLE_GLOW;			// Object can show the +use glow effect

int EF_BONEMERGE;				// Performs bone merge on client side
int EF_BRIGHTLIGHT;				// DLIGHT centered at entity origin
int EF_DIMLIGHT;				// player flashlight
int EF_NOINTERP;				// don't interpolate the next frame
int EF_NOSHADOW;				// Don't cast no shadow
int EF_NODRAW;					// don't draw entity
int EF_NORECEIVESHADOW;			// Don't receive no shadow
int EF_BONEMERGE_FASTCULL;		// For use with EF_BONEMERGE. If this is set, then it places this ent's origin at its
								// parent and uses the parent's bbox + the max extents of the aiment.
								// Otherwise, it sets up the parent's bones every frame to figure out where to place
								// the aiment, which is inefficient because it'll setup the parent's bones even if
								// the parent is not in the PVS.
int EF_ITEM_BLINK;				// blink an item so that the user notices it.
int EF_PARENT_ANIMATES;			// always assume that the parent entity is animating
```

- - -

## Available Angelscript Addons

Addon                   | Description
----------------------- | ---------------------------------------------------------------------------------
[string](manual/doc_script_stdlib_string.html)                  | The string object supports a number of operators, and has several class methods and supporting global functions to facilitate the manipulation of strings.
[array](manual/doc_datatypes_arrays.html)                   | The array object supports a number of operators and has several class methods to facilitate the manipulation of strings.
[dictionary](manual/doc_datatypes_dictionary.html)              | The dictionary object is a reference type, so it's possible to use handles to the dictionary object when passing it around to avoid costly copies.
[datetime](manual/doc_script_stdlib_datetime.html)                | The datetime type represents a calendar date and time. It can be used to do math operations with dates, such as comparing two dates, determining the difference between dates, and addition/substraction on dates to form new dates.

 * @filename homepage
 */

/**PAGE
 *
 * Forwards that is ran automatically
 *
 * @page Forwards
 * @category Globals
 *
 * @desc_md false
 * @type void
 *
*/

// Forwards that's being used

//============================================================================================================================
/// Map
//============================================================================================================================

/**JSON
 * When the map has started
 *
 * @desc_md false
 * @global true
 * @name OnMapInit
 * @type void
 * @isfunc true
 * @classfunction true
 *
 * @infobox_enable true
 * @infobox_type deprecated
 * @infobox_desc Use The President's [OnMapStart][ThePresident_OnMapStart] function instead.
 *
*/

/**JSON
 * When the server is shutting down, or switching maps
 *
 * @desc_md false
 * @global true
 * @name OnMapShutdown
 * @type void
 * @isfunc true
 * @classfunction true
 *
*/

/**JSON
 * Called each frame (when there are players present)
 *
 * @desc_md false
 * @global true
 * @name OnProcessRound
 * @type void
 * @isfunc true
 * @classfunction true
 *
*/

//============================================================================================================================
/// Plugin
//============================================================================================================================

/**JSON
 * When the plugin is starting
 *
 * @desc_md false
 * @global true
 * @name OnPluginInit
 * @type void
 * @isfunc true
 * @classfunction true
 * @restrict plugin
 *
*/

/**JSON
 * When the plugin is shutting down
 *
 * @desc_md false
 * @global true
 * @name OnPluginUnload
 * @type void
 * @isfunc true
 * @classfunction true
 * @restrict plugin
 *
*/

//============================================================================================================================
/// Player
//============================================================================================================================

/**JSON
 * When an entity is picked up, if the said entity is registered on the "Entity Register"
 *
 * @desc_md false
 * @global true
 * @name OnEntityPickedUp
 * @type void
 * @classfunction true
 *
 * @args %class_CTerrorPlayer% pPlayer # The player that picked up said entity
 * @args %class_CBaseEntity% pEntity # The object of the entity
*/

/**JSON
 * When an entity is used, if the said entity is registered on the "Entity Register"
 *
 * @desc_md false
 * @global true
 * @name OnEntityUsed
 * @type void
 * @classfunction true
 *
 * @args %class_CTerrorPlayer% pPlayer # The player that used said entity
 * @args %class_CBaseEntity% pEntity # The object of the entity
*/

/**JSON
 * When a prop_button is used, if the said entity is registered on the "Entity Register"
 *
 * @desc_md false
 * @global true
 * @name OnButtonUsed
 * @type void
 * @classfunction true
 *
 * @args %class_CTerrorPlayer% pPlayer # The player that used said entity
 * @args %class_CBaseEntity% pEntity # The object of the entity
*/

/**JSON
 * When an entity is dropped, if the said entity is registered on the "Entity Register"
 *
 * @desc_md false
 * @global true
 * @name OnEntityDropped
 * @type void
 * @classfunction true
 *
 * @args %class_CTerrorPlayer% pPlayer # The player that dropped said entity
 * @args %class_CBaseEntity% pEntity # The object of the entity
*/

/**JSON
 * When the entity has woken up, if the said entity is registered on the "Entity Register"
 *
 * However, this only applies to physics props.
 *
 * @desc_md false
 * @global true
 * @name OnEntityWokenUp
 * @type void
 * @classfunction true
 *
 * @args %class_CBaseEntity% pEntity # The object of the entity
*/

/**JSON
 * When the entity is damaged, if the said entity is registered on the "Entity Register"
 *
 * @desc_md false
 * @global true
 * @name OnEntityDamaged
 * @type void
 * @classfunction true
 *
 * @args %class_CBaseEntity% pAttacker # The entity that attacked this entity
 * @args %class_CBaseEntity% pEntity # The object of the entity
*/

/**JSON
 * When the entity is destroyed, if the said entity is registered on the "Entity Register"
 *
 * @desc_md false
 * @global true
 * @name OnEntityBreak
 * @type void
 * @classfunction true
 *
 * @args %class_CBaseEntity% pAttacker # The entity that attacked this entity
 * @args %string_in% strTargetname # The targetname of the entity that got destroyed
*/

/**JSON
 * When the entity is sending a specific output, if the said output is registered on the "Entity Register"
 *
 * @desc_md false
 * @global true
 * @name OnEntityOutput
 * @type void
 * @classfunction true
 *
 * @args %string_in% strOutput # The output value that the output want to call
 * @args %class_CBaseEntity% pActivator # The entity that activated the output
 * @args %class_CBaseEntity% pCaller # The entity that called the output
*/

/**JSON
 * When the entity is sending a specific output, if the said output is registered on the "Entity Register"
 *
 * @desc_md false
 * @global true
 * @name OnEntityOutputParam
 * @type void
 * @classfunction true
 *
 * @args %string_in% strOutput # The output value that the output want to call
 * @args %string_in% strParamater # The paramater of the output
 * @args %class_CBaseEntity% pActivator # The entity that activated the output
 * @args %class_CBaseEntity% pCaller # The entity that called the output
*/

//============================================================================================================================
/// ThePresident
//============================================================================================================================

/**JSON
 * When the map has begun
 *
 * @desc_md false
 * @global true
 * @name ThePresident_OnMapStart
 * @type void
 * @isfunc true
 * @classfunction true
 *
*/

/**JSON
 * When the map has ended
 *
 * @desc_md false
 * @global true
 * @name ThePresident_OnMapEnd
 * @type void
 * @isfunc true
 * @classfunction true
 *
*/

/**JSON
 * When the round has begun
 *
 * @desc_md false
 * @global true
 * @name ThePresident_OnRoundStart
 * @type void
 * @isfunc true
 * @classfunction true
 *
*/

/**JSON
 * When the round has ended
 *
 * @desc_md false
 * @global true
 * @name ThePresident_OnRoundEnd
 * @type void
 * @isfunc true
 * @classfunction true
 *
*/

//============================================================================================================================
/// ThePresident -- Flatline
//============================================================================================================================

/**JSON
 * When the wave has just begun on Flatline
 *
 * @desc_md false
 * @global true
 * @name ThePresident_OnWaveStart
 * @type void
 * @isfunc true
 * @classfunction true
 *
*/

/**JSON
 * When the wave has been completed on Flatline
 *
 * @desc_md false
 * @global true
 * @name ThePresident_OnWaveCompleted
 * @type void
 * @isfunc true
 * @classfunction true
 *
*/

/**JSON
 * When the wave has failed on Flatline
 *
 * @desc_md false
 * @global true
 * @name ThePresident_OnWaveFailed
 * @type void
 * @isfunc true
 * @classfunction true
 *
*/
