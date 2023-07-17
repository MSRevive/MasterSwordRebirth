//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#include <angelscript.h>

#include "CASGlobalVars.h"
#if !defined( ASCOMPILER )
#include "msdllheaders.h"
#endif

#if !defined( ASCOMPILER )
#include <angelscript/CHL2ASServerManager.h>
#endif

extern globalvars_t* gpGlobals;

// Reads the page information and is required!
// If it doesn't have this (and not defined first), the API doc gen will **NOT** generate the information!!
/**PAGE
 *
 * A description of Globals or something
 *
 * @page Globals
 * @category Globals
 *
 * @desc_md false
 * @type void
 *
*/

std::string CASGlobalVars::GetCurrentMapName()
{
#if !defined( ASCOMPILER )
	return STRING( gpGlobals->mapname );
#else
	return "";
#endif
}


float CASGlobalVars::GetCurrentGameTime()
{
#if !defined( ASCOMPILER )
	return gpGlobals->time;
#else
	return 0;
#endif
}


int CASGlobalVars::GetMaxClients()
{
#if !defined( ASCOMPILER )
	return gpGlobals->maxClients;
#else
	return 0;
#endif
}

static void Globalss_StripWhitespaceChars( char *szBuffer )
{
	char *szOut = szBuffer;

	for ( char *szIn = szOut; *szIn; szIn++ )
	{
		if ( *szIn != ' ' && *szIn != '\r' )
			*szOut++ = *szIn;
	}
	*szOut = '\0';
}

bool CASGlobalVars::IsCurrentMap( std::string& strValue )
{
	return FStrEq( strValue.c_str(), GetCurrentMapName().c_str() );
}

/**
 * Class name for CASGlobalVars in scripts.
 */
#define AS_CASGLOBALVARS_CLASSNAME "CASGlobalVars"

static CASGlobalVars g_ASGlobalVars;

void RegisterScriptCASGlobalVars( asIScriptEngine& engine )
{
	const char* const pszObjectName = AS_CASGLOBALVARS_CLASSNAME;

	engine.RegisterObjectType( pszObjectName, 0, asOBJ_REF | asOBJ_NOCOUNT );

/**JSON
 * Grabs the current name of the map (without the .bsp)
 *
 * @desc_md true
 * @global false
 * @name GetCurrentMapName
 * @type string
 * @isfunc true
 *
 * @return Returns the current name of the map
 *
*/
	engine.RegisterObjectMethod(
		pszObjectName, "string GetCurrentMapName()",
		asMETHOD( CASGlobalVars, GetCurrentMapName ), asCALL_THISCALL );

/**JSON
 * Checks if we are on a specific map
 *
 * @desc_md true
 * @global false
 * @name IsCurrentMap
 * @type bool
 * @isfunc true
 *
 * @args %string_in% szMap # Our mapname we want to check
 *
 * @return Returns true if we are on this specific map
 *
*/
	engine.RegisterObjectMethod(
		pszObjectName, "bool IsCurrentMap(string &in szMap)",
		asMETHOD( CASGlobalVars, IsCurrentMap ), asCALL_THISCALL );

/**JSON
 * Grabs the current global time
 *
 * @desc_md false
 * @global false
 * @name GetCurrentTime
 * @type float
 * @isfunc true
 *
 * @return preset_return_float
 *
*/
	engine.RegisterObjectMethod(
		pszObjectName, "float GetCurrentTime()",
		asMETHOD( CASGlobalVars, GetCurrentGameTime ), asCALL_THISCALL );

/**JSON
 * Grabs the total amount of players that can be connected to the server
 *
 * @desc_md false
 * @global false
 * @name GetMaxClients
 * @type int
 * @isfunc true
 *
 * @return preset_return_int
 *
*/
	engine.RegisterObjectMethod(
		pszObjectName, "int GetMaxClients()",
		asMETHOD( CASGlobalVars, GetMaxClients ), asCALL_THISCALL );

	engine.RegisterGlobalProperty( AS_CASGLOBALVARS_CLASSNAME " Globals", &g_ASGlobalVars );
}
