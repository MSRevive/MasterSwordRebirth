//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#include <angelscript.h>
#include <AngelscriptUtils/util/ASUtil.h>
#include <AngelscriptUtils/util/ASLogging.h>

#include "angelscript/HL2ASConstants.h"
#include "angelscript/CHL2ASServerManager.h"

#include "angelscript/CASContagionImport.h"

#include "CASLog.h"

#if defined( SPDLOG )
#include "spdlog_main.h"
#endif

/**PAGE
 *
 * Class used for logging purposes.
 *
 * @page Log
 * @category Globals
 *
 * @desc_md false
 * @type void
 *
*/


#if defined( SPDLOG )
spdlog_type ASLogTypeToSpdlogLogType( eASLogTypes eLogType )
{
	switch ( eLogType )
	{
	case AS_LOGTYPE_INFO:		return LOGTYPE_INFO;
	case AS_LOGTYPE_CRITICAL:	return LOGTYPE_CRITICAL;
	case AS_LOGTYPE_WARN:		return LOGTYPE_WARN;
	case AS_LOGTYPE_ERROR:		return LOGTYPE_ERROR;
	case AS_LOGTYPE_DEBUG:		return LOGTYPE_DEBUG;
	default:					return LOGTYPE_TRACE;
	}
}
#endif


void CASLog::PrintToServerConsole( const eASLogTypes eLogType, const std::string &strMsg )
{
#if !defined( ASCOMPILER )
	// You want to make a "string format attack"? Then about you go fuck yourself then?
	char szBuffer[4096];
	strcpy( szBuffer, strMsg.c_str() );

	for ( int i = 0; i < sizeof( szBuffer ); i++ )
	{
		if ( szBuffer[i] == '\0' )
			break;

		if ( szBuffer[i] != '%' )
			continue;

		if ( szBuffer[i + 1] == 'p' || szBuffer[i + 1] == 'd' || szBuffer[i + 1] == 'c' ||
			szBuffer[i + 1] == 'u' || szBuffer[i + 1] == 'x' || szBuffer[i + 1] == 's' ||
			szBuffer[i + 1] == 'n' )
		{
			szBuffer[i] = ' ';
			szBuffer[i + 1] = ' ';
		}
	}

	as::LogType type;
	switch ( eLogType )
	{
		case AS_LOGTYPE_INFO:		type = as::LogType::LOG_INF; break;
		case AS_LOGTYPE_CRITICAL:	type = as::LogType::LOG_ERR_CRIT; break;
		case AS_LOGTYPE_WARN:		type = as::LogType::LOG_WRN; break;
		case AS_LOGTYPE_ERROR:		type = as::LogType::LOG_ERR; break;
		case AS_LOGTYPE_DEBUG:		type = as::LogType::LOG_DBG; break;
		default: type = as::LogType::LOG_NRM; break;
	}

	g_ASManager.ASLogPrint( "Angelscript Logger", ContagionImport::UTIL_CurrentMapLog(), type, szBuffer );
#endif
}


void CASLog::LogLocation( const std::string &strLogLocation, const eASLogTypes eLogType, const std::string &strMsg )
{
#if !defined( ASCOMPILER )
	// You want to make a "string format attack"? Then about you go fuck yourself then?
	char szBuffer[4096];
	strcpy( szBuffer, strMsg.c_str() );

	for ( int i = 0; i < sizeof( szBuffer ); i++ )
	{
		if ( szBuffer[i] == '\0' )
			break;

		if ( szBuffer[i] != '%' )
			continue;

		if ( szBuffer[i + 1] == 'p' || szBuffer[i + 1] == 'd' || szBuffer[i + 1] == 'c' ||
			szBuffer[i + 1] == 'u' || szBuffer[i + 1] == 'x' || szBuffer[i + 1] == 's' ||
			szBuffer[i + 1] == 'n' )
		{
			szBuffer[i] = ' ';
			szBuffer[i + 1] = ' ';
		}
	}

	as::LogType type;
	switch ( eLogType )
	{
		case AS_LOGTYPE_INFO:		type = as::LogType::LOG_INF; break;
		case AS_LOGTYPE_CRITICAL:	type = as::LogType::LOG_ERR_CRIT; break;
		case AS_LOGTYPE_WARN:		type = as::LogType::LOG_WRN; break;
		case AS_LOGTYPE_ERROR:		type = as::LogType::LOG_ERR; break;
		case AS_LOGTYPE_DEBUG:		type = as::LogType::LOG_DBG; break;
		default: type = as::LogType::LOG_NRM; break;
	}

	g_ASManager.ASLogPrint( "Angelscript Logger", ContagionImport::UTIL_LogFolder( strLogLocation.c_str(), true ), type, szBuffer );
#endif
}

/**
 * Class name for CASLog in scripts.
 */
#define AS_CASLOG_CLASSNAME "CASLog"

static CASLog g_ASLog;

/**JSON
 * Enumeration of types (or severity) of messages.
 *
 * @type enum
 * @name LogTypes
 *
 * @args LOGTYPE_TRACE # For "code tracing" purposes only, only scripters will need this.
 * @args LOGTYPE_INFO # Useful information about plugin's behaviour like something has been finished with success.
 * @args LOGTYPE_CRITICAL # A critical error prevents the plugin to function properly and needs immediate attention.
 * @args LOGTYPE_WARN # Warning about potential problems that could be minor (incorrect behaviour, bad result...) or major (plugin failing to function properly).
 * @args LOGTYPE_ERROR # An error causes problems but doesn't require immediate attention.
 * @args LOGTYPE_DEBUG # Information that could be used by people like servers operators for troubleshooting purposes.
 *
 */
void RegisterScriptEnumASLogTypes( asIScriptEngine &engine )
{
	const char *pszObjectName = "LogTypes";

	engine.RegisterEnum( pszObjectName );

	engine.RegisterEnumValue( pszObjectName, "LOGTYPE_TRACE", AS_LOGTYPE_TRACE );
	engine.RegisterEnumValue( pszObjectName, "LOGTYPE_INFO", AS_LOGTYPE_INFO );
	engine.RegisterEnumValue( pszObjectName, "LOGTYPE_CRITICAL", AS_LOGTYPE_CRITICAL );
	engine.RegisterEnumValue( pszObjectName, "LOGTYPE_WARN", AS_LOGTYPE_WARN );
	engine.RegisterEnumValue( pszObjectName, "LOGTYPE_ERROR", AS_LOGTYPE_ERROR );
	engine.RegisterEnumValue( pszObjectName, "LOGTYPE_DEBUG", AS_LOGTYPE_DEBUG );
}

void RegisterScriptCASLog( asIScriptEngine &engine )
{
	RegisterScriptEnumASLogTypes( engine );

	const char *pszObjectName = AS_CASLOG_CLASSNAME;

	engine.RegisterObjectType( pszObjectName, 0, asOBJ_REF | asOBJ_NOCOUNT );

/**JSON
 * Log a message of a certain severity to the console.
 *
 * @type void
 * @name PrintToServerConsole
 *
 * @args %enum_LogTypes% eLogType # The type or severity of the message.
 * @args %string_in% strMsg # The message to show.
 *
 * @return Prints to the server console and logs
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void PrintToServerConsole( const LogTypes eLogType, const string& in strMsg )",
		asMETHOD( CASLog, PrintToServerConsole ), asCALL_THISCALL );

/**JSON
 * Log a message of a certain severity within a specific context to a specific location.
 *
 * @type void
 * @name ToLocation
 *
 * @args %string_in% strLogLocation # The location the log file will be saved at
 * @args %enum_LogTypes% eLogType # The type or severity of the message.
 * @args %string_in% strMsg # The message to log.
 *
 * @return Prints to the server console and logs
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void ToLocation( const string& in strLogLocation, const LogTypes eLogType, const string& in strMsg )",
		asMETHOD( CASLog, LogLocation ), asCALL_THISCALL );

	engine.RegisterGlobalProperty( AS_CASLOG_CLASSNAME " Log", &g_ASLog );
}
