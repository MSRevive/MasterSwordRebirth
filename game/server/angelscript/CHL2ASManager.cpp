//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#include "CHL2ASManager.h"

#if !defined( ASCOMPILER )
#include "angelscript/CHL2ASServerManager.h"
#include "CASContagionImport.h"
#endif


#if !defined( ASCOMPILER )
as::LogType ASLogTypeToSpdLogType( asEMsgType eType )
{
	switch ( eType )
	{
		case asMSGTYPE_ERROR:	return as::LogType::LOG_ERR;
		case asMSGTYPE_WARNING:	return as::LogType::LOG_WRN;
		default:				return as::LogType::LOG_INF;
	}
}
#endif

void CHL2ASManager::MessageCallback( asSMessageInfo* pMsg ) const
{
	const char* pType = "";

	// Get the prefix.
	switch ( pMsg->type )
	{
		case asMSGTYPE_ERROR:	pType = "Error: "; break;
		case asMSGTYPE_WARNING: pType = "Warning: "; break;
		default: break;
	}

	// Only display the section if it was actually set. Some messages are not triggered by script code compilation or execution.
	const bool bHasSection = pMsg->section && *pMsg->section;

	bool bNeedsNewline = false;

#if defined( ASCOMPILER )
	if ( bHasSection )
	{
		Msg( ">> Section \"%s\"", pMsg->section );
		bNeedsNewline = true;
	}

	// Some messages don't refer to script code, and set both to 0.
	if ( pMsg->row != 0 && pMsg->col != 0 )
	{
		if ( bHasSection )
			Msg( " " );

		Msg( "(%d, %d)", pMsg->row, pMsg->col );
		bNeedsNewline = true;
	}

	if ( bNeedsNewline )
		Msg( "\n" );
	
	Msg( ">> %s%s\n", pType, pMsg->message );
#else
	std::string strSection = "";
	if ( bHasSection )
	{
		strSection = "Section \"" + std::string( pMsg->section ) + "\"";
		bNeedsNewline = true;
	}

	if ( pMsg->row != 0 && pMsg->col != 0 )
	{
		if ( bHasSection )
			strSection += " ";

		strSection += "(line = " + std::to_string( pMsg->row ) + ", col = " + std::to_string( pMsg->col ) +")";
		bNeedsNewline = true;
	}

	if ( bNeedsNewline )
		strSection += "\n";
	else
		strSection += " ";

	strSection += "\t" + std::string( pType ) + std::string( pMsg->message );

	g_ASManager.ASLogPrint(
		"Angelscript Manager",
		ContagionImport::UTIL_CurrentMapLog(),
		ASLogTypeToSpdLogType( pMsg->type ),
		strSection.c_str()
	);
#endif
}

bool CHL2ASManager::InitializeManager( IASInitializer& initializer )
{
	if ( !m_Manager.Initialize( initializer ) )
	{
		g_ASManager.ASPrint( "Failed to initialize Angelscript\n" );
		return false;
	}
	return true;
}

void CHL2ASManager::Shutdown()
{
	m_Manager.Shutdown();
}
