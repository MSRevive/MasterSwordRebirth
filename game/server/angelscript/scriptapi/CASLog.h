//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#ifndef GAME_SHARED_ZPS_ANGELSCRIPT_SCRIPTAPI_CASLOG_H
#define GAME_SHARED_ZPS_ANGELSCRIPT_SCRIPTAPI_CASLOG_H


#include <string>


class asIScriptEngine;


// Angelscript API version of "spdlog_types".
// Side note: could link to this in the API docs "https://stackoverflow.com/questions/2031163/when-to-use-the-different-log-levels"
/**
 * Enumeration of types (or severity) of messages.
 */
enum eASLogTypes
{
	/** For "code tracing" purposes only, only scripters will need this. */
	AS_LOGTYPE_TRACE = 0,
	/** Useful information about plugin's behaviour like something has been finished with success. */
	AS_LOGTYPE_INFO,
	/** A critical error prevents the plugin to function properly and needs immediate attention. */
	AS_LOGTYPE_CRITICAL,
	/** Warning about potential problems that could be minor (incorrect behaviour, bad result...) or major (plugin failing to function properly). */
	AS_LOGTYPE_WARN,
	/** An error causes problems but doesn't require immediate attention. */
	AS_LOGTYPE_ERROR,
	/** Information that could be used by people like servers operators for troubleshooting purposes. */
	AS_LOGTYPE_DEBUG
};


/**
* Class used for logging purposes.
*/
class CASLog final
{
public:
	CASLog() = default;
	~CASLog() = default;

	void PrintToServerConsole( const eASLogTypes eLogType, const std::string &strMsg );
	void LogLocation( const std::string &strLogLocation, const eASLogTypes eLogType, const std::string &strMsg );

private:
	CASLog( const CASLog & ) = delete;
	CASLog& operator=( const CASLog & ) = delete;
};


/**
 * Registers the logging types enumeration for Angelscript, called "LogTypes".
 * @param engine Script engine.
 */
void RegisterScriptEnumASLogTypes( asIScriptEngine& engine );

/**
 * Registers the logging class for Angelscript, called "Log".
 * @param engine Script engine.
 */
void RegisterScriptCASLog( asIScriptEngine& engine );

#endif // GAME_SHARED_ZPS_ANGELSCRIPT_SCRIPTAPI_CASLOG_H
