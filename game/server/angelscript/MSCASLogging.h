#ifndef GAME_SERVER_MSC_ASLOGGING_H
#define GAME_SERVER_MSC_ASLOGGING_H

#include <memory>
#include <string.h>
#include <fstream>

#ifdef WIN32
#undef VOID
#undef GetObject
#endif

/**
*	@file
*	Provides the log global
*/

namespace as
{
	typedef enum
	{
		LOG_NRM = 0,
		LOG_DBG,
		LOG_INF,
		LOG_WRN,
		LOG_ERR,
		LOG_ERR_CRIT,
		LOG_EXCEPTION
	} LogType;

	/**
	*	@brief The logger used by AngelscriptUtils. Expected to never be null
	*	
	*	@details The user is responsible for setting this and freeing it on program shutdown
	*	If not provided, CASManager will create a default logger that outputs to stdout
	*/
	extern void DoLogging(const char* strFile, LogType type, const std::string& szMsg);
	extern void Log( const char* strFormat, ... );
	extern void LogDbg( const char* strFormat, ... );
	extern void LogInfo( const char* strFormat, ... );
	extern void LogWarning( const char* strFormat, ... );
	extern void LogError( const char* strFormat, ... );
	extern void LogErrorCritical( const char* strFormat, ... );
}

#endif