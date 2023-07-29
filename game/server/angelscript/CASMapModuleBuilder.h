//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#ifndef GAME_SHARED_ZPS_ANGELSCRIPT_CASMAPMODULEBUILDER_H
#define GAME_SHARED_ZPS_ANGELSCRIPT_CASMAPMODULEBUILDER_H

#include "CASBaseModuleBuilder.h"

/**
*	Builder for the map script.
*/
class CASMapModuleBuilder : public CASBaseModuleBuilder
{
public:
	/**
	*	Constructor.
	*	@param pszMapScript Map script to load.
	*/
	CASMapModuleBuilder( const char* const pszMapScript );

	bool DefineWords( CScriptBuilder& builder ) override;

	void PrintError( ErrorBaseModuleLoader err, const char *strinput1, const char *strinput2 = "", const char *strinput3 = "", const char *strinput4 = "" ) override;
};

#endif // GAME_SHARED_ZPS_ANGELSCRIPT_CASMAPMODULEBUILDER_H
