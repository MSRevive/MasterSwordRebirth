//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#include <AngelscriptUtils/add_on/scriptbuilder/scriptbuilder.h>

#include "CASMapModuleBuilder.h"

#include "angelscript/CHL2ASServerManager.h"

CASMapModuleBuilder::CASMapModuleBuilder( const char* const pszMapScript )
	: CASBaseModuleBuilder( "data/scripts/", "Map" )
{
	IncludeCoreMSCScripts();
	if ( pszMapScript && *pszMapScript )
		AddScript( pszMapScript );
	g_ASManager.ASPrint( "Beginning map script Compiling...\n" );
}

bool CASMapModuleBuilder::DefineWords( CScriptBuilder& builder )
{
	return CASBaseModuleBuilder::DefineWords( builder );
}

void CASMapModuleBuilder::PrintError( ErrorBaseModuleLoader err, const char* strinput1, const char* strinput2, const char* strinput3, const char* strinput4 )
{
	CASBaseModuleBuilder::PrintError( err, strinput1, strinput2, strinput3, strinput4 );
}
