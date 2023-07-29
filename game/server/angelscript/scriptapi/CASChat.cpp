//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#include "cbase.h"
#include "CASChat.h"

#include "angelscript/HL2ASConstants.h"
#if !defined( ASCOMPILER )
#include "angelscript/CHL2ASServerManager.h"
#include "terror/TerrorPlayer.h"
#else
#include "angelscript/CASCManager.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/**PAGE
 *
 * Class used for chat related purposes.
 *
 * @page Chat
 * @category Utilities
 *
 * @desc_md false
 * @type void
 *
*/

void CASChat::CenterMessage( ChatTeam iChatType, const std::string& szString )
{
#if !defined( ASCOMPILER )
	CBasePlayer *pTargets = nullptr;

	if ( iChatType > chat_any )
	{
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			// Find all the zombies within range
			pTargets = UTIL_PlayerByIndex( i );
			if ( pTargets && pTargets->GetTeamNumber() == iChatType )
				ClientPrint( pTargets, HUD_PRINTCENTER, szString.c_str() );
		}
	}
	else
		UTIL_ClientPrintAll( HUD_PRINTCENTER, szString.c_str() );
#endif
}


void CASChat::HintMessage( ChatTeam iChatType, const std::string& szString )
{
#if !defined( ASCOMPILER )
	CBasePlayer *pTargets = nullptr;

	if ( iChatType > chat_any )
	{
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			// Find all the zombies within range
			pTargets = UTIL_PlayerByIndex( i );
			if ( pTargets && pTargets->GetTeamNumber() == iChatType )
				ClientPrint( pTargets, HUD_PRINTNOTIFY, szString.c_str() );
		}
	}
	else
		UTIL_ClientPrintAll( HUD_PRINTNOTIFY, szString.c_str() );
#endif
}


/**MARKDOWN
Code Example:
```cpp
// Print for all
Chat.PrintToChat( all, "This text has {green}green{default} text." );

// Print for specific player
Chat.PrintToChat( pPlayer, "This text has {green}green{default} text." );
```

## Available Chat Colors
[ShowAllChatColors]
 * @filename PrintToChat
 */
void CASChat::PrintToChat( ChatTeam iChatType, const std::string& szString )
{
#if !defined( ASCOMPILER )
	CBasePlayer *pTargets = nullptr;

	std::string ColorMsg = "\x01";
	ColorMsg += szString;

	if ( iChatType > chat_any )
	{
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			// Find all the zombies within range
			pTargets = UTIL_PlayerByIndex( i );
			if ( pTargets && pTargets->GetTeamNumber() == iChatType )
				ClientPrint( pTargets, HUD_PRINTTALK, ColorMsg.c_str() );
		}
	}
	else
		UTIL_ClientPrintAll( HUD_PRINTTALK, ColorMsg.c_str() );
#endif
}


void CASChat::PrintToConsole( ChatTeam iChatType, const std::string& szString )
{
#if !defined( ASCOMPILER )
	CBasePlayer *pTargets = nullptr;

	if ( iChatType > chat_any )
	{
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			// Find all the zombies within range
			pTargets = UTIL_PlayerByIndex( i );
			if ( pTargets && pTargets->GetTeamNumber() == iChatType )
				ClientPrint( pTargets, HUD_PRINTCONSOLE, szString.c_str() );
		}
	}
	else
		UTIL_ClientPrintAll( HUD_PRINTCONSOLE, szString.c_str() );
#endif
}


void CASChat::PrintToConsole2( const std::string& szString )
{
#if !defined( ASCOMPILER )
	Msg( "%s\n", UTIL_StripColors( szString.c_str() ) );
#endif
}


void CASChat::CenterMessage_Player( CBasePlayer *pPlayer, const std::string& szString )
{
#if !defined( ASCOMPILER )
	if ( !pPlayer ) return;
	ClientPrint( pPlayer, HUD_PRINTCENTER, szString.c_str() );
#endif
}


void CASChat::HintMessage_Player( CBasePlayer *pPlayer, const std::string& szString )
{
#if !defined( ASCOMPILER )
	if ( !pPlayer ) return;
	ClientPrint( pPlayer, HUD_PRINTNOTIFY, szString.c_str() );
#endif
}


void CASChat::PrintToChat_Player( CBasePlayer *pPlayer, const std::string& szString )
{
#if !defined( ASCOMPILER )
	if ( !pPlayer ) return;
	std::string ColorMsg = "\x01";
	ColorMsg += szString;
	ClientPrint( pPlayer, HUD_PRINTTALK, ColorMsg.c_str() );
#endif
}


void CASChat::PrintToChat_PlayerTerror( CTerrorPlayer *pPlayer, const std::string& szString )
{
#if !defined( ASCOMPILER )
	if ( !pPlayer ) return;
	std::string ColorMsg = "\x01";
	ColorMsg += szString;
	ClientPrint( pPlayer, HUD_PRINTTALK, ColorMsg.c_str() );
#endif
}


void CASChat::PrintToConsole_Player( CBasePlayer *pPlayer, const std::string& szString )
{
#if !defined( ASCOMPILER )
	if ( pPlayer )
		ClientPrint( pPlayer, HUD_PRINTCONSOLE, szString.c_str() );
	else
		PrintToConsole2( szString );
#endif
}

void CASChat::PrintToConsole_TerrorPlayer( CTerrorPlayer *pPlayer, const std::string& szString )
{
#if !defined( ASCOMPILER )
	if ( pPlayer )
		ClientPrint( pPlayer, HUD_PRINTCONSOLE, szString.c_str() );
	else
		PrintToConsole2( szString );
#endif
}


/**
 * Class name for CASChat in scripts.
 */
#define AS_CASCHAT_CLASSNAME "CASChat"

static CASChat g_ASChat;

/**JSON
 * Enumeration of team chat.
 *
 * @type enum
 * @name ChatTeam
 *
 * @args all # Will display the message to all players
 * @args survivor # Will display the message to the survivor team only
 * @args zombie # Will display the message to the zombie team only
 *
 */
static void RegisterScriptChatTeam( asIScriptEngine& engine )
{
	const char* const pszObjectName = "ChatTeam";

	engine.RegisterEnum( pszObjectName );

	engine.RegisterEnumValue( pszObjectName, "all", chat_any );
	engine.RegisterEnumValue( pszObjectName, "survivor", chat_survivor );
	engine.RegisterEnumValue( pszObjectName, "zombie", chat_zombie );
}


void RegisterASChat( asIScriptEngine &engine )
{
	RegisterScriptChatTeam( engine );

	const char* const pszObjectName = AS_CASCHAT_CLASSNAME;

	engine.RegisterObjectType( pszObjectName, 0, asOBJ_REF | asOBJ_NOCOUNT );

/**JSON
 * Prints a message on the center of the screen.
 *
 * @type void
 * @name CenterMessage
 *
 * @args %enum_ChatTeam% iChatType # Determines which team this will be displayed to
 * @args %string_in% strMsg # The message that will be displayed
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void CenterMessage(ChatTeam iChatType, const string& in strMsg)", 
		asMETHOD( CASChat, CenterMessage ), asCALL_THISCALL );

/**JSON
 * Prints a message on the center of the screen.
 *
 * @type void
 * @name CenterMessagePlayer
 *
 * @args %class_CBasePlayer% pPlayer # Determines which player this will be displayed to
 * @args %string_in% strMsg # The message that will be displayed
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void CenterMessagePlayer(CBasePlayer@ pPlayer, const string& in strMsg)", 
		asMETHOD( CASChat, CenterMessage_Player ), asCALL_THISCALL );

/**JSON
 * Prints a hint message that will be shown at the top of the screen.
 *
 * @type void
 * @name HintMessage
 *
 * @args %enum_ChatTeam% iChatType # Determines which team this will be displayed to
 * @args %string_in% strMsg # The message that will be displayed
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void HintMessage(ChatTeam iChatType, const string& in strMsg)", 
		asMETHOD( CASChat, HintMessage ), asCALL_THISCALL );

/**JSON
 * Prints a hint message that will be shown at the top of the screen.
 *
 * @type void
 * @name HintMessagePlayer
 *
 * @args %class_CBasePlayer% pPlayer # Determines which player this will be displayed to
 * @args %string_in% strMsg # The message that will be displayed
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void HintMessagePlayer(CBasePlayer@ pPlayer, const string& in strMsg)", 
		asMETHOD( CASChat, HintMessage_Player ), asCALL_THISCALL );

/**JSON
 * Prints a message to the chat.
 *
 * @type void
 * @name PrintToChat
 * @desc_md true
 *
 * @args %enum_ChatTeam% iChatType # Determines which team this will be displayed to
 * @args %string_in% strMsg # The message that will be displayed
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void PrintToChat(ChatTeam iChatType, const string& in strMsg)", 
		asMETHOD( CASChat, PrintToChat ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void PrintToChat(CBasePlayer@ pPlayer, const string& in strMsg)", 
		asMETHOD( CASChat, PrintToChat_Player ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void PrintToChat(CTerrorPlayer@ pPlayer, const string& in strMsg)", 
		asMETHOD( CASChat, PrintToChat_PlayerTerror ), asCALL_THISCALL );

/**JSON
 * Prints a message to the chat.
 *
 * @type void
 * @name PrintToChatPlayer
 *
 * @args %class_CBasePlayer% pPlayer # Determines which player this will be displayed to
 * @args %string_in% strMsg # The message that will be displayed
 *
 * @infobox_enable true
 * @infobox_type deprecated
 * @infobox_desc Use [PrintToChat][PrintToChat] function instead.
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void PrintToChatPlayer(CBasePlayer@ pPlayer, const string& in strMsg)", 
		asMETHOD( CASChat, PrintToChat_Player ), asCALL_THISCALL );
	engine.RegisterObjectMethod(
		pszObjectName, "void PrintToChatPlayer(CTerrorPlayer@ pPlayer, const string& in strMsg)", 
		asMETHOD( CASChat, PrintToChat_PlayerTerror ), asCALL_THISCALL );

/**JSON
 * Prints a message to the client's console.
 *
 * @type void
 * @name PrintToConsole
 *
 * @args %enum_ChatTeam% iChatType # Determines which team this will be displayed to
 * @args %string_in% strMsg # The message that will be displayed
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void PrintToConsole(ChatTeam iChatType, const string& in strMsg)", 
		asMETHOD( CASChat, PrintToConsole ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void PrintToConsole(const string& in strMsg)", 
		asMETHOD( CASChat, PrintToConsole2 ), asCALL_THISCALL );

/**JSON
 * Prints a message to the client's console.
 *
 * @type void
 * @name PrintToConsolePlayer
 *
 * @args %class_CBasePlayer% pPlayer # Determines which player this will be displayed to
 * @args %string_in% strMsg # The message that will be displayed
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void PrintToConsolePlayer(CBasePlayer@ pPlayer, const string& in strMsg)", 
		asMETHOD( CASChat, PrintToConsole_Player ), asCALL_THISCALL );
	engine.RegisterObjectMethod(
		pszObjectName, "void PrintToConsolePlayer(CTerrorPlayer@ pPlayer, const string& in strMsg)", 
		asMETHOD( CASChat, PrintToConsole_TerrorPlayer ), asCALL_THISCALL );

	engine.RegisterGlobalProperty( AS_CASCHAT_CLASSNAME " Chat", &g_ASChat );
}
