#ifndef ANGELSCRIPT_IASCONTEXTRESULTHANDLER_H
#define ANGELSCRIPT_IASCONTEXTRESULTHANDLER_H

#include <angelscript.h>

#include "ASUtilsConfig.h"

#include "util/ASPlatform.h"

/**
*	Provides result handling functions for context operations.
*	Is reference counted.
*/
class IASContextResultHandler
{
public:
	virtual ~IASContextResultHandler() = 0;

	virtual void AddRef() const = 0;

	virtual void Release() const = 0;

	/**
	*	Processes the asIScriptContext::Prepare return value.
	*	@param function Function that was prepared for execution.
	*	@param context Context that was prepared.
	*	@param iResult Result code.
	*/
	virtual void ProcessPrepareResult( asIScriptFunction& function, asIScriptContext& context, int iResult );

	/**
	*	Processes the asIScriptContext::Execute return value.
	*	@param function Function that was executed.
	*	@param context Context that was used to execute the function.
	*	@param iResult Result code.
	*/
	virtual void ProcessExecuteResult( asIScriptFunction& function, asIScriptContext& context, int iResult );

	/**
	*	Processes the asIScriptContext::Unprepare return value.
	*	@param context Context that was unprepared.
	*	@param iResult Result code.
	*/
	virtual void ProcessUnprepareResult( asIScriptContext& context, int iResult );
};

inline IASContextResultHandler::~IASContextResultHandler()
{
}

inline void IASContextResultHandler::ProcessPrepareResult( asIScriptFunction&, asIScriptContext&, int )
{
}

inline void IASContextResultHandler::ProcessExecuteResult( asIScriptFunction&, asIScriptContext&, int )
{
}

inline void IASContextResultHandler::ProcessUnprepareResult( asIScriptContext&, int )
{
}

namespace as
{
/**
*	Gets the result handler from the given context.
*	Does not increment the reference count for the returned handler.
*	@return If the context has a result handler, returns the handler. Otherwise, returns null.
*/
inline IASContextResultHandler* GetContextResultHandler( const asIScriptContext& context )
{
	return reinterpret_cast<IASContextResultHandler*>( context.GetUserData( ASUTILS_CONTEXT_RESULTHANDLER_USERDATA_ID ) );
}

/**
*	Sets the result handler for the given context.
*	@param context Context to set the handler on.
*	@param pHandler Handler to set. Can be null.
*/
inline void SetContextResultHandler( asIScriptContext& context, IASContextResultHandler* pHandler )
{
	//AddRef the new one first in case it's the same handler that was already present.
	//This prevents the ref count from dropping to 0 unexpectedly.
	if( pHandler )
		pHandler->AddRef();

	auto pOldHandler = reinterpret_cast<IASContextResultHandler*>( context.SetUserData( pHandler, ASUTILS_CONTEXT_RESULTHANDLER_USERDATA_ID ) );

	if( pOldHandler )
		pOldHandler->Release();
}

/**
*	Callback used to free the context result handler.
*	Set this after creating the engine.
*/
inline void FreeContextResultHandler( asIScriptContext* pContext )
{
	as::SetContextResultHandler( *pContext, nullptr );
}
}

#endif //ANGELSCRIPT_IASCONTEXTRESULTHANDLER_H
