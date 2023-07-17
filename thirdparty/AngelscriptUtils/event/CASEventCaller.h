#ifndef ANGELSCRIPT_CASEVENTCALLER_H
#define ANGELSCRIPT_CASEVENTCALLER_H

#include "CASBaseEventCaller.h"

#include "CASEvent.h"

/**
*	@addtogroup ASEvents
*
*	@{
*/

/**
*	Return codes for functions that hook into an event.
*/
enum class HookReturnCode
{
	/**
	*	Continue executing.
	*/
	CONTINUE,

	/**
	*	The function handled the event, stop.
	*/
	HANDLED
};

/**
*	Result codes for hook invocation.
*/
enum class HookCallResult
{
	/**
	*	An error occurred while executing the hook.
	*/
	FAILED,

	/**
	*	No functions handled the hook.
	*/
	NONE_HANDLED,

	/**
	*	One or more functions handled the hook.
	*/
	HANDLED
};

/**
*	Class that can call CASEvent classes.
*/
class CASEventCaller : public CASBaseEventCaller<CASEventCaller, CASEvent, HookCallResult, HookCallResult::FAILED>
{
public:
	ReturnType_t CallEvent( EventType_t& event, asIScriptContext* pContext, CallFlags_t flags, va_list list );
};

/**
*	Registers the HookReturnCode enum.
*	@param engine Script engine.
*/
void RegisterScriptHookReturnCode( asIScriptEngine& engine );

/** @} */

#endif //ANGELSCRIPT_CASEVENTCALLER_H
