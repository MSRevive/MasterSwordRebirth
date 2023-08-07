#include <cassert>
#include <iostream>

#include <spdlog/spdlog.h>

#include <angelscript.h>

#include "util/ASLogging.h"
#include "util/ASUtil.h"

#include "IASContextResultHandler.h"
#include "IASInitializer.h"

#include "CASManager.h"

#include "std_make_unique.h"

CASManager* CASManager::m_pActiveManager = nullptr;

CASManager* CASManager::GetActiveManager()
{
	assert( m_pActiveManager );

	return m_pActiveManager;
}

void CASManager::ActivateManager( CASManager* pManager )
{
	m_pActiveManager = pManager;
}

void CASManager::Activate()
{
	ActivateManager( this );
}

void CASManager::Deactivate()
{
	if( this == m_pActiveManager )
	{
		m_pActiveManager = nullptr;
	}
}

CASManager::CASManager()
{
}

CASManager::~CASManager()
{
	assert( !m_pScriptEngine );
}

//Used to caller OnInitEnd autmatically.
struct InitEndCaller
{
	IASInitializer& initializer;
	bool bSuccess = false;

	InitEndCaller( IASInitializer& initializer )
		: initializer( initializer )
	{
	}

	~InitEndCaller()
	{
		initializer.OnInitEnd( bSuccess );
	}

private:
	InitEndCaller( const InitEndCaller& ) = delete;
	InitEndCaller& operator=( const InitEndCaller& ) = delete;
};

bool CASManager::Initialize( IASInitializer& initializer )
{
	if( !as::log )
	{
		as::log = spdlog::create<spdlog::sinks::wincolor_stderr_sink_mt>( "ASUtils" );
	}

	if( m_pScriptEngine )
	{
		return true;
	}

	m_pScriptEngine = asCreateScriptEngine( ANGELSCRIPT_VERSION );

	if( !m_pScriptEngine )
	{
		return false;
	}

	InitEndCaller initEndCaller( initializer );

	initializer.OnInitBegin();

	//Set the cleanup callback for the result handler.
	m_pScriptEngine->SetContextUserDataCleanupCallback( as::FreeContextResultHandler, ASUTILS_CONTEXT_RESULTHANDLER_USERDATA_ID );

	const bool bUseEventManager = initializer.UseEventManager();

	if( bUseEventManager )
	{
		m_EventManager = std::make_unique<CASEventManager>( *m_pScriptEngine, initializer.GetEventNamespace() );
	}

	m_ModuleManager = std::make_unique<CASModuleManager>( *m_pScriptEngine, m_EventManager );

	asSFuncPtr msgCallback;
	void* pObj;
	asDWORD callConv;

	if( !initializer.GetMessageCallback( msgCallback, pObj, callConv ) )
	{
		msgCallback = asMETHOD( CASManager, MessageCallback );
		pObj = this;
		callConv = asCALL_THISCALL;
	}

	m_pScriptEngine->SetMessageCallback( msgCallback, pObj, callConv );

	Activate();

	if( !initializer.RegisterCoreAPI( *this ) )
		return false;

	if( bUseEventManager )
	{
		if( !initializer.AddEvents( *this, *m_EventManager ) )
			return false;
	
		//Registers all events. One-time event that happens on startup.
		m_EventManager->RegisterEvents( *GetEngine() );
	}

	if( !initializer.RegisterAPI( *this ) )
		return false;

	initEndCaller.bSuccess = true;

	return true;
}

void CASManager::Shutdown()
{
	if( !m_pScriptEngine )
	{
		return;
	}

	Activate();

	if( m_EventManager )
	{
		//Unhook all functions to prevent dangling pointers.
		m_EventManager->UnhookAllFunctions();

		m_EventManager.reset();
	}

	if( m_ModuleManager )
	{
		//Clear all modules and destroy all descriptors.
		m_ModuleManager->Clear();
		m_ModuleManager.reset();
	}

	//Let it go.
	m_pScriptEngine->ShutDownAndRelease();
	m_pScriptEngine = nullptr;

	Deactivate();
}

void CASManager::MessageCallback( const asSMessageInfo* pMsg )
{
	const char* pType = "";

	//Get the prefix.
	switch( pMsg->type )
	{
	case asMSGTYPE_ERROR: pType = "Error: "; break;
	case asMSGTYPE_WARNING: pType = "Warning: "; break;
	default: break;
	}

	//Only display the section if it was actually set. Some messages are not triggered by script code compilation or execution.
	const bool bHasSection = pMsg->section && *pMsg->section;

	bool bNeedsNewline = false;

	if( bHasSection )
	{
		std::cout << "Section \"" << pMsg->section << "\"";
		bNeedsNewline = true;
	}

	//Some messages don't refer to script code, and set both to 0.
	if( pMsg->row != 0 && pMsg->col != 0 )
	{
		if( bHasSection )
			std::cout << ' ';

		std::cout << "(" << pMsg->row << ", " << pMsg->col << ")";
		bNeedsNewline = true;
	}

	if( bNeedsNewline )
		std::cout << std::endl;

	std::cout << pType << pMsg->message << std::endl;
}
