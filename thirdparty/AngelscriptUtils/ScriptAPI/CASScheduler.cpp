#include <angelscript.h>

#include "AngelscriptUtils/CASManager.h"
#include "AngelscriptUtils/CASModule.h"

#include "AngelscriptUtils/util/ASLogging.h"
#include "AngelscriptUtils/util/ASUtil.h"
#include "AngelscriptUtils/util/ContextUtils.h"

#include "AngelscriptUtils/wrapper/ASCallable.h"
#include "AngelscriptUtils/wrapper/CASArguments.h"
#include "AngelscriptUtils/wrapper/CASContext.h"

#include "CASScheduler.h"

void CASScheduler::CScheduledFunction::Remove( asIScriptEngine& engine )
{
	if( m_bRemoved )
		return;

	m_bRemoved = true;

	if( m_pArguments )
	{
		delete m_pArguments;
		m_pArguments = nullptr;
	}

	if( m_pThis )
	{
		asITypeInfo* pType = engine.GetTypeInfoById( m_iTypeId );

		engine.ReleaseScriptObject( m_pThis, pType );
		m_pThis = nullptr;
	}

	as::SetRefPointer<asIScriptFunction>( m_pFunction, nullptr );
}

CASScheduler::CScheduledFunction::~CScheduledFunction()
{
	assert( m_bRemoved );
}

CASScheduler::CASScheduler( CASModule& owningModule )
	: m_OwningModule( owningModule )
{
	//Don't AddRef the owning module here; the scheduler is owned by it, so the module will never be freed if a strong reference is kept here
}

CASScheduler::~CASScheduler()
{
	//Should be empty by now.
	assert( !m_pFunctionListHead );
}

void CASScheduler::SetTimeoutHandler( asIScriptGeneric* pArguments )
{
	//The following arguments are always passed before the varargs part

	const auto& szFunctionName = *reinterpret_cast<const std::string*>( pArguments->GetArgAddress( 0 ) );
	const float flDelay = pArguments->GetArgFloat( 1 );

	//Always repeat only once
	reinterpret_cast<CASScheduler*>( pArguments->GetObject() )->SetInterval( szFunctionName, flDelay, 1, 2, *pArguments );
}

void CASScheduler::SetIntervalHandler( asIScriptGeneric* pArguments )
{
	//The following arguments are always passed before the varargs part

	const auto& szFunctionName = *reinterpret_cast<const std::string*>( pArguments->GetArgAddress( 0 ) );
	const float flRepeatTime = pArguments->GetArgFloat( 1 );
	const int iRepeatCount = pArguments->GetArgDWord( 2 );

	if( !iRepeatCount || iRepeatCount < CASScheduler::REPEAT_INF_TIMES )
	{
		as::log->critical(
			"Error: CScheduler::SetInterval: can only add function '{}' if repeat count is positive and non-zero, or REPEAT_INFINITE_TIMES!",
			szFunctionName );
		pArguments->SetReturnAddress( nullptr );
		return;
	}

	reinterpret_cast<CASScheduler*>( pArguments->GetObject() )->SetInterval( szFunctionName, flRepeatTime, iRepeatCount, 3, *pArguments );
}

void CASScheduler::SetInterval_NoArgs( asIScriptGeneric* pArguments )
{
	const auto& szFunctionName = *reinterpret_cast<const std::string*>( pArguments->GetArgAddress( 0 ) );
	const float flRepeatTime = pArguments->GetArgFloat( 1 );

	reinterpret_cast<CASScheduler*>( pArguments->GetObject() )->SetInterval( 
		szFunctionName, flRepeatTime, CASScheduler::REPEAT_INF_TIMES, 2, *pArguments );
}

void CASScheduler::SetTimeoutObj( asIScriptGeneric* pArguments )
{
	//The following arguments are always passed before the varargs part

	void* pThisPointer = pArguments->GetArgAddress( 0 );
	int iTypeId = pArguments->GetArgTypeId( 0 );

	const auto& szFunctionName = *reinterpret_cast<const std::string*>( pArguments->GetArgAddress( 1 ) );
	const float flDelay = pArguments->GetArgFloat( 2 );

	//Always repeat only once
	reinterpret_cast<CASScheduler*>( pArguments->GetObject() )->SetInterval( 
		pThisPointer, iTypeId, szFunctionName, flDelay, 1, 3, *pArguments );
}

void CASScheduler::SetIntervalObj( asIScriptGeneric* pArguments )
{
	//The following arguments are always passed before the varargs part

	void* pThisPointer = pArguments->GetArgAddress( 0 );
	int iTypeId = pArguments->GetArgTypeId( 0 );

	const auto& szFunctionName = *reinterpret_cast<const std::string*>( pArguments->GetArgAddress( 1 ) );
	const float flRepeatTime = pArguments->GetArgFloat( 2 );
	const int iRepeatCount = pArguments->GetArgDWord( 3 );

	if( !iRepeatCount || iRepeatCount < CASScheduler::REPEAT_INF_TIMES )
	{
		as::log->critical(
			"Error: CScheduler::SetInterval: can only add function '{}' if repeat count is positive and non-zero, or REPEAT_INFINITE_TIMES!",
			szFunctionName );
		pArguments->SetReturnAddress( nullptr );
		return;
	}

	reinterpret_cast<CASScheduler*>( pArguments->GetObject() )->SetInterval( 
		pThisPointer, iTypeId, szFunctionName, flRepeatTime, iRepeatCount, 4, *pArguments );
}

void CASScheduler::SetIntervalObj_NoArgs( asIScriptGeneric* pArguments )
{
	void* pThisPointer = pArguments->GetArgAddress( 0 );
	int iTypeId = pArguments->GetArgTypeId( 0 );

	const auto& szFunctionName = *reinterpret_cast<const std::string*>( pArguments->GetArgAddress( 1 ) );
	const float flRepeatTime = pArguments->GetArgFloat( 2 );

	reinterpret_cast<CASScheduler*>( pArguments->GetObject() )->SetInterval( 
		pThisPointer, iTypeId, szFunctionName, flRepeatTime, CASScheduler::REPEAT_INF_TIMES, 3, *pArguments );
}

void CASScheduler::SetInterval( const std::string& szFunctionName, float flRepeatTime, int iRepeatCount, asUINT uiStartIndex, asIScriptGeneric& arguments )
{
	SetInterval( nullptr, 0, szFunctionName, flRepeatTime, iRepeatCount, uiStartIndex, arguments );
}

void CASScheduler::SetInterval( void* pThis, int iTypeId, const std::string& szFunctionName, float flRepeatTime, int iRepeatCount, asUINT uiStartIndex, asIScriptGeneric& arguments )
{
	if( flRepeatTime < 0.0f )
	{
		as::log->critical( "Error: CScheduler::SetInterval: negative repeat time or delay is not allowed!" );
		arguments.SetReturnAddress( nullptr );
		return;
	}

	auto pEngine = arguments.GetEngine();

	bool bSuccess = true;

	CScheduledFunction* pFunc = nullptr;

	CASArguments* pArgs = new CASArguments();

	if( ( bSuccess = pArgs->SetArguments( arguments, uiStartIndex ) ) != false )
	{
		if( pThis && iTypeId & asTYPEID_OBJHANDLE )
			pThis = *reinterpret_cast<void**>( pThis );

		asITypeInfo* pType = pThis ? pEngine->GetTypeInfoById( iTypeId ) : nullptr;

		asIScriptFunction* pFunction = nullptr;

		if( pThis )
		{
			if( pType )
			{
				if( pType->GetFlags() & asOBJ_REF )
				{
					pFunction = as::FindFunction( *pEngine, as::CASMethodIterator( *pType ), szFunctionName, *pArgs );
				}
				else
				{
					as::log->critical( "Error: CScheduler::SetInterval: could not add '{}::{}::{}', object type must be a reference!", 
						pType->GetNamespace(), pType->GetName(), szFunctionName );
				}
			}
			else
			{
				as::log->critical( "Error: CScheduler::SetInterval: could not add function '{}', object type for this pointer not found!", szFunctionName );
			}
		}
		else
		{
			pFunction = as::FindFunction( *pEngine, as::CASFunctionIterator( *m_OwningModule.GetModule() ), szFunctionName, *pArgs );
		}

		if( pFunction )
		{
			//TODO: m_flLastTime may not be the same as the current time if Think isn't called every frame.

			pFunc = new CScheduledFunction( 
				pFunction,
				m_flLastTime + flRepeatTime,
				flRepeatTime,
				iRepeatCount,
				pThis,
				iTypeId,
				pArgs
				);

			if( pThis )
				pEngine->AddRefScriptObject( pThis, pType );

			//Add to either main list or thinking list
			CScheduledFunction** ppHead = m_bThinking ? &m_pThinkListHead : &m_pFunctionListHead;

			pFunc->SetNext( *ppHead );

			*ppHead = pFunc;

			//For the return value, so the engine doesn't release our internal ref
			pFunc->AddRef();
		}
		else
		{
			delete pArgs;
			as::log->critical( "Error: CScheduler::SetInterval: could not add function '{}', function not found", szFunctionName );
			bSuccess = false;
		}
	}
	else
	{
		as::log->critical( "Error: CScheduler::SetInterval: could not add function '{}', failed to parse arguments", szFunctionName );
		bSuccess = false;
	}

	arguments.SetReturnAddress( pFunc );
}

void CASScheduler::RemoveTimer( CScheduledFunction* pFunction )
{
	if( !pFunction )
		return;

	CScheduledFunction* pNext = m_pFunctionListHead;
	CScheduledFunction* pLast = nullptr;

	auto& engine = *m_OwningModule.GetModule()->GetEngine();

	bool bRemoved = false;

	while( pNext )
	{
		if( pNext == pFunction )
		{
			//If we're currently executing this function, remove it later.
			if( m_pCurrentFunction == pNext )
				m_bShouldRemove = true;
			else
				RemoveFunction( engine, pLast, pNext );

			bRemoved = true;
			break;
		}

		pLast = pNext;
		pNext = pNext->GetNext();
	}

	if( !bRemoved )
	{
		//Not found in the list, release reference given by Angelscript.
		pFunction->Release();
	}
}

CASScheduler::CScheduledFunction* CASScheduler::GetCurrentFunction() const
{
	if( m_pCurrentFunction )
		m_pCurrentFunction->AddRef();

	return m_pCurrentFunction;
}

void CASScheduler::Think( const float flCurrentTime )
{
	m_bThinking = true;

	CScheduledFunction* pNext = m_pFunctionListHead;
	CScheduledFunction* pLast = nullptr;
	CScheduledFunction* pNextNext = nullptr;

	bool bRemove;

	auto& engine = *m_OwningModule.GetModule()->GetEngine();

	{
		CASOwningContext context( engine );

		while( pNext )
		{
			bRemove = false;

			pNextNext = pNext->GetNext();

			m_pCurrentFunction = pNext;

			//Scripts can change the repeat count setting, so make sure to check it
			if( pNext->ShouldRemove() )
			{
				bRemove = true;
			}
			else
			{
				if( pNext->GetNextCallTime() <= flCurrentTime )
				{
					auto* pFunction = pNext->GetFunction();

					//Must happen before the actual call so scripts can "amend" the repeat count if they want to
					pNext->Called();

					//Set this now so scripts can modify it if they want to
					pNext->SetNextCallTime( flCurrentTime + pNext->GetRepeatTime() );

					bool bSuccess = false;

					if( auto pThis = pNext->GetThis() )
					{
						CASMethod method( *pFunction, context, pThis );

						bSuccess = method.CallArgs( CallFlag::NONE, *pNext->GetArguments() );
					}
					else
					{
						CASFunction function( *pFunction, context );

						bSuccess = function.CallArgs( CallFlag::NONE, *pNext->GetArguments() );
					}

					if( !bSuccess )
					{
						const auto szFunctionName = as::FormatFunctionName( *pFunction );
						as::log->critical( "Error: CScheduler::Think: execution of function {} failed!\n", 
										   szFunctionName );
					}

					//Could've been flagged for removal during the call.
					if( m_bShouldRemove || pNext->ShouldRemove() )
					{
						m_bShouldRemove = false;
						bRemove = true;
					}
				}
			}

			if( bRemove )
				RemoveFunction( engine, pLast, pNext );
			else
				pLast = pNext;

			pNext = pNextNext;
		}
	}

	m_flLastTime = flCurrentTime;

	m_pCurrentFunction = nullptr;

	m_bThinking = false;

	//Functions were scheduled while we were thinking, merge into the main list.
	if( m_pThinkListHead )
	{
		//pLast points to the last function in the main list; if any
		if( pLast )
			pLast->SetNext( m_pThinkListHead );
		else
			m_pFunctionListHead = m_pThinkListHead;

		m_pThinkListHead = nullptr;
	}
}

void CASScheduler::ClearTimerList()
{
	CScheduledFunction* pNext;

	auto& engine = *m_OwningModule.GetModule()->GetEngine();

	while( m_pFunctionListHead )
	{
		pNext = m_pFunctionListHead->GetNext();
		m_pFunctionListHead->Remove( engine );	//Remove all references to other objects. Prevents memory leaks and circular references.
		m_pFunctionListHead->Release();
		m_pFunctionListHead = pNext;
	}
}

void CASScheduler::AdjustTime( float flTime )
{
	CScheduledFunction* pNext = m_pFunctionListHead;

	while( pNext )
	{
		pNext->SetNextCallTime( pNext->GetNextCallTime() - flTime );
		pNext = pNext->GetNext();
	}
}

void CASScheduler::RemoveFunction( asIScriptEngine& engine, CScheduledFunction* pLast, CScheduledFunction* pCurrent )
{
	if( pLast )
		pLast->SetNext( pCurrent->GetNext() );
	else 
		m_pFunctionListHead = pCurrent->GetNext();

	pCurrent->Remove( engine );
	pCurrent->Release();
}

static void RegisterScriptScheduledFunction( asIScriptEngine* pEngine )
{
	const char* pszObjectName = "CScheduledFunction";
	pEngine->RegisterObjectType( pszObjectName, 0, asOBJ_REF );

	as::RegisterRefCountedBaseClass<CASScheduler::CScheduledFunction>( pEngine, pszObjectName );

	pEngine->RegisterObjectMethod(
		pszObjectName, "float GetNextCallTime() const", 
		asMETHOD( CASScheduler::CScheduledFunction, GetNextCallTime ), asCALL_THISCALL );

	pEngine->RegisterObjectMethod(
		pszObjectName, "void SetNextCallTime(const float flNextCallTime)", 
		asMETHOD( CASScheduler::CScheduledFunction, SetNextCallTime ), asCALL_THISCALL );

	pEngine->RegisterObjectMethod(
		pszObjectName, "float GetRepeatTime() const", 
		asMETHOD( CASScheduler::CScheduledFunction, GetRepeatTime ), asCALL_THISCALL );

	pEngine->RegisterObjectMethod(
		pszObjectName, "void SetRepeatTime(const float flRepeatTime)", 
		asMETHOD( CASScheduler::CScheduledFunction, SetRepeatTime ), asCALL_THISCALL );

	pEngine->RegisterObjectMethod(
		pszObjectName, "int GetRepeatCount() const",
		asMETHOD( CASScheduler::CScheduledFunction, GetRepeatCount ), asCALL_THISCALL );

	pEngine->RegisterObjectMethod(
		pszObjectName, "bool IsInfiniteRepeat() const",
		asMETHOD( CASScheduler::CScheduledFunction, IsInfiniteRepeat ), asCALL_THISCALL );

	pEngine->RegisterObjectMethod(
		pszObjectName, "void SetRepeatCount(const int iRepeatCount)",
		asMETHOD( CASScheduler::CScheduledFunction, SetRepeatCount ), asCALL_THISCALL );

	pEngine->RegisterObjectMethod(
		pszObjectName, "void MakeInfiniteRepeat()",
		asMETHOD( CASScheduler::CScheduledFunction, MakeInfiniteRepeat ), asCALL_THISCALL );

	pEngine->RegisterObjectMethod(
		pszObjectName, "bool HasBeenRemoved() const",
		asMETHOD( CASScheduler::CScheduledFunction, HasBeenRemoved ), asCALL_THISCALL );
}

void RegisterScriptScheduler( asIScriptEngine* pEngine )
{
	RegisterScriptScheduledFunction( pEngine );

	const char* pszObjectName = "CScheduler";

	pEngine->RegisterObjectType( 
		pszObjectName, sizeof( CASScheduler ), asOBJ_REF | asOBJ_NOCOUNT );

	pEngine->RegisterObjectProperty(
		pszObjectName, "const int REPEAT_INFINITE_TIMES", 
		asOFFSET( CASScheduler, REPEAT_INFINITE_TIMES ) );

	/*
	*	SetTimeout variants
	*/

	as::RegisterVarArgsMethod(
		*pEngine, pszObjectName, 
		"CScheduledFunction@", "SetTimeout", "const string& in szFunction, float flDelay",
		0, 8, 
		asFUNCTION( CASScheduler::SetTimeoutHandler ) );

	as::RegisterVarArgsMethod(
		*pEngine, pszObjectName, 
		"CScheduledFunction@",  "SetTimeout", "?& in thisObject, const string& in szFunction, float flDelay",
		0, 8,
		asFUNCTION( CASScheduler::SetTimeoutObj ) );

	/*
	*	SetInterval variants
	*/

	as::RegisterVarArgsMethod(
		*pEngine, pszObjectName,
		"CScheduledFunction@", "SetInterval", "const string& in szFunction, float flRepeatTime, int iRepeatCount",
		0, 8, asFUNCTION( CASScheduler::SetIntervalHandler ) );

	pEngine->RegisterObjectMethod(
		pszObjectName, "CScheduledFunction@ SetInterval(const string& in szFunction, float flRepeatTime)",
		asFUNCTION( CASScheduler::SetInterval_NoArgs ), asCALL_GENERIC );

	as::RegisterVarArgsMethod(
		*pEngine, pszObjectName,
		"CScheduledFunction@", "SetInterval", "?& in thisObject, const string& in szFunction, float flRepeatTime, int iRepeatCount",
		0, 8,
		asFUNCTION( CASScheduler::SetIntervalObj ) );

	pEngine->RegisterObjectMethod(
		pszObjectName, "CScheduledFunction@ SetInterval(?& in thisObject, const string& in szFunction, float flRepeatTime)",
		asFUNCTION( CASScheduler::SetIntervalObj_NoArgs ), asCALL_GENERIC );

	pEngine->RegisterObjectMethod(
		pszObjectName, "void RemoveTimer(CScheduledFunction@ pFunction)", 
		asMETHOD( CASScheduler, RemoveTimer ), asCALL_THISCALL );

	pEngine->RegisterObjectMethod(
		pszObjectName, "CScheduledFunction@ GetCurrentFunction() const",
		asMETHOD( CASScheduler, GetCurrentFunction ), asCALL_THISCALL );

	pEngine->RegisterObjectMethod(
		pszObjectName, "void ClearTimerList()", 
		asMETHOD( CASScheduler, ClearTimerList ), asCALL_THISCALL );
}
