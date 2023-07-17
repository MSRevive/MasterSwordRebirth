//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#ifndef GAME_SERVER_ZPS_ANGELSCRIPT_CHL2ASSERVERMANAGER_H
#define GAME_SERVER_ZPS_ANGELSCRIPT_CHL2ASSERVERMANAGER_H

#include "steam/steamtypes.h"
#include "CHL2ASManager.h"
#include "MSCASLogging.h"

class CASModule;

#define ANGELSCRIPT_TYPE_MSG "[Angelscript]"

// Log position
#define LOG_ANGELSCRIPT "angelscript.log"
#define LOG_STATS "stats.log"
#define LOG_VCONSOLE "vconsole.log"

typedef enum
{
	ASCALL_ANY = 0,
	ASCALL_MAP_SCRIPT,
	ASCALL_BASE_SCRIPT
} ASFuncCall;

/**
*	Manages the map script and handles some important map stage activities.
*/
class CHL2ASServerManager final : public CHL2ASManager
{
public:
	CHL2ASServerManager();

	CASManager& GetASManager() { return m_Manager; }

	bool Initialize() override;

	void Shutdown() override;

	void Think();

	/**
	*	A new map has been started. Handle map script compilation, if any script name was provided.
	*	Calls MapInit.
	*	@param pszMapScriptFileName Name of the map script to load.
	*/
	void WorldCreated( const char* const pszMapScriptFileName );

	/**
	*	Runs a function from the plugin and map script
	*	@param strFunctionVoid Name of the function that should be ran
	*/
	void CallFunctionVoid( const char* const strFunctionVoid, bool bFuncError = false );
	void CallCoreFunctionVoid( const char* const strFunctionVoid, bool bFuncError = false );

	/**
	*	The map has ended, and worldspawn is being removed.
	*	Calls MapEnd.
	*/
	void WorldEnded();

	bool IsReady() const { return m_bReady; }

	void SetReady( const bool bReady ) { m_bReady = bReady; }

	void ASPrint( const char *strFormat, ... );
	void ASLogPrint( const char* strTitle, const char *szFile, as::LogType eType, const char* strFormat, ... );
	void ASLogPrintOnce( uint64 id, const char* strTitle, const char *szFile, as::LogType eType, const char* strFormat, ... );

private:
	// For ASLogPrintOnce, saves the log title to memory
	struct LogSaved_t
	{
		uint64	id;
		char file[64];
	};
	std::vector<LogSaved_t> m_OncePrintLog;

	CASModule* m_pModule = nullptr;

	bool m_bReady;
};

extern CHL2ASServerManager g_ASManager;

#endif // GAME_SERVER_ZPS_ANGELSCRIPT_CHL2ASSERVERMANAGER_H
