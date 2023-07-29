#include "MSCASLogging.h"

#include <stdio.h>
#include <stdarg.h>

// START PORT
// Ported from Source Engine
int V_vsnprintf_AS( char *pDest, int maxLen, char const *pFormat, va_list params )
{
	int len = _vsnprintf( pDest, maxLen, pFormat, params );
	if ( len < 0 )
	{
		len = maxLen;
		pDest[maxLen-1] = 0;
	}
	return len;
}

#define Q_vsnprintf		V_vsnprintf_AS
// END PORT

namespace as
{
	void DoLogging( const char *strFile, LogType type, const std::string &szMsg )
	{
		std::string szType;
		switch ( type )
		{
			case LOG_DBG: szType = "[Debug] >> "; break;
			case LOG_INF: szType = "[Info] >> "; break;
			case LOG_WRN: szType = "[Warning] >> "; break;
			case LOG_ERR: szType = "[Error] >> "; break;
			case LOG_ERR_CRIT: szType = "[Error::Critical] >> "; break;
			default: break;
		}

		// Append
		std::ofstream file( strFile, std::ios::app );
		if ( file )
		{
			file << szType.c_str() << szMsg.c_str();
			file.close();
		}
		// File not found? then create it
		else
		{
			file.open( strFile );
			if ( file )
			{
				file << szType.c_str() << szMsg.c_str();
				file.close();
			}
		}
	}

	void Log( const char* strFormat, ... )
	{
		va_list argptr;
		char tempString[1024];

		va_start( argptr, strFormat );
		Q_vsnprintf( tempString, sizeof(tempString), strFormat, argptr );
		va_end( argptr );

		DoLogging( "logs/angelscript.log", LOG_NRM, tempString );
	}

	void LogDbg( const char* strFormat, ... )
	{
		va_list argptr;
		char tempString[1024];

		va_start( argptr, strFormat );
		Q_vsnprintf( tempString, sizeof(tempString), strFormat, argptr );
		va_end( argptr );

		DoLogging( "logs/angelscript.log", LOG_DBG, tempString );
	}

	void LogInfo( const char* strFormat, ... )
	{
		va_list argptr;
		char tempString[1024];

		va_start( argptr, strFormat );
		Q_vsnprintf( tempString, sizeof(tempString), strFormat, argptr );
		va_end( argptr );

		DoLogging( "logs/angelscript.log", LOG_INF, tempString );
	}

	void LogWarning( const char* strFormat, ... )
	{
		va_list argptr;
		char tempString[1024];

		va_start( argptr, strFormat );
		Q_vsnprintf( tempString, sizeof(tempString), strFormat, argptr );
		va_end( argptr );

		DoLogging( "logs/angelscript.log", LOG_WRN, tempString );
	}

	void LogError( const char* strFormat, ... )
	{
		va_list argptr;
		char tempString[1024];

		va_start( argptr, strFormat );
		Q_vsnprintf( tempString, sizeof(tempString), strFormat, argptr );
		va_end( argptr );

		DoLogging( "logs/angelscript.log", LOG_ERR, tempString );
	}

	void LogErrorCritical( const char* strFormat, ... )
	{
		va_list argptr;
		char tempString[1024];

		va_start( argptr, strFormat );
		Q_vsnprintf( tempString, sizeof(tempString), strFormat, argptr );
		va_end( argptr );

		DoLogging( "logs/angelscript.log", LOG_ERR_CRIT, tempString );
	}
}
