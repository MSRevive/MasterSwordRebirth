//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#include "cbase.h"
#include "CASEvents.h"

#include "angelscript/HL2ASConstants.h"
#if !defined( ASCOMPILER )
#include "angelscript/CHL2ASServerManager.h"
#else
#include "angelscript/CASCManager.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/**PAGE
 *
 * All the available events that can be used by plugins & map scripts
 *
 * @page Hooks
 * @category Globals
 *
 * @desc_md false
 * @type void
 *
*/


HookCallResult CallGlobalEvent( CASEvent& event, CallFlags_t flags, ... )
{
	va_list list;

	va_start( list, flags );

	CASEventCaller caller;

	const auto result = caller.VCall( event, g_ASManager.GetASManager().GetEngine(), flags, list );

	va_end( list );

	return result;
}

void RegisterASEventsEnums( asIScriptEngine &engine )
{
	const char *pszObjectName = "InfectionState";

/**JSON
 * Enumeration of infection states.
 *
 * @type enum
 * @name InfectionState
 *
 * @args state_none # No longer infected
 * @args state_infection # Normal infection
 * @args state_infection_forced # Forced infection
 * @args state_infection_turning # Turning into a zombie
 * @args state_infection_completed # Infection completed, has fully turned
 *
 */
	engine.RegisterEnum( pszObjectName );
	engine.RegisterEnumValue( pszObjectName, "state_none", infectstate_none );
	engine.RegisterEnumValue( pszObjectName, "state_infection", infectstate_infected );
	engine.RegisterEnumValue( pszObjectName, "state_infection_forced", infectstate_infected_forced );
	engine.RegisterEnumValue( pszObjectName, "state_infection_turning", infectstate_infected_turning );
	engine.RegisterEnumValue( pszObjectName, "state_infection_completed", infectstate_infected_complete );
}

void RegisterASEvents( asIScriptEngine &engine )
{
	RegisterASEventsEnums( engine );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
ASGameEvent::ASGameEvent()
{
	m_bIsLocal = false;
	m_bIsReliable = false;
	m_pDataKeys = new KeyValues( "generic" );
	m_Name = "generic";
}

ASGameEvent::~ASGameEvent()
{
	m_pDataKeys->deleteThis();
}

bool ASGameEvent::GetBool( const char *keyName, bool defaultValue)
{
	MEM_ALLOC_CREDIT();
	return m_pDataKeys->GetInt( keyName, defaultValue ) != 0;
}

int ASGameEvent::GetInt( const char *keyName, int defaultValue)
{
	MEM_ALLOC_CREDIT();
	return m_pDataKeys->GetInt( keyName, defaultValue );
}

uint64 ASGameEvent::GetUint64( const char *keyName, uint64 defaultValue)
{
	MEM_ALLOC_CREDIT();
	return m_pDataKeys->GetUint64( keyName, defaultValue );
}

float ASGameEvent::GetFloat( const char *keyName, float defaultValue )
{
	MEM_ALLOC_CREDIT();
	return m_pDataKeys->GetFloat( keyName, defaultValue );
}

const char *ASGameEvent::GetString( const char *keyName, const char *defaultValue )
{
	MEM_ALLOC_CREDIT();
	return m_pDataKeys->GetString( keyName, defaultValue );
}

bool ASGameEvent::IsEmpty( const char *keyName )
{
	MEM_ALLOC_CREDIT();
	return m_pDataKeys->IsEmpty( keyName );
}

bool ASGameEvent::IsLocal() const
{
	return m_bIsLocal;
}

bool ASGameEvent::IsReliable() const
{
	return m_bIsReliable;
}

std::string ASGameEvent::AS_GetName()
{
	return m_Name;
}

bool ASGameEvent::AS_IsEmpty( std::string keyName )
{
	return IsEmpty( keyName.c_str() );
}

uint64 ASGameEvent::AS_GetBool( std::string keyName, bool defaultValue )
{
	return GetBool( keyName.c_str(), defaultValue );
}

uint64 ASGameEvent::AS_GetUint64( std::string keyName, uint64 defaultValue )
{
	return GetUint64( keyName.c_str(), defaultValue );
}

int ASGameEvent::AS_GetInt( std::string keyName, int defaultValue )
{
	return GetInt( keyName.c_str(), defaultValue );
}

float ASGameEvent::AS_GetFloat( std::string keyName, float defaultValue )
{
	return GetFloat( keyName.c_str(), defaultValue );
}

std::string ASGameEvent::AS_GetString( std::string keyName, std::string defaultValue )
{
	auto ref = GetString( keyName.c_str(), defaultValue.c_str() );
	return ref ? ref : "";
}

#include "memdbgoff.h"

void ConstructIGameEvent( ASGameEvent* pMemory )
{
	new ( pMemory ) ASGameEvent();
}

void CopyConstructIGameEvent( ASGameEvent* pMemory, const ASGameEvent& asEvent )
{
	new ( pMemory ) ASGameEvent( asEvent );
}

#include "memdbgon.h"

void DestroyIGameEvent( ASGameEvent* pMemory )
{
	pMemory->~ASGameEvent();
}

void RegisterIGameEvent( asIScriptEngine &engine )
{
	const char* const pszObjectNetData = "ASGameEvent";

	engine.RegisterObjectType(
		pszObjectNetData, sizeof( ASGameEvent ),
		asOBJ_VALUE | asOBJ_APP_CLASS | asOBJ_APP_CLASS_CONSTRUCTOR | asOBJ_APP_CLASS_COPY_CONSTRUCTOR | asOBJ_APP_CLASS_ALLFLOATS );

	engine.RegisterObjectMethod(
		pszObjectNetData, "Color& opAssign(const Color& in other)",
		asMETHODPR( Color, operator=, ( const Color& ), Color& ), asCALL_THISCALL );

/**JSON
 * Constructs a ASGameEvent class.
 *
 * @type class
 * @ref ASGameEvent
 * @name ASGameEvent
 *
 * @return Returns NetData class which can be manipulated
 */
	engine.RegisterObjectBehaviour(
		pszObjectNetData, asBEHAVE_CONSTRUCT, "void ASGameEvent()",
		asFUNCTION( ConstructIGameEvent ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectBehaviour(
		pszObjectNetData, asBEHAVE_CONSTRUCT, "void ASGameEvent(const Color& in other)",
		asFUNCTION( CopyConstructIGameEvent ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Destroys the ASGameEvent
 *
 * @type void
 * @ref ASGameEvent
 * @name DestroyGameEvent
 * @isfunc true
 *
 */
	engine.RegisterObjectBehaviour(
		pszObjectNetData, asBEHAVE_DESTRUCT, "void DestroyGameEvent()",
		asFUNCTION( DestroyIGameEvent ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Grabs the name of the event
 *
 * @type string
 * @ref ASGameEvent
 * @name GetName
 * @isfunc true
 *
 * @return Returns the value as string
 */
	engine.RegisterObjectMethod(
		pszObjectNetData, "string GetName() const",
		asMETHOD( ASGameEvent, AS_GetName ), asCALL_THISCALL );

/**JSON
 * Grabs the name of the event
 *
 * @type string
 * @ref ASGameEvent
 * @name GetName
 * @isfunc true
 *
 * @return Returns the value as string
 */
	engine.RegisterObjectMethod(
		pszObjectNetData, "bool IsEmpty(string keyName)",
		asMETHOD( ASGameEvent, AS_IsEmpty ), asCALL_THISCALL );

/**JSON
 * Checks if this is an local event or not
 *
 * @type bool
 * @ref ASGameEvent
 * @name IsLocal
 * @isfunc true
 *
 * @return Returns true if the event is local
 */
	engine.RegisterObjectMethod(
		pszObjectNetData, "bool IsLocal() const",
		asMETHOD( ASGameEvent, IsLocal ), asCALL_THISCALL );

/**JSON
 * Checks if this event is reliable or not
 *
 * @type bool
 * @ref ASGameEvent
 * @name IsReliable
 * @isfunc true
 *
 * @return Returns true if the event is reliable
 */
	engine.RegisterObjectMethod(
		pszObjectNetData, "bool IsReliable() const",
		asMETHOD( ASGameEvent, IsReliable ), asCALL_THISCALL );

/**JSON
 * Grabs the string from the event
 *
 * @type string
 * @ref ASGameEvent
 * @name GetString
 *
 * @args %string% keyName # Our event name
 * @args %uint64% defaultValue # Our default value
 *
 * @return Returns the value as string
 */
	engine.RegisterObjectMethod(
		pszObjectNetData, "string GetString(string keyName, string defaultValue = \"\")",
		asMETHOD( ASGameEvent, AS_GetString ), asCALL_THISCALL );

/**JSON
 * Grabs the float from the event
 *
 * @type float
 * @ref ASGameEvent
 * @name GetFloat
 *
 * @args %string% keyName # Our event name
 * @args %uint64% defaultValue # Our default value
 *
 * @return Returns the value as float
 */
	engine.RegisterObjectMethod(
		pszObjectNetData, "float GetFloat(string keyName, float defaultValue = 0.0f)",
		asMETHOD( ASGameEvent, AS_GetFloat ), asCALL_THISCALL );

/**JSON
 * Grabs the boolean from the event
 *
 * @type bool
 * @ref ASGameEvent
 * @name GetBool
 *
 * @args %string% keyName # Our event name
 * @args %uint64% defaultValue # Our default value
 *
 * @return Returns the value as boolean
 */
	engine.RegisterObjectMethod(
		pszObjectNetData, "bool GetBool(string keyName, bool defaultValue = false)",
		asMETHOD( ASGameEvent, AS_GetBool ), asCALL_THISCALL );

/**JSON
 * Grabs the interger from the event
 *
 * @type int
 * @ref ASGameEvent
 * @name GetInt
 *
 * @args %string% keyName # Our event name
 * @args %uint64% defaultValue # Our default value
 *
 * @return Returns the value as int
 */
	engine.RegisterObjectMethod(
		pszObjectNetData, "int GetInt(string keyName, int defaultValue = 0)",
		asMETHOD( ASGameEvent, AS_GetInt ), asCALL_THISCALL );

/**JSON
 * Grabs the uint64 from the event
 *
 * @type uint64
 * @ref ASGameEvent
 * @name GetUint64
 *
 * @args %string% keyName # Our event name
 * @args %uint64% defaultValue # Our default value
 *
 * @return Returns the value as uint64
 */
	engine.RegisterObjectMethod(
		pszObjectNetData, "uint64 GetUint64(string keyName, uint64 defaultValue = 0)",
		asMETHOD( ASGameEvent, AS_GetUint64 ), asCALL_THISCALL );
}

//=======================================================================================================================
//=======================================================================================================================
// Rounds

/**JSON
 * When the wave starts on Flatline
 *
 * @type event
 * @eventtype FlatLine
 * @name WaveStart
 * @restrict any
 *
 * @args %int_in% iRound # The current round
 *
*/
CASEvent asevent_FlatLine_WaveStart( "WaveStart", "int& in", "FlatLine", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When the wave ended on Flatline
 *
 * @type event
 * @eventtype FlatLine
 * @name WaveEnded
 * @restrict any
 *
 * @args %bool_in% bCompleted # Did we complete this wave or not?
 *
*/
CASEvent asevent_FlatLine_WaveEnded( "WaveEnded", "bool& in", "FlatLine", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When we started the last wave on Flatline
 *
 * @type event
 * @eventtype FlatLine
 * @name WaveLast
 * @restrict any
 *
 *
*/
CASEvent asevent_FlatLine_WaveLast( "WaveLast", "", "FlatLine", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When we we should start escaping on Flatline
 *
 * @type event
 * @eventtype FlatLine
 * @name EscapeSequence
 * @restrict any
 *
 *
*/
CASEvent asevent_FlatLine_EscapeSequence( "EscapeSequence", "", "FlatLine", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When we progressed half way on Flatline
 *
 * @type event
 * @eventtype FlatLine
 * @name HalfTime
 * @restrict any
 *
 *
*/
CASEvent asevent_FlatLine_HalfTime( "HalfTime", "", "FlatLine", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When the player spawned in the world.
 *
 * @type event
 * @eventtype Player
 * @name OnPlayerSpawn
 * @restrict any
 *
 * @args %class_CTerrorPlayer% pPlayer # The player that recently spawned
 *
*/
CASEvent asevent_PlayerSpawn( "OnPlayerSpawn", "CTerrorPlayer@", "Player", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When the player spawned for the first time.
 *
 * @type event
 * @eventtype Player
 * @name OnPlayerInitSpawn
 * @restrict any
 *
 * @args %class_CTerrorPlayer% pPlayer # The player that spawned
 *
*/
CASEvent asevent_PlayerInitSpawn( "OnPlayerInitSpawn", "CTerrorPlayer@", "Player", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When the player is sending a chat message.
 *
 * @type event
 * @eventtype Player
 * @name PlayerSay
 * @restrict plugin
 *
 * @args %class_CTerrorPlayer% pPlayer # The player that is sending the message
 * @args %class_CASCommand% pArgs # The chat arguments
 *
*/
CASEvent asevent_PlayerSay( "PlayerSay", "CTerrorPlayer@, CASCommand@", "Player", ModuleAccessMask::PLUGIN, EventStopMode::CALL_ALL );

/**JSON
 * When the player is executing a menu item
 *
 * @type event
 * @eventtype Player
 * @name OnMenuExecuted
 *
 * @args %class_CTerrorPlayer% pPlayer # The player that executed this menu
 * @args %string_in% szID # The ID of the menu
 * @args %int_in% iValue # The value of the item
 *
*/
CASEvent asevent_PlayerMenuExecuted( "OnMenuExecuted", "CTerrorPlayer@, const string& in, int& in", "Player", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When the player is using the console.
 *
 * @type event
 * @eventtype Player
 * @name OnConCommand
 * @restrict plugin
 *
 * @args %class_CTerrorPlayer% pPlayer # The player that is sending the message
 * @args %class_CASCommand% pArgs # The console arguments
 *
*/
CASEvent asevent_PlayerOnConCommand( "OnConCommand", "CTerrorPlayer@, CASCommand@", "Player", ModuleAccessMask::PLUGIN, EventStopMode::CALL_ALL );

/**JSON
 * When the player has joined the game.
 *
 * @type event
 * @eventtype Player
 * @name OnPlayerConnect
 * @restrict any
 *
 * @args %string_in% strPlayerName # The name of the player
 * @args %string_in% strPlayerIP # The IP of the player
 *
*/
CASEvent asevent_PlayerConnect( "OnPlayerConnect", "const string& in, const string& in", "Player", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When the player has fully joined
 *
 * @type event
 * @eventtype Player
 * @name OnPlayerConnected
 * @restrict any
 *
 * @args %class_CTerrorPlayer% pPlayer # The player that just joined
 *
*/
CASEvent asevent_PlayerConnected( "OnPlayerConnected", "CTerrorPlayer@", "Player", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When the player has disconnected.
 *
 * @type event
 * @eventtype Player
 * @name OnPlayerDisconnected
 * @restrict any
 *
 * @args %class_CTerrorPlayer% pPlayer # The player that has disconnected
 *
*/
CASEvent asevent_PlayerDisconnected( "OnPlayerDisconnected", "CTerrorPlayer@", "Player", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When the player dies.
 *
 * @type event
 * @eventtype Player
 * @name OnPlayerKilled
 * @restrict any
 *
 * @args %class_CTerrorPlayer% pPlayer # The player that died
 * @args %CTakeDamageInfo_in% DamageInfo # The damage information
 *
*/
CASEvent asevent_PlayerKilled( "OnPlayerKilled", "CTerrorPlayer@, CTakeDamageInfo &in", "Player", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * Executes after the player has died.
 *
 * @type event
 * @eventtype Player
 * @name OnPlayerKilledPost
 * @restrict any
 *
 * @args %class_CTerrorPlayer% pPlayer # The player that died
 *
*/
CASEvent asevent_PlayerKilledPost( "OnPlayerKilledPost", "CTerrorPlayer@n", "Player", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When the player gets damaged.
 *
 * @type event
 * @eventtype Player
 * @name OnPlayerDamaged
 * @restrict any
 *
 * @args %class_CTerrorPlayer% pPlayer # The player that got damaged
 * @args %CTakeDamageInfo_in% DamageInfo # The damage information
 *
*/
CASEvent asevent_PlayerDamaged( "OnPlayerDamaged", "CTerrorPlayer@, CTakeDamageInfo &in", "Player", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When the player gets infected, or when the infection state updates.
 *
 * @type event
 * @eventtype Player
 * @name OnPlayerInfected
 * @restrict any
 *
 * @args %class_CTerrorPlayer% pPlayer # The player that got infected
 * @args %enum_InfectionState% iState # The infection state
 *
*/
CASEvent asevent_PlayerInfected( "OnPlayerInfected", "CTerrorPlayer@, InfectionState iState", "Player", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When the player drops an item / weapon. Gets executed when the player dies as well.
 *
 * @type event
 * @eventtype Player
 * @name OnEntityDropped
 * @restrict any
 *
 * @args %class_CTerrorPlayer% pPlayer # The player that's dropping the item
 * @args %class_CBaseEntity% pEntity # The entity that got dropped
 *
*/
CASEvent asevent_PlayerDroppedItem( "OnEntityDropped", "CTerrorPlayer@, CBaseEntity@", "Player", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When the a player gets added to the admin list
 *
 * @type event
 * @eventtype Admin
 * @name OnUserAdd
 * @restrict plugin
 *
 * @args %class_CTerrorPlayer% pPlayer # The player that got added as an admin
 * @args %bool_in% bRet # Returns true if the command succeded
 *
*/
CASEvent asevent_Admin_AddUser( "OnUserAdd", "CTerrorPlayer@, bool &in", "Admin", ModuleAccessMask::PLUGIN, EventStopMode::CALL_ALL );

/**JSON
 * When the a player gets admin status gets updated
 *
 * @type event
 * @eventtype Admin
 * @name OnUserUpdated
 * @restrict plugin
 *
 * @args %class_CTerrorPlayer% pPlayer # The player that got it's admin status updated
 * @args %enum_AdminAccessLevel_t% iLevel # The level that we want to be set as
 * @args %enum_AdminAccessLevel_t% iRet # The Actual level returned from the command
 *
*/
CASEvent asevent_Admin_UpdateUser( "OnUserUpdated", "CTerrorPlayer@, AdminAccessLevel_t, AdminAccessLevel_t", "Admin", ModuleAccessMask::PLUGIN, EventStopMode::CALL_ALL );

/**JSON
 * When the a player gets removed to the admin list
 *
 * @type event
 * @eventtype Admin
 * @name OnUserRemove
 * @restrict plugin
 *
 * @args %class_CTerrorPlayer% pPlayer # The player that got removed as an admin
 * @args %bool_in% bRet # Returns true if the command succeded
 *
*/
CASEvent asevent_Admin_RemoveUser( "OnUserRemove", "CTerrorPlayer@, bool &in", "Admin", ModuleAccessMask::PLUGIN, EventStopMode::CALL_ALL );

/**JSON
 * When the player gets banned.
 *
 * @type event
 * @eventtype Admin
 * @name OnUserBanned
 * @restrict plugin
 *
 * @args %class_CTerrorPlayer% pPlayer # The player that is going to get banned
 * @args %int_in% iMinutes # The amount of time in minutes
 * @args %string_in% strReason # The ban reason
 *
*/
CASEvent asevent_Admin_BannedUser( "OnUserBanned", "CTerrorPlayer@, int, const string &in", "Admin", ModuleAccessMask::PLUGIN, EventStopMode::CALL_ALL );

/**JSON
 * When the player gets banned (Extended).
 *
 * @type event
 * @eventtype Admin
 * @name OnUserBannedEx
 * @restrict plugin
 *
 * @args %class_CTerrorPlayer% pPlayer # The player that is going to get banned
 * @args %class_CTerrorPlayer% pAdmin # The player that called the ban
 * @args %int_in% iMinutes # The amount of time in minutes
 * @args %string_in% strReason # The ban reason
 *
*/
CASEvent asevent_Admin_BannedUserEx( "OnUserBannedEx", "CTerrorPlayer@, CTerrorPlayer@, int, const string &in", "Admin", ModuleAccessMask::PLUGIN, EventStopMode::CALL_ALL );

/**JSON
 * When the player gets banned (SteamID).
 *
 * @type event
 * @eventtype Admin
 * @name OnUserBannedSteamID
 * @restrict plugin
 *
 * @args %int_in% uid # The steamid that is going to get banned
 * @args %class_CTerrorPlayer% pAdmin # The player that called the ban
 * @args %int_in% iMinutes # The amount of time in minutes
 * @args %string_in% strReason # The ban reason
 *
*/
CASEvent asevent_Admin_BannedUserSteamID( "OnUserBannedSteamID", "int, CTerrorPlayer@, int, const string &in", "Admin", ModuleAccessMask::PLUGIN, EventStopMode::CALL_ALL );

/**JSON
 * When the player gets muted or gagged.
 *
 * @type event
 * @eventtype Admin
 * @name OnUserBannedEx
 * @restrict plugin
 *
 * @args %int_bool% bMuted # If true, then this is a mute instead of a gag.
 * @args %class_CTerrorPlayer% pPlayer # The player that is going to get banned
 * @args %class_CTerrorPlayer% pAdmin # The player that called the ban
 * @args %int_in% iMinutes # The amount of time in minutes
 * @args %string_in% strReason # The ban reason
 *
*/
CASEvent asevent_Admin_GagMute( "OnUserGagMute", "bool, CTerrorPlayer@, CTerrorPlayer@, int, const string &in", "Admin", ModuleAccessMask::PLUGIN, EventStopMode::CALL_ALL );

/**JSON
 * When the player gets muted or gagged (SteamID).
 *
 * @type event
 * @eventtype Admin
 * @name OnUserGagMuteSteamID
 * @restrict plugin
 *
 * @args %int_bool% bMuted # If true, then this is a mute instead of a gag.
 * @args %int_in% uid # The steamid that is going to get muted/gagged
 * @args %class_CTerrorPlayer% pAdmin # The player that called the mute/gag
 * @args %int_in% iMinutes # The amount of time in minutes
 * @args %string_in% strReason # The mute/gag reason
 *
*/
CASEvent asevent_Admin_GagMuteSteamID( "OnUserGagMuteSteamID", "bool, int, CTerrorPlayer@, int, const string &in", "Admin", ModuleAccessMask::PLUGIN, EventStopMode::CALL_ALL );

/**JSON
 * When the player gets warned (SteamID).
 *
 * @type event
 * @eventtype Admin
 * @name OnUserWarnedSteamID
 * @restrict plugin
 *
 * @args %int_in% uid # The steamid that is going to get warned
 * @args %class_CTerrorPlayer% pAdmin # The player that called the warning
 * @args %string_in% strReason # The ban reason
 *
*/
CASEvent asevent_Admin_WarnedUserSteamID( "OnUserWarnedSteamID", "int, CTerrorPlayer@, const string &in", "Admin", ModuleAccessMask::PLUGIN, EventStopMode::CALL_ALL );

/**JSON
 * When the player gets warned.
 *
 * @type event
 * @eventtype Admin
 * @name OnUserWarned
 * @restrict plugin
 *
 * @args %class_CTerrorPlayer% pPlayer # The player that is going to get warned
 * @args %class_CTerrorPlayer% pAdmin # The player that called the warning
 * @args %string_in% strReason # The ban reason
 *
*/
CASEvent asevent_Admin_WarnedUser( "OnUserWarned", "CTerrorPlayer@, CTerrorPlayer@, const string &in", "Admin", ModuleAccessMask::PLUGIN, EventStopMode::CALL_ALL );

/**JSON
 * When the player gets kicked.
 *
 * @type event
 * @eventtype Admin
 * @name OnUserKicked
 * @restrict plugin
 *
 * @args %class_CTerrorPlayer% pPlayer # The player that is going to get kicked
 * @args %string_in% strReason # The kick reason
 *
*/
CASEvent asevent_Admin_KickedUser( "OnUserKicked", "CTerrorPlayer@, const string &in", "Admin", ModuleAccessMask::PLUGIN, EventStopMode::CALL_ALL );

/**JSON
 * When an entity touched this trigger (if it gets accepted by the filters).
 *
 * @type event
 * @eventtype Trigger
 * @name OnStartTouch
 * @restrict any
 *
 * @args %class_CBaseEntity% pTrigger # The trigger object that was fired
 * @args %string_in% strEntityName # The name of the trigger
 * @args %class_CBaseEntity% pEntity # The entity that touched this trigger
 *
*/
CASEvent asevent_Trigger_OnTouch( "OnStartTouch", "CBaseEntity@, const string &in, CBaseEntity@", "Trigger", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When an entity stopped touching this trigger (if it gets accepted by the filters).
 *
 * @type event
 * @eventtype Trigger
 * @name OnTouching
 * @restrict any
 *
 * @args %class_CBaseEntity% pTrigger # The trigger object that was fired
 * @args %string_in% strEntityName # The name of the trigger
 * @args %class_CBaseEntity% pEntity # The entity that is touching this trigger
 *
*/
CASEvent asevent_Trigger_OnTouching( "OnTouching", "CBaseEntity@, const string &in, CBaseEntity@", "Trigger", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When an entity is touching this trigger (if it gets accepted by the filters).
 *
 * @type event
 * @eventtype Trigger
 * @name OnEndTouch
 * @restrict any
 *
 * @args %class_CBaseEntity% pTrigger # The trigger object that was fired
 * @args %string_in% strEntityName # The name of the trigger
 * @args %class_CBaseEntity% pEntity # The entity that stopped touching this trigger
 *
*/
CASEvent asevent_Trigger_OnEndTouch( "OnEndTouch", "CBaseEntity@, const string &in, CBaseEntity@", "Trigger", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When an entity is being created
 *
 * @type event
 * @eventtype Entities
 * @name OnEntityCreation
 * @restrict any
 *
 * @args %string_in% strClassname # The classname of the entity
 * @args %class_CBaseEntity% pEntity # The entity that was recently created
 *
*/
CASEvent asevent_Entities_OnEntityCreation( "OnEntityCreation", "const string& in, CBaseEntity@", "Entities", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When an entity is being destroyed
 *
 * @type event
 * @eventtype Entities
 * @name OnEntityDestruction
 * @restrict any
 *
 * @args %string_in% strClassname # The classname of the entity
 * @args %class_CBaseEntity% pEntity # The entity that is being destroyed
 *
*/
CASEvent asevent_Entities_OnEntityDestruction( "OnEntityDestruction", "const string& in, CBaseEntity@", "Entities", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When an entity is being destroyed
 *
 * @type event
 * @eventtype Entities
 * @name OnPhysicsBreak
 * @restrict any
 *
 * @args %class_CBaseEntity% pAttacker # The entity that attacked this entity
 * @args %class_CBaseEntity% pEntity # The entity that is being destroyed
 *
*/
CASEvent asevent_Entities_OnPhysicsBreak( "OnPhysicsBreak", "CBaseEntity@, CBaseEntity@", "Entities", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When the survivor AI spawns
 *
 * @type event
 * @eventtype Survivor
 * @name OnSurvivorSpawned
 * @restrict any
 *
 * @args %class_Survivor% pSurvivor # The survivor that spawned
 *
*/
CASEvent asevent_Survivor_Spawned( "OnSurvivorSpawned", "Survivor@", "Survivor", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When the survivor AI gets damaged.
 *
 * @type event
 * @eventtype Survivor
 * @name OnSurvivorDamaged
 * @restrict any
 *
 * @args %class_Survivor% pSurvivor # The survivor that got damaged
 * @args %CTakeDamageInfo_in% DamageInfo # The damage information
 *
*/
CASEvent asevent_Survivor_Damaged( "OnSurvivorDamaged", "Survivor@, CTakeDamageInfo &in", "Survivor", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When the survivor AI gets killed.
 *
 * @type event
 * @eventtype Survivor
 * @name OnSurvivorKilled
 * @restrict any
 *
 * @args %class_Survivor% pSurvivor # The survivor that got damaged
 * @args %CTakeDamageInfo_in% DamageInfo # The damage information
 *
*/
CASEvent asevent_Survivor_Killed( "OnSurvivorKilled", "Survivor@, CTakeDamageInfo &in", "Survivor", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When the infected spawns
 *
 * @type event
 * @eventtype Infected
 * @name OnInfectedSpawned
 * @restrict any
 *
 * @args %class_Infected% pInfected # The infected that spawned
 *
*/
CASEvent asevent_Infected_Spawned( "OnInfectedSpawned", "Infected@", "Infected", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When the infected gets damaged.
 *
 * @type event
 * @eventtype Infected
 * @name OnInfectedDamaged
 * @restrict any
 *
 * @args %class_Infected% pInfected # The infected that got damaged
 * @args %CTakeDamageInfo_in% DamageInfo # The damage information
 *
*/
CASEvent asevent_Infected_Damaged( "OnInfectedDamaged", "Infected@, CTakeDamageInfo &in", "Infected", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When the infected gets killed.
 *
 * @type event
 * @eventtype Infected
 * @name OnInfectedKilled
 * @restrict any
 *
 * @args %class_Infected% pInfected # The infected that got damaged
 * @args %CTakeDamageInfo_in% DamageInfo # The damage information
 *
*/
CASEvent asevent_Infected_Killed( "OnInfectedKilled", "Infected@, CTakeDamageInfo &in", "Infected", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When the cosmetic gets killed
 *
 * @type event
 * @eventtype Cosmetic
 * @name OnCosmeticKilled
 * @restrict any
 *
 * @args %string_in% strModel # The model that the cosmetic used
 * @args %Vector_in% vecPosition # The position the cosmetic
 * @args %QAngle_in% qAngles # The angles of the cosmetic
 *
*/
CASEvent asevent_CosmeticMerge_Killed( "OnCosmeticKilled", "const string& in, const Vector &in, const QAngle &in", "Cosmetic", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When the prop_car_alarm gets tripped
 *
 * @type event
 * @eventtype Entities
 * @name OnCarAlarmTripped
 * @restrict any
 *
 * @args %class_CBaseEntity% pCause # The entity that caused the alarm to trip (Survivor or CTerrorPlayer)
 * @args %class_CBaseEntity% pCar # The car that got tripped
 *
*/
CASEvent asevent_CarAlarmTripped( "OnCarAlarmTripped", "CBaseEntity@, CBaseEntity@", "Entities", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When the alarm ends from prop_car_alarm
 *
 * @type event
 * @eventtype Entities
 * @name OnCarAlarmTrippedEnded
 * @restrict any
 *
 * @args %class_CBaseEntity% pCar # The car that got tripped
 *
*/
CASEvent asevent_CarAlarmTrippedEnd( "OnCarAlarmTrippedEnded", "CBaseEntity@", "Entities", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When there are no survivors remaining.
 *
 * @type event
 * @eventtype ThePresident
 * @name OnNoSurvivorsRemaining
 * @restrict any
 *
 * @args %int% iCandidate # The current gamemode type
 *
*/
CASEvent asevent_ThePresident_NoSurvivorsLeft( "OnNoSurvivorsRemaining", "int", "ThePresident", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When the round gets terminated
 *
 * @type event
 * @eventtype ThePresident
 * @name OnTerminateRound
 * @restrict any
 *
 * @args %int% iTeam # The team that won
 *
*/
CASEvent asevent_ThePresident_TerminateRound( "OnTerminateRound", "int", "ThePresident", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When an entity is being created from the random item pool
 *
 * @type event
 * @eventtype ThePresident
 * @name OnRandomItemSpawn
 * @restrict any
 *
 * @args %string_in% strClassname # The classname of the entity
 * @args %class_CBaseEntity% pEntity # The entity that was recently created
 *
*/
CASEvent asevent_ThePresident_RandomItemSpawn( "OnRandomItemSpawn", "const string& in, CBaseEntity@", "ThePresident", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When a player heals themselves or their target (survivor/player)
 *
 * @type event
 * @eventtype Weapons
 * @name InoculatorHeal
 * @restrict any
 *
 * @args %class_CTerrorPlayer% pOwner # The owner of the weapon
 * @args %class_CBaseEntity% pTarget # The target we healed
 *
*/
CASEvent asevent_Weapons_InoculatorHeal( "InoculatorHeal", "CTerrorPlayer@, CBaseEntity@", "Weapons", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When a player heals themselves or their target (survivor/player)
 *
 * @type event
 * @eventtype Weapons
 * @name FirstAidHeal
 * @restrict any
 *
 * @args %class_CTerrorPlayer% pOwner # The owner of the weapon
 * @args %class_CBaseEntity% pTarget # The target we healed
 *
*/
CASEvent asevent_Weapons_FirstAidHeal( "FirstAidHeal", "CTerrorPlayer@, CBaseEntity@", "Weapons", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When a player bashes with their firearm, or attacks using their melee weapon
 *
 * @type event
 * @eventtype Weapons
 * @name Melee
 * @restrict any
 *
 * @args %class_CTerrorPlayer% pOwner # The owner of the weapon
 * @args %bool_in% bMeleeBash # Returns true if we bashed with our firearm. False if we used our melee weapon
 * @args %bool_in% bHeavy # Returns true if this was a heavy melee attack/bash
 * @args %class_CBaseEntity% pHit # The target we hit
 *
*/
CASEvent asevent_Weapons_Melee( "Melee", "CTerrorPlayer@, bool& in, bool& in, CBaseEntity@", "Weapons", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );

/**JSON
 * When a player uses (+attack) the item carry
 *
 * @type event
 * @eventtype Weapons
 * @name ItemCarry
 * @restrict any
 *
 * @args %class_CTerrorPlayer% pOwner # The owner of the weapon
 * @args %string_in% strName # The name of the item carry (if it does not have an objective name, it will return weapon_itemcarry)
 *
*/
CASEvent asevent_Weapons_ItemCarry( "ItemCarry", "CTerrorPlayer@, const string& in", "Weapons", ModuleAccessMask::ALL, EventStopMode::CALL_ALL );
