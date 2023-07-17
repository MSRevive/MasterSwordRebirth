#include <cassert>

#include <angelscript.h>

#include "ScriptAPI/CASScheduler.h"

#include "CASModule.h"

CASModule::CASModule( asIScriptModule* pModule, const CASModuleDescriptor& descriptor, IASModuleUserData* pUserData )
	: m_pModule( pModule )
	, m_pDescriptor( &descriptor )
	, m_pScheduler( new CASScheduler( *this ) )
	, m_pUserData( pUserData )
{
	assert( pModule );

	pModule->SetUserData( this, ASUTILS_CASMODULE_USER_DATA_ID );
}

CASModule::~CASModule()
{
	SetUserData( nullptr );

	//Discard should've been called first.
	assert( !m_pModule );

	//Delete last, in case code calls it during destruction
	delete m_pScheduler;
}

void CASModule::Release() const
{
	if( InternalRelease() )
	{
		delete this;
	}
}

void CASModule::Discard()
{
	//Clears out the functions that might be holding references to this module
	m_pScheduler->ClearTimerList();

	if( m_pModule )
	{
		m_pModule->Discard();
		m_pModule = nullptr;
	}
}

const char* CASModule::GetModuleName() const
{
	assert( m_pModule );

	return m_pModule->GetName();
}

CASModule* GetModuleFromScriptModule( const asIScriptModule* pModule )
{
	assert( pModule );

	return reinterpret_cast<CASModule*>( pModule->GetUserData( ASUTILS_CASMODULE_USER_DATA_ID ) );
}

CASModule* GetModuleFromScriptFunction( const asIScriptFunction* pFunction )
{
	assert( pFunction );

	auto pDelegate = pFunction->GetDelegateFunction();

	auto pFunc = pDelegate ? pDelegate : pFunction;

	auto pModule = GetModuleFromScriptModule( pFunc->GetModule() );

	return pModule;
}

CASModule* GetModuleFromScriptContext( asIScriptContext* pContext )
{
	assert( pContext );

	auto pFunction = pContext->GetFunction( pContext->GetCallstackSize() - 1 );

	if( !pFunction )
		return nullptr;

	return GetModuleFromScriptFunction( pFunction );
}

asIScriptModule* GetScriptModuleFromScriptContext( asIScriptContext* pContext )
{
	assert( pContext );

	auto pFunction = pContext->GetFunction( pContext->GetCallstackSize() - 1 );

	if( !pFunction )
		return nullptr;

	return pFunction->GetModule();
}
