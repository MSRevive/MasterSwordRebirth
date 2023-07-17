#ifndef GAME_ANGELSCRIPT_LOGGER_INTERFACE_H
#define GAME_ANGELSCRIPT_LOGGER_INTERFACE_H

#if !defined( ASCOMPILER )
#include "CHL2ASServerManager.h"
#endif

#include <AngelscriptUtils/util/ASLogging.h>
#include <stdarg.h>

// Header only interface
namespace as
{
	inline void DoInterface( as::LogType type, const char *szMsg )
	{
		// Log to file
		DoLogging( "logs/angelscript.log", type, szMsg );
		// Game print
#if !defined( ASCOMPILER )
		std::string szType( ANGELSCRIPT_TYPE_MSG );
#else
		std::string szType;
#endif
		switch ( type )
		{
			case LOG_DBG: szType += " [{green}Debug{default}] {yellow}>> {white}"; break;
			case LOG_INF: szType += " [{cyan}Info{default}] {yellow}>> {white}"; break;
			case LOG_WRN: szType += " [{orange}Warning{default}] {yellow}>> {white}"; break;
			case LOG_ERR: szType += " [{red}Error{default}] {yellow}>> {white}"; break;
			case LOG_ERR_CRIT: szType += " [{red}Error{yellow}::{orange}Critical{default}] {yellow}>> {white}"; break;
			default: break;
		}
		szType += szMsg;
		g_ASManager.ASPrint( szType.c_str() );
	}

	// Normal method
	inline void GameLog( as::LogType type, const char* strFormat, ... )
	{
		va_list argptr;
		char tempString[1024];
		va_start( argptr, strFormat );
		snprintf( tempString, sizeof(tempString), strFormat, argptr );
		va_end( argptr );

		DoInterface( type, tempString );
	}
}

#endif	//GAME_ANGELSCRIPT_LOGGER_INTERFACE_H