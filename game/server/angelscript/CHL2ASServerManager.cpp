//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#include "angelscript/angelscript.h"
#include "CASContagionImport.h"

#include <AngelscriptUtils/CASModule.h>
#include <AngelscriptUtils/util/ASExtendAdapter.h>
#include <AngelscriptUtils/wrapper/ASCallable.h>

#include "angelscript/ScriptAPI/CASCBaseEntity.h"

#include <AngelscriptUtils/ScriptAPI/CASScheduler.h>

#include "angelscript/ScriptAPI/CASSchedule.h"

#include "angelscript/HL2ASConstants.h"

#include "angelscript/CHL2ASServerInitializer.h"

#include "angelscript/CASMapModuleBuilder.h"

#include "angelscript/ScriptAPI/CASEngine.h"

#include "angelscript/ScriptAPI/CASCustomEntityUtils.h"

#include "CHL2ASServerManager.h"
#include "CASLoggerInterface.h"

#if defined( CONTAGION )
#include "ThePresident.h"
#endif

#include "filesystem.h"

CHL2ASServerManager g_ASManager;

CHL2ASServerManager::CHL2ASServerManager()
{
	m_bReady = false;
}

bool CHL2ASServerManager::Initialize()
{
	CHL2ASServerInitializer initializer( *this );

	if ( !InitializeManager( initializer ) )
		return false;

	// Map scripts are per-map scripts that always have their hooks executed before any other module.
	auto descriptor = m_Manager.GetModuleManager().AddDescriptor( "MapScript", ModuleAccessMask::MAPSCRIPT, as::ModulePriority::HIGHEST );

	if ( !descriptor.first )
	{
		ASPrint( "Failed to add MapScript module type\n" );
		return false;
	}

	// Base scripts contains our entities and the like.
	descriptor = m_Manager.GetModuleManager().AddDescriptor( "Base", ModuleAccessMask::BASE, as::ModulePriority::HIGHEST - 1 );

	if ( !descriptor.first )
	{
		ASPrint( "Failed to add Base module type\n" );
		return false;
	}

	// Create the AS Schedule base (If it doesn't exist)
	if ( !g_ASSchedule )
		g_ASSchedule = new CASScheduleBase();

	// Make sure this exist
	if ( g_pFileSystem )
		g_pFileSystem->CreateDirHierarchy( "logs", "GAME" );

	// Create custom classes
	if ( !g_CustomEntities.Initialize() )
	{
		// Force Log it if it fails
		as::GameLog( as::LogType::LOG_ERR, "Failed to initialize Custom Entities system" );
	}

	return true;
}

void CHL2ASServerManager::Shutdown()
{
	if ( !m_bReady )
		return;

	// Purge it
	m_OncePrintLog.clear();

	g_CustomEntities.Shutdown();

	if ( m_pModule )
	{
		m_Manager.GetEventManager()->UnhookModuleFunctions( m_pModule );
		m_Manager.GetModuleManager().RemoveModule( m_pModule );
		m_pModule = nullptr;
	}

	if ( g_ASSchedule )
		g_ASSchedule->Shutdown();

#if defined( SOURCE )
	if ( g_ASRegisteredEntities )
		g_ASRegisteredEntities->Shutdown();
#endif

	CHL2ASManager::Shutdown();
}

void CHL2ASServerManager::Think()
{
	if ( !m_bReady )
		return;

	if ( m_pModule )
		m_pModule->GetScheduler()->Think( gpGlobals->time );

	if ( g_ASSchedule )
		g_ASSchedule->Think();

	gASHammerEvents->Update();
}

void CHL2ASServerManager::CallFunctionVoid( const char* const strFunctionVoid, bool bFuncError )
{
	for ( size_t uiIndex = 0; uiIndex < m_Manager.GetModuleManager().GetModuleCount(); ++uiIndex )
	{
		auto pModule = m_Manager.GetModuleManager().FindModuleByIndex( uiIndex );
		if ( const auto pFunction = pModule->GetModule()->GetFunctionByDecl( strFunctionVoid ) )
			as::Call( pFunction );
		else
		{
			if ( bFuncError )
				ASLogPrint( "CallFunctionVoid", ContagionImport::UTIL_CurrentMapLog(), as::LogType::LOG_ERR, "The function {limegreen}%s{default} failed to be executed on Module{cyan}%s{default}!", strFunctionVoid, pModule->GetModuleName() );
		}
	}
}

void CHL2ASServerManager::CallCoreFunctionVoid( const char* const strFunctionVoid, bool bFuncError )
{
	for ( size_t uiIndex = 0; uiIndex < m_Manager.GetModuleManager().GetModuleCount(); ++uiIndex )
	{
		auto pModule = m_Manager.GetModuleManager().FindModuleByIndex( uiIndex );
		if ( const auto pFunction = pModule->GetModule()->GetFunctionByDecl( strFunctionVoid ) )
			as::Call( pFunction );
		else
		{
			if ( bFuncError )
				ASLogPrint( "CallCoreFunctionVoid", ContagionImport::UTIL_CurrentMapLog(), as::LogType::LOG_ERR, "The function {limegreen}%s{default} failed to be executed on Module{cyan}%s{default}!", strFunctionVoid, pModule->GetModuleName() );
		}
	}
}

void CHL2ASServerManager::WorldCreated( const char* const pszMapScriptFileName )
{
	if ( !m_bReady )
		return;

	g_CustomEntities.WorldCreated();

	CASMapModuleBuilder builder( pszMapScriptFileName );

	m_pModule = m_Manager.GetModuleManager().BuildModule( "MapScript", "MapModule", builder );

	if ( !m_pModule )
		ASPrint( "Failed to create map script");

	CallCoreFunctionVoid( "void CoreInit()" );
	CallFunctionVoid( "void OnMapInit()", true );
}

void CHL2ASServerManager::WorldEnded()
{
	if ( !m_bReady )
		return;

	g_CustomEntities.WorldEnded();

	CallFunctionVoid( "void OnMapShutdown()", true );

	if ( m_pModule )
	{
		m_pModule->GetModule()->UnbindAllImportedFunctions();
		// Get rid of all hooks from this map script
		m_Manager.GetEventManager()->UnhookModuleFunctions( m_pModule );
		m_Manager.GetModuleManager().RemoveModule( m_pModule );
		m_pModule = nullptr;
	}

	// Destroy all schedule tasks, so we don't break anything when map shutsdown
	if ( g_ASSchedule )
		g_ASSchedule->DestroyTasks();

	gASHammerEvents->Destroy();

#if defined( SOURCE )
	// Destroy all registered entities, we no longer have any use for them
	if ( g_ASRegisteredEntities )
		g_ASRegisteredEntities->Clear();
#endif
}

void CHL2ASServerManager::ASPrint( const char* strFormat, ... )
{
	char szBuffer[ 4096 ];
	szBuffer[ 0 ] = 0;

	va_list list;
	va_start( list, strFormat );
	vsnprintf( szBuffer, sizeof( szBuffer ), strFormat, list );
	va_end( list );

	std::string console_text( ANGELSCRIPT_TYPE_MSG );
	console_text += " " + std::string( ContagionImport::UTIL_StripColors( szBuffer ) );

	ALERT( at_console, "%s\n", console_text.c_str() );
}

void CHL2ASServerManager::ASLogPrint( const char* strTitle, const char *szFile, as::LogType eType, const char* strFormat, ... )
{
	char szBuffer[ 4096 ];
	szBuffer[ 0 ] = 0;

	va_list list;
	va_start( list, strFormat );
	vsnprintf( szBuffer, sizeof( szBuffer ), strFormat, list );
	va_end( list );

	std::string szType;
	if ( strTitle )
	{
		switch ( eType )
		{
			case as::LogType::LOG_DBG: szType = "{yellow}[{aquamarine}%title% >> Debug{yellow}] {default}:: "; break;
			case as::LogType::LOG_INF: szType = "{yellow}[{cyan}%title% >> Info{yellow}] {default}:: "; break;
			case as::LogType::LOG_WRN: szType = "{yellow}[{axis}%title% >> Warning{yellow}] {default}:: "; break;
			case as::LogType::LOG_ERR: szType = "{yellow}[{crimson}%title% >> Error{yellow}] {default}:: "; break;
			case as::LogType::LOG_ERR_CRIT: szType = "{yellow}[{crimson}%title% >> Error::Critical{yellow}] {default}:: "; break;
			case as::LogType::LOG_EXCEPTION: szType = "{yellow}[{crimson}%title% >> Error::Exception{yellow}] {default}:: "; break;
			default: break;
		}
		ContagionImport::UTIL_STDReplaceString( szType, "%title%", strTitle );
	}
	else
	{
		switch ( eType )
		{
			case as::LogType::LOG_DBG: szType = "{yellow}[{aquamarine}Debug{yellow}] {default}:: "; break;
			case as::LogType::LOG_INF: szType = "{yellow}[{cyan}Info{yellow}] {default}:: "; break;
			case as::LogType::LOG_WRN: szType = "{yellow}[{axis}Warning{yellow}] {default}:: "; break;
			case as::LogType::LOG_ERR: szType = "{yellow}[{crimson}Error{yellow}] {default}:: "; break;
			case as::LogType::LOG_ERR_CRIT: szType = "{yellow}[{crimson}Error::Critical{yellow}] {default}:: "; break;
			case as::LogType::LOG_EXCEPTION: szType = "{yellow}[{crimson}Error::Exception{yellow}] {default}:: "; break;
			default: break;
		}
	}

	time_t t = time( NULL );
	tm* timePtr = localtime( &t );
	if ( !timePtr ) return;

	std::string szOutput( "{yellow}[{darkturquoise}%year%-%month%-%day% {orange}%hr%:%min%:%sec%{yellow}] " + szType );
	ContagionImport::UTIL_STDReplaceString( szOutput, "%year%", std::to_string( timePtr->tm_year + 1900 ) );
	ContagionImport::UTIL_STDReplaceString( szOutput, "%month%", UTIL_VarArgs( "%02d", timePtr->tm_mon+1 ) );
	ContagionImport::UTIL_STDReplaceString( szOutput, "%day%", UTIL_VarArgs( "%02d", timePtr->tm_mday ) );
	ContagionImport::UTIL_STDReplaceString( szOutput, "%hr%", UTIL_VarArgs( "%02d", timePtr->tm_hour+1 ) );
	ContagionImport::UTIL_STDReplaceString( szOutput, "%min%", UTIL_VarArgs( "%02d", timePtr->tm_min+1 ) );
	ContagionImport::UTIL_STDReplaceString( szOutput, "%sec%", UTIL_VarArgs( "%02d", timePtr->tm_sec+1 ) );
	szOutput += szBuffer;
	szOutput += "\n";

	// Print
	ALERT( at_console, "%s\n", ContagionImport::UTIL_StripColors( szOutput.c_str() ) );

	// Append
	std::ofstream file( szFile, std::ios::app );
	if ( file )
	{
		file << ContagionImport::UTIL_StripColors( szOutput.c_str() );
		file.close();
	}
	// File not found? then create it
	else
	{
		file.open( szFile );
		if ( file )
		{
			file << ContagionImport::UTIL_StripColors( szOutput.c_str() );
			file.close();
		}
	}
}

void CHL2ASServerManager::ASLogPrintOnce( uint64 id, const char* strTitle, const char *szFile, as::LogType eType, const char* strFormat, ... )
{
	for ( const auto& log : m_OncePrintLog )
	{
		if ( FStrEq( log.file, szFile ) && log.id == id ) return;
	}

	LogSaved_t inf;
	snprintf( inf.file, sizeof( inf.file ), szFile );
	inf.id = id;

	m_OncePrintLog.emplace_back( std::move( inf ) );

	char szBuffer[ 4096 ];
	szBuffer[ 0 ] = 0;

	va_list list;
	va_start( list, strFormat );
	vsnprintf( szBuffer, sizeof( szBuffer ), strFormat, list );
	va_end( list );
	ASLogPrint( strTitle, szFile, eType, szBuffer );
}