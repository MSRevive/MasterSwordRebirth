#include <algorithm>
#include <cassert>

#include "AngelscriptUtils/util/ASLogging.h"
#include "AngelscriptUtils/util/ASUtil.h"

#include "AngelscriptUtils/CASModule.h"

#include "CASBaseEvent.h"

CASBaseEvent::CASBaseEvent( const asDWORD accessMask )
	: m_AccessMask( accessMask )
{
	assert( accessMask != 0 );
}

CASBaseEvent::~CASBaseEvent()
{
	RemoveAllFunctions();
}

size_t CASBaseEvent::GetFunctionCount() const
{
	return m_Functions.size();
}

asIScriptFunction* CASBaseEvent::GetFunctionByIndex( const size_t uiIndex ) const
{
	assert( uiIndex < m_Functions.size() );

	return m_Functions[ uiIndex ];
}

bool CASBaseEvent::AddFunction( asIScriptFunction* pFunction )
{
	assert( pFunction );

	//Don't add functions while this event is being invoked.
	if( IsTriggering() )
	{
		as::CASCallerInfo info;

		as::GetCallerInfo( info );

		as::log->critical( "CBaseEvent::AddFunction: {}({}, {}): Cannot add function while invoking event!", info.pszSection, info.iLine, info.iColumn );
		return false;
	}

	if( !pFunction )
		return false;

	if( std::find( m_Functions.begin(), m_Functions.end(), pFunction ) != m_Functions.end() )
		return true;

	m_Functions.push_back( pFunction );

	pFunction->AddRef();

	std::stable_sort( m_Functions.begin(), m_Functions.end(), []( const asIScriptFunction* pLHS, const asIScriptFunction* pRHS )
	{
		auto pLHSModule = GetModuleFromScriptFunction( pLHS );
		auto pRHSModule = GetModuleFromScriptFunction( pRHS );

		return ModuleLess( pLHSModule, pRHSModule );
	} );

	return true;
}

bool CASBaseEvent::Hook( void* pValue, const int iTypeId )
{
	assert( pValue );

	if( !pValue )
		return false;

	asIScriptFunction* pFunction = nullptr;

	if( !ValidateHookFunction( iTypeId, pValue, "HookFunction", pFunction ) )
	{
		return false;
	}

	return AddFunction( pFunction );
}

void CASBaseEvent::RemoveFunction( asIScriptFunction* pFunction )
{
	if( !pFunction )
		return;

	auto it = std::find( m_Functions.begin(), m_Functions.end(), pFunction );

	if( it == m_Functions.end() )
		return;

	( *it )->Release();

	if( IsTriggering() )
	{
		//Currently triggering, mark as removed.
		*it = nullptr;
	}
	else
	{
		//Remove now.
		m_Functions.erase( it );
	}
}

void CASBaseEvent::Unhook( void* pValue, const int iTypeId )
{
	assert( pValue );

	if( !pValue )
		return;

	asIScriptFunction* pFunction = nullptr;

	if( !ValidateHookFunction( iTypeId, pValue, "UnhookFunction", pFunction ) )
	{
		return;
	}

	RemoveFunction( pFunction );
}

void CASBaseEvent::RemoveFunctionsOfModule( CASModule* pModule )
{
	assert( pModule );

	//This method should never be called while in an event invocation.
	if( IsTriggering() )
	{
		assert( !"CBaseEvent::RemoveFunctionsOfModule: Module hooks should not be removed while invoking events!" );

		as::CASCallerInfo info;

		as::GetCallerInfo( info );

		as::log->critical( "CBaseEvent::RemoveFunctionsOfModule: {}({}, {}): Module hooks should not be removed while invoking events!", info.pszSection, info.iLine, info.iColumn );
		return;
	}

	if( !pModule )
		return;

	//These functions might be null in some edge cases due to hooks being removed in event calls.
	//Fixed version provided by HoraceWeebler - Solokiller
	m_Functions.erase(
		std::remove_if(
			m_Functions.begin(),
			m_Functions.end(),
			[ &pModule ]( const asIScriptFunction* pFunction ) -> bool
			{
				if( !pFunction )
					return true;
				else if( pModule == GetModuleFromScriptFunction( pFunction ) )
				{
					pFunction->Release();
					return true;
				}
				else
					return false;
			}
		),
		m_Functions.end()
	);
}

void CASBaseEvent::RemoveAllFunctions()
{
	//This method should never be called while in an event invocation.
	if( m_iInCallCount != 0 )
	{
		assert( !"CASEvent::RemoveAllFunctions: Hooks should not be removed while invoking events!" );
		as::log->critical( "CASEvent::RemoveAllFunctions: Hooks should not be removed while invoking events!" );

		if( auto pContext = asGetActiveContext() )
		{
			pContext->SetException( "Cannot remove event hooks during the event's invocation" );
		}

		return;
	}

	for( auto pFunc : m_Functions )
	{
		if( pFunc->GetDelegateFunction() )
			pFunc->Release();

		pFunc->Release();
	}

	m_Functions.clear();
}

bool CASBaseEvent::ValidateHookFunction( const int iTypeId, void* pObject, const char* const pszScope, asIScriptFunction*& pOutFunction ) const
{
	auto pEngine = asGetActiveContext()->GetEngine();

	pOutFunction = nullptr;

	asITypeInfo* pObjectType = pEngine->GetTypeInfoById( iTypeId );

	if( !pObjectType )
	{
		as::log->critical( "CBaseEvent::{}: unknown type!", pszScope );
		return false;
	}

	if( !( pObjectType->GetFlags() & asOBJ_FUNCDEF ) )
	{
		as::log->critical( "CBaseEvent::{}: Object is not a function or delegate!", pszScope );
		return false;
	}

	if( !pObject )
	{
		as::log->critical( "CBaseEvent::{}: Object is null!", pszScope );
		return false;
	}

	//pObjectType->GetTypeId() is -1 for some reason
	if( iTypeId & asTYPEID_OBJHANDLE )
	{
		pObject = *reinterpret_cast<void**>( pObject );
	}

	if( !pObject )
	{
		as::log->critical( "CBaseEvent::{}: Object is null!", pszScope );
		return false;
	}

	asIScriptFunction* pFunction = reinterpret_cast<asIScriptFunction*>( pObject );

	if( !pFunction )
	{
		as::log->critical( "CBaseEvent::{}: Null function passed!", pszScope );
		return false;
	}

	asIScriptFunction* const pFuncDef = GetFuncDef();

	assert( pFuncDef );

	if( !pFuncDef )
	{
		as::log->critical( "CBaseEvent::{}: No funcdef for event!", pszScope );
		return false;
	}

	//Verify the function format
	if( !pFuncDef->IsCompatibleWithTypeId( pFunction->GetTypeId() ) )
	{
		const auto szFunctionName = as::FormatFunctionName( *pFunction );

		as::log->critical( "CBaseEvent::{}: Function '{}' is incompatible with event '{}'!",
						   pszScope, szFunctionName, pFuncDef->GetName() );

		return false;
	}

	pOutFunction = pFunction;

	return true;
}

void CASBaseEvent::DumpHookedFunctions( const char* const pszName ) const
{
	if( pszName )
		as::log->info( "{}", pszName );

	for( size_t uiIndex = 0; uiIndex < GetFunctionCount(); ++uiIndex )
	{
		auto pFunc = GetFunctionByIndex( uiIndex );

		auto pModule = pFunc->GetModule();

		if( !pModule )
		{
			auto pDelegate = pFunc->GetDelegateFunction();

			if( pDelegate )
			{
				pModule = pDelegate->GetModule();
			}
		}

		if( !pModule )
		{
			as::log->info( "Null module!" );
			continue;
		}

		const auto szFunctionName = as::FormatFunctionName( *pFunc );
		as::log->info( "Module \"{}\", \"{}\"", pModule->GetName(), szFunctionName );
	}

	as::log->info( "End functions" );
}

void CASBaseEvent::ClearRemovedHooks()
{
	//Can happen when recursively triggering events.
	if( IsTriggering() )
		return;

	for( auto it = m_Functions.begin(); it != m_Functions.end(); )
	{
		if( !( *it ) )
		{
			it = m_Functions.erase( it );
		}
		else
		{
			++it;
		}
	}
}

void RegisterScriptCBaseEvent( asIScriptEngine& engine )
{
	const char* const pszObjectName = "CBaseEvent";

	engine.RegisterObjectType(
		pszObjectName, 0, asOBJ_REF | asOBJ_NOCOUNT );

	RegisterScriptCBaseEvent<CASBaseEvent>( engine, pszObjectName );
}
