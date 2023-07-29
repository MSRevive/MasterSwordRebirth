#include "CASEventCaller.h"

CASEventCaller::ReturnType_t CASEventCaller::CallEvent( EventType_t& event, asIScriptContext* pContext, CallFlags_t flags, va_list list )
{
	CASContext ctx( *pContext );

	bool bSuccess = true;

	HookReturnCode returnCode = HookReturnCode::CONTINUE;

	asIScriptModule* pLastModule = nullptr;

	decltype( event.GetFunctionByIndex( 0 ) ) pFunc;

	for( decltype( event.GetFunctionCount() ) index = 0; index < event.GetFunctionCount(); ++index )
	{
		pFunc = event.GetFunctionByIndex( index );

		if( !pFunc )
		{
			//Function was removed in a hook call, skip.
			continue;
		}

		if( event.GetStopMode() == EventStopMode::MODULE_HANDLED && returnCode == HookReturnCode::HANDLED )
		{
			//A hook in the last module handled it, so stop.
			if( pLastModule && pLastModule != pFunc->GetModule() )
				break;
		}

		pLastModule = pFunc->GetModule();

		CASFunction func( *pFunc, ctx );

		//The hook might remove itself from the list, so make sure we still have a strong reference.
		pFunc->AddRef();

		const auto successCall = func.VCall( flags, list );

		pFunc->Release();

		bSuccess = successCall && bSuccess;

		//Only check if a HANDLED value was returned if we're still continuing.
		if( successCall && returnCode == HookReturnCode::CONTINUE )
		{
			bSuccess = func.GetReturnValue( &returnCode ) && bSuccess;
		}

		if( returnCode == HookReturnCode::HANDLED )
		{
			if( event.GetStopMode() == EventStopMode::ON_HANDLED )
				break;
		}
	}

	if( !bSuccess )
		return HookCallResult::FAILED;

	return returnCode == HookReturnCode::HANDLED ? HookCallResult::HANDLED : HookCallResult::NONE_HANDLED;
}

void RegisterScriptHookReturnCode( asIScriptEngine& engine )
{
	const char* const pszObjectName = "HookReturnCode";

	int result = engine.RegisterEnum( pszObjectName );

	assert( result >= 0 );

	result = engine.RegisterEnumValue( pszObjectName, "HOOK_CONTINUE", static_cast<int>( HookReturnCode::CONTINUE ) );
	assert( result >= 0 );

	result = engine.RegisterEnumValue( pszObjectName, "HOOK_HANDLED", static_cast<int>( HookReturnCode::HANDLED ) );
	assert( result >= 0 );
}
