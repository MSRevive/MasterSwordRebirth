#ifndef ANGELSCRIPT_CASBASEEVENTCALLER_H
#define ANGELSCRIPT_CASBASEEVENTCALLER_H

#include <cassert>
#include <cstdarg>

#include <angelscript.h>
#include "AngelscriptUtils/wrapper/ASCallable.h"

/**
*	Base class for classes that can call events.
*	Uses the curiously recurring template pattern.
*
*	The subclass is required to provide a few things:
*	A method with this format:
*	ReturnType_t CallEvent( EventType_t& event, asIScriptContext* pContext, CallFlags_t flags, va_list list )
*	This method will perform the actual call to the hook.
*
*	@tparam SUBCLASS Class that inherits from this class.
*	@tparam EVENTTYPE Represents the type of the event being called.
*	@tparam RETURNTYPE Type that will be returned by call methods.
*	@tparam FAILEDRETURNVAL Value to return when a call fails.
*/
template<typename SUBCLASS, typename EVENTTYPE, typename RETURNTYPE, RETURNTYPE FAILEDRETURNVAL>
class CASBaseEventCaller
{
public:
	/**
	*	Type of the subclass.
	*/
	typedef SUBCLASS SubClass_t;

	/**
	*	Type of the event being called.
	*/
	typedef EVENTTYPE EventType_t;

	/**
	*	Type of the return value.
	*/
	typedef RETURNTYPE ReturnType_t;

	/**
	*	Return value when the call fails.
	*/
	static const RETURNTYPE FAILED_RETURN_VALUE = FAILEDRETURNVAL;

public:
	//Can be instanced and have member vars for call specific state.
	CASBaseEventCaller() = default;
	~CASBaseEventCaller() = default;

	//Copyable
	CASBaseEventCaller( const CASBaseEventCaller& other ) = default;
	CASBaseEventCaller& operator=( const CASBaseEventCaller& ) = default;

	//Movable
	CASBaseEventCaller( CASBaseEventCaller&& other ) {}
	CASBaseEventCaller& operator=( CASBaseEventCaller&& ) {}

	/**
	*	Forwards the call to the subclass.
	*/
	inline ReturnType_t VCall( EventType_t& event, asIScriptContext* pContext, CallFlags_t flags, va_list list )
	{
		//Take care of some common bookkeeping here.
		assert( pContext );

		if( !pContext )
			return FAILED_RETURN_VALUE;

		IncrementCallCount( event );

		auto result = static_cast<SubClass_t*>( this )->CallEvent( event, pContext, flags, list );

		DecrementCallCount( event );

		assert( GetCallCount( event ) >= 0 );

		//Clear any removed hooks.
		event.ClearRemovedHooks();

		return result;
	}

	/**
	*	Calls the given event using the given context.
	*	@param event Event to call.
	*	@param pContext Context to use.
	*	@param list Arguments.
	*/
	inline ReturnType_t VCall( EventType_t& event, asIScriptContext* pContext, va_list list )
	{
		return VCall( event, pContext, CallFlag::NONE, list );
	}

	/**
	*	Calls the given event using a context acquired from the given engine.
	*	@param event Event to call.
	*	@param pScriptEngine Script engine to use.
	*	@param flags Call flags.
	*	@param list Arguments.
	*/
	inline ReturnType_t VCall( EventType_t& event, asIScriptEngine* pScriptEngine, CallFlags_t flags, va_list list )
	{
		auto pContext = pScriptEngine->RequestContext();

		auto result = VCall( event, pContext, flags, list );

		pScriptEngine->ReturnContext( pContext );

		return result;
	}

	/**
	*	Calls the given event using a context acquired from the given engine.
	*	@param event Event to call.
	*	@param pScriptEngine Script engine to use.
	*	@param list Arguments.
	*/
	inline ReturnType_t VCall( EventType_t& event, asIScriptEngine* pScriptEngine, va_list list )
	{
		auto pContext = pScriptEngine->RequestContext();

		auto result = VCall( event, pContext, CallFlag::NONE, list );

		pScriptEngine->ReturnContext( pContext );

		return result;
	}

	/**
	*	Calls the given event using the given context.
	*	@param event Event to call.
	*	@param pContext Context to use.
	*	@param flags Flags.
	*	@param ... Arguments.
	*/
	inline ReturnType_t Call( EventType_t& event, asIScriptContext* pContext, CallFlags_t flags, ... )
	{
		va_list list;

		va_start( list, flags );

		auto result = VCall( event, pContext, flags, list );

		va_end( list );

		return result;
	}

	/**
	*	@see Call( EventType_t& event, asIScriptContext* pContext, CallFlags_t flags, ... )
	*/
	inline ReturnType_t Call( EventType_t& event, asIScriptContext* pContext, ... )
	{
		va_list list;

		va_start( list, pContext );

		auto result = VCall( event, pContext, list );

		va_end( list );

		return result;
	}

	/**
	*	Calls the given event using a context acquired from the given engine.
	*	@param event Event to call.
	*	@param pScriptEngine Script engine to use.
	*	@param flags Call flags.
	*	@param ... Arguments.
	*/
	inline ReturnType_t Call( EventType_t& event, asIScriptEngine* pScriptEngine, CallFlags_t flags, ... )
	{
		va_list list;

		va_start( list, flags );

		auto result = VCall( event, pScriptEngine, flags, list );

		va_end( list );

		return result;
	}

	/**
	*	Calls the given event using a context acquired from the given engine.
	*	@param event Event to call.
	*	@param pScriptEngine Script engine to use.
	*	@param ... Arguments.
	*/
	inline ReturnType_t Call( EventType_t& event, asIScriptEngine* pScriptEngine, ... )
	{
		va_list list;

		va_start( list, pScriptEngine );

		auto result = VCall( event, pScriptEngine, list );

		va_end( list );

		return result;
	}

protected:
	//These provide access to the event's call counter
	int GetCallCount( EventType_t& event )
	{
		return event.GetCallCount();
	}

	void IncrementCallCount( EventType_t& event )
	{
		event.IncrementCallCount();
	}

	void DecrementCallCount( EventType_t& event )
	{
		event.DecrementCallCount();
	}
};

#endif //ANGELSCRIPT_CASBASEEVENTCALLER_H
