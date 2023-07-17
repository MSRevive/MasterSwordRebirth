//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#ifndef GAME_SHARED_ZPS_ANGELSCRIPT_SCRIPTAPI_CASGLOBALS_H
#define GAME_SHARED_ZPS_ANGELSCRIPT_SCRIPTAPI_CASGLOBALS_H

#include <string>
#include "qangle.h"

class asIScriptEngine;
class CScriptArray;

/**
 * Class that holds global variables.
 * People experienced with GoldSrc/Source engine programming/scripting will recognize it as the "gpGlobals" struct/class.
 */
class CASGlobalVars final
{
public:
	CASGlobalVars() = default;
	~CASGlobalVars() = default;

	/**
	 * Returns the name of the map being played currently by the server.
	 * @returns The current map's name.
	 */
	std::string GetCurrentMapName();

	/**
	 * Returns the current game time in seconds of the server.
	 * This time is reset to 0 whenever the map starts.
	 * @warning Do not mistake "game time" with "real time"!
	 * @returns The current game time in seconds (or time elapsed since the map started).
	 */
	float GetCurrentGameTime();

	/**
	 * Returns the maximum amount of clients this server can host.
	 * @returns The server's maximum clients capacity.
	 */
	int GetMaxClients();

	bool IsCurrentMap( std::string &strValue );

private:
	CASGlobalVars( const CASGlobalVars & ) = delete;
	CASGlobalVars& operator=( const CASGlobalVars & ) = delete;
};

/**
 * Registers the CGlobalVars (also known as "gpGlobals") class for Angelscript, called "Globals".
 * @param engine Script engine.
 */
void RegisterScriptCASGlobalVars( asIScriptEngine& engine );

#endif // GAME_SHARED_ZPS_ANGELSCRIPT_SCRIPTAPI_CASGLOBALS_H
