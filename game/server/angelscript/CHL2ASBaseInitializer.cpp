//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#include <angelscript/add_on/scriptstdstring.h>
#include <angelscript/add_on/scriptarray.h>
#include <angelscript/add_on/scriptdictionary.h>
#include <angelscript/add_on/scriptany.h>
#include <angelscript/add_on/scriptbuilder.h>

#ifdef WIN32
#include <angelscript/add_on/datetime.h>
#include <angelscript/add_on/scriptmath.h>
#include <angelscript/add_on/scriptmathcomplex.h>
#else
#include "add_on/datetime.h"
#include "add_on/scriptmath.h"
#include "add_on/scriptmathcomplex.h"
#endif

//#include "ScriptAPI/CASEvents.h"
#include "ScriptAPI/CASSchedule.h"
#include "ScriptAPI/CASGlobalVars.h"
#include "ScriptAPI/CASLog.h"
#include "ScriptAPI/CASMath.h"
//#include "ScriptAPI/CASFileSystem.h"
#include "cvardef.h"

#include "HL2ASConstants.h"

#include "CHL2ASManager.h"

#include "CHL2ASBaseInitializer.h"

#include "CASContagionImport.h"

#include "angelscript/CHL2ASServerManager.h"

#if defined( SPDLOG )
#include "spdlog_main.h"
#endif


extern cvar_t ms_as_debug;

bool CHL2ASBaseInitializer::GetMessageCallback( asSFuncPtr& outFuncPtr, void*& pOutObj, asDWORD& outCallConv )
{
	outFuncPtr = asMETHOD( CHL2ASManager, MessageCallback );
	pOutObj = &m_Manager;
	outCallConv = asCALL_THISCALL;

	return true;
}

bool CHL2ASBaseInitializer::RegisterCoreAPI( CASManager& manager )
{
	auto& engine = *manager.GetEngine();

	engine.SetDefaultAccessMask( ModuleAccessMask::ALL );

#ifdef SPDLOG
	SpdLog(
		true,
		"Angelscript Base Initializer",
		ContagionImport::UTIL_LogFolder( LOG_ANGELSCRIPT, false ),
		LOGTYPE_INFO,
		"Angelscript v%s",
		ANGELSCRIPT_VERSION_STRING
	);
	SpdLog(
		true,
		"Angelscript Base Initializer",
		ContagionImport::UTIL_LogFolder( LOG_ANGELSCRIPT, false ),
		LOGTYPE_INFO,
		"Registering Angelscript API"
	);
#else
#if !defined( ASCOMPILER )
	g_ASManager.ASLogPrint(
		"Angelscript Base Initializer",
		ContagionImport::UTIL_LogFolder( LOG_ANGELSCRIPT, false ),
		as::LogType::LOG_INF,
		"Angelscript v%s",
		ANGELSCRIPT_VERSION_STRING
	);
	g_ASManager.ASLogPrint(
		"Angelscript Base Initializer",
		ContagionImport::UTIL_LogFolder( LOG_ANGELSCRIPT, false ),
		as::LogType::LOG_INF,
		"Registering Angelscript API"
	);
#else
	Msg( "Angelscript v%s\n", ANGELSCRIPT_VERSION_STRING );
	Msg( "Registering Angelscript API\n" );
#endif
#endif

	RegisterStdString( &engine );
	RegisterScriptArray( &engine, true );
	RegisterScriptDictionary( &engine );
	RegisterScriptAny( &engine );
	RegisterScriptASMath( engine );

	RegisterScriptDateTime( &engine );
	RegisterScriptMath( &engine );
	RegisterScriptMathComplex( &engine );

	RegisterScriptSchedule( engine );

	//RegisterASEvents( engine );
	RegisterScriptEventAPI( engine );

	RegisterScriptCASGlobalVars( engine );
	RegisterScriptCASLog( engine );
	//RegisterScriptFileSystem( engine );

	return true;
}

bool CHL2ASBaseInitializer::RegisterAPI( CASManager& manager )
{
	auto& engine = *manager.GetEngine();
#if !defined( ASCOMPILER )
	if ( ms_as_debug.value )
	{
#ifdef SPDLOG
		SpdLog(
			true,
			"Angelscript Base Initializer",
			ContagionImport::UTIL_LogFolder( LOG_ANGELSCRIPT, false ),
			LOGTYPE_INFO,
			"Finished registering API"
		);
		SpdLog(
			true,
			"Angelscript Base Initializer",
			ContagionImport::UTIL_LogFolder( LOG_ANGELSCRIPT, false ),
			LOGTYPE_INFO,
			"%u Object types",
			engine.GetObjectTypeCount()
		);
		SpdLog(
			true,
			"Angelscript Base Initializer",
			ContagionImport::UTIL_LogFolder( LOG_ANGELSCRIPT, false ),
			LOGTYPE_INFO,
			"%u Global functions",
			engine.GetGlobalFunctionCount()
		);
		SpdLog(
			true,
			"Angelscript Base Initializer",
			ContagionImport::UTIL_LogFolder( LOG_ANGELSCRIPT, false ),
			LOGTYPE_INFO,
			"%u Global properties",
			engine.GetGlobalPropertyCount()
		);
		SpdLog(
			true,
			"Angelscript Base Initializer",
			ContagionImport::UTIL_LogFolder( LOG_ANGELSCRIPT, false ),
			LOGTYPE_INFO,
			"%u Enums",
			engine.GetEnumCount()
		);
		SpdLog(
			true,
			"Angelscript Base Initializer",
			ContagionImport::UTIL_LogFolder( LOG_ANGELSCRIPT, false ),
			LOGTYPE_INFO,
			"%u Funcdefs",
			engine.GetFuncdefCount()
		);
		SpdLog(
			true,
			"Angelscript Base Initializer",
			ContagionImport::UTIL_LogFolder( LOG_ANGELSCRIPT, false ),
			LOGTYPE_INFO,
			"%u Typedefs",
			engine.GetTypedefCount()
		);
#else
		g_ASManager.ASLogPrint(
			"Angelscript Base Initializer",
			ContagionImport::UTIL_LogFolder( LOG_ANGELSCRIPT, false ),
			as::LogType::LOG_INF,
			"Finished registering API"
		);
		g_ASManager.ASLogPrint(
			"Angelscript Base Initializer",
			ContagionImport::UTIL_LogFolder( LOG_ANGELSCRIPT, false ),
			as::LogType::LOG_INF,
			"%u Object types",
			engine.GetObjectTypeCount()
		);
		g_ASManager.ASLogPrint(
			"Angelscript Base Initializer",
			ContagionImport::UTIL_LogFolder( LOG_ANGELSCRIPT, false ),
			as::LogType::LOG_INF,
			"%u Global functions",
			engine.GetGlobalFunctionCount()
		);
		g_ASManager.ASLogPrint(
			"Angelscript Base Initializer",
			ContagionImport::UTIL_LogFolder( LOG_ANGELSCRIPT, false ),
			as::LogType::LOG_INF,
			"%u Global properties",
			engine.GetGlobalPropertyCount()
		);
		g_ASManager.ASLogPrint(
			"Angelscript Base Initializer",
			ContagionImport::UTIL_LogFolder( LOG_ANGELSCRIPT, false ),
			as::LogType::LOG_INF,
			"%u Enums",
			engine.GetEnumCount()
		);
		g_ASManager.ASLogPrint(
			"Angelscript Base Initializer",
			ContagionImport::UTIL_LogFolder( LOG_ANGELSCRIPT, false ),
			as::LogType::LOG_INF,
			"%u Funcdefs",
			engine.GetFuncdefCount()
		);
		g_ASManager.ASLogPrint(
			"Angelscript Base Initializer",
			ContagionImport::UTIL_LogFolder( LOG_ANGELSCRIPT, false ),
			as::LogType::LOG_INF,
			"%u Typedefs",
			engine.GetTypedefCount()
		);
#endif
	}
#endif
	return true;
}

bool CHL2ASBaseInitializer::AddEvents( CASManager& manager, CASEventManager& eventManager )
{
	return true;
}
