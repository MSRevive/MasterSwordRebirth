#ifndef WRAPPER_CASCALLABLE_H
#define WRAPPER_CASCALLABLE_H

#include <cassert>
#include <cstdarg>

#include <angelscript.h>

#include "AngelscriptUtils/util/ASPlatform.h"
#include "AngelscriptUtils/util/ContextUtils.h"

#include "AngelscriptUtils/IASContextResultHandler.h"

#include "ASCallableConst.h"
#include "CASContext.h"

class CASContext;
class CASArguments;

/**
*	@defgroup ASCallable Angelscript Call Utils
*
*	@{
*/

namespace as
{
/**
*	Calls a function.
*	@param callable Callable object.
*	@param flags Call flags.
*	@param args The arguments for the function.
*	@tparam CALLABLE Type of the callable object.
*	@tparam ARGS Arguments list type.
*	@return true on success, false otherwise.
*/
template<typename CALLABLE, typename ARGS>
bool CallFunction( CALLABLE& callable, CallFlags_t, const ARGS& args )
{
	auto pContext = callable.GetContext().GetContext();

	assert( pContext );

	if( !pContext )
		return false;

	auto& function = callable.GetFunction();

	auto result = pContext->Prepare( &function );

	auto pResultHandler = as::GetContextResultHandler( *pContext );

	if( pResultHandler )
		pResultHandler->ProcessPrepareResult( function, *pContext, result );

	if( result < 0 )
	{
		return false;
	}

	if( !callable.PreSetArguments() )
		return false;

	auto success = ctx::SetArguments( function, *pContext, args );

	if( !success )
		return false;

	if( !callable.PreExecute() )
		return false;

	result = pContext->Execute();

	if( pResultHandler )
		pResultHandler->ProcessExecuteResult( function, *pContext, result );

	if( !callable.PostExecute( result ) )
		return false;

	return result >= 0;
}
}

/**
*	Base class for callable types.
*	Do not use directly.
*/
class CASCallable
{
protected:
	template<typename CALLABLE, typename ARGS>
	friend bool as::CallFunction( CALLABLE& callable, CallFlags_t flags, const ARGS& args );

public:
	/**
	*	Constructor.
	*	@param function Function to call.
	*	@param context Context to use for calls.
	*/
	CASCallable( asIScriptFunction& function, CASContext& context );

	/**
	*	@return The function.
	*/
	asIScriptFunction& GetFunction() { return m_Function; }

	/**
	*	@return The context.
	*/
	CASContext& GetContext() { return m_Context; }

	/**
	*	@return Whether this function is valid.
	*/
	bool IsValid() const;

	/**
	*	Gets the return value.
	*	@param pReturnValue Pointer to the variable that will receive the return value. Must match the type being retrieved.
	*	@return true if the value was successfully retrieved, false otherwise.
	*/
	bool GetReturnValue( void* pReturnValue );

protected:
	/**
	*	Called before the arguments are set. Lets the callable type evaluate itself.
	*	Non-virtual, uses templates to invoke the correct type.
	*	@return true if the call should continue, false otherwise.
	*/
	bool PreSetArguments() { return true; }

	/**
	*	Called after arguments are set, before the function is executed. Lets the callable type evaluate itself.
	*	Non-virtual, uses templates to invoke the correct type.
	*	@return true if the call should continue, false otherwise.
	*/
	bool PreExecute() { return true; }

	/**
	*	Called after the function is executed. Lets the callable type evaluate itself.
	*	Non-virtual, uses templates to invoke the correct type.
	*	@param iResult Result of the asIScriptContext::Execute call.
	*	@return true if the call should continue, false otherwise.
	*/
	bool PostExecute( const int iResult );

private:
	asIScriptFunction& m_Function;
	CASContext& m_Context;

private:
	CASCallable( const CASCallable& ) = delete;
	CASCallable& operator=( const CASCallable& ) = delete;
};

inline CASCallable::CASCallable( asIScriptFunction& function, CASContext& context )
	: m_Function( function )
	, m_Context( context )
{
}

inline bool CASCallable::IsValid() const
{
	return m_Context.GetContext() != nullptr;
}

inline bool CASCallable::GetReturnValue( void* pReturnValue )
{
	assert( m_Context );

	asDWORD uiFlags;
	const int iTypeId = m_Function.GetReturnTypeId( &uiFlags );

	return ctx::GetReturnValue( *m_Context.GetContext(), iTypeId, uiFlags, pReturnValue );
}

inline bool CASCallable::PostExecute( const int )
{
	return true;
}

/**
*	Helper class that defines the call methods for callable types.
*	Do not use directly.
*/
template<typename SUBCLASS>
class CASTCallable : public CASCallable
{
public:
	CASTCallable( asIScriptFunction& function, CASContext& context )
		: CASCallable( function, context )
	{
	}

	/**
	*	Calls the function.
	*	@param flags Call flags.
	*	@param list Pointer to a va_list that contains the arguments for the function.
	*	@return true on success, false otherwise.
	*/
	bool VCall( CallFlags_t flags, va_list list )
	{
		return as::CallFunction( GetThisRef(), flags, list );
	}

	/**
	*	Calls the function.
	*	@param flags Call flags.
	*	@param ... The arguments for the function.
	*	@return true on success, false otherwise.
	*/
	bool Call( CallFlags_t flags, ... )
	{
		va_list list;

		va_start( list, flags );

		const auto success = VCall( flags, list );

		va_end( list );

		return success;
	}

	/**
	*	Calls the function.
	*	@param flags Call flags.
	*	@param args List of arguments.
	*	@return true on success, false otherwise.
	*/
	bool CallArgs( CallFlags_t flags, const CASArguments& args )
	{
		return as::CallFunction( GetThisRef(), flags, args );
	}

	/**
	*	Calls the function.
	*	@param flags Call flags.
	*	@param ... The arguments for the function.
	*	@return true on success, false otherwise.
	*/
	bool operator()( CallFlags_t flags, ... )
	{
		va_list list;

		va_start( list, flags );

		const auto success = VCall( flags, list );

		va_end( list );

		return success;
	}

private:
	inline SUBCLASS& GetThisRef() { return static_cast<SUBCLASS&>( *this ); }
};

/**
*	A regular function.
*/
class CASFunction final : public CASTCallable<CASFunction>
{
public:
	CASFunction( asIScriptFunction& function, CASContext& context )
		: CASTCallable( function, context )
	{
	}
};

/**
*	An object method.
*/
class CASMethod final : public CASTCallable<CASMethod>
{
protected:
	template<typename CALLABLE, typename ARGS>
	friend bool as::CallFunction( CALLABLE& callable, CallFlags_t flags, const ARGS& args );

public:
	/**
	*	@copydoc CASCallable::CASCallable( asIScriptFunction& function, CASContext& context )
	*	@param pThis Pointer to the object instance.
	*/
	CASMethod( asIScriptFunction& function, CASContext& context, const void* pThis );

	bool IsValid() const;

protected:
	bool PreSetArguments();

private:
	void* m_pThis;
};

inline CASMethod::CASMethod( asIScriptFunction& function, CASContext& context, const void* pThis )
	: CASTCallable( function, context )
	, m_pThis( const_cast<void*>( pThis ) )
{
	assert( pThis );
}

inline bool CASMethod::IsValid() const
{
	return CASCallable::IsValid() && m_pThis;
}

inline bool CASMethod::PreSetArguments()
{
	auto pContext = GetContext().GetContext();

	return pContext->SetObject( m_pThis ) >= 0;
}

namespace as
{
/**
*	Performs a function call.
*	Do not use directly.
*	@param functor Functor that Can perform function calls.
*	@param pContext Script context. If null, acquires a context using asIScriptEngine::RequestContext.
*	@param flags Call flags.
*	@param pFunction Function to call.
*	@param args Argument list to use for the call.
*	@tparam FUNCTOR Functor type.
*	@tparam ARGS Argument list type.
*	@return true on success, false otherwise.
*/
template<typename FUNCTOR, typename ARGS>
inline bool VCall( FUNCTOR functor, asIScriptContext* pContext, CallFlags_t flags, asIScriptFunction* pFunction, ARGS& args )
{
	assert( pFunction );

	if( !pFunction )
		return false;

	if( pContext )
	{
		CASContext ctx( *pContext );

		return functor( *pFunction, ctx, flags, args );
	}
	else
	{
		CASOwningContext ctx( *pFunction->GetEngine() );

		return functor( *pFunction, ctx, flags, args );
	}
}

/**
*	Internal helper functor used for calls.
*/
struct CASFunctionFunctor final
{
	/**
	*	Calls a function using varargs.
	*	@param function Function to call.
	*	@param context Context to use.
	*	@param flags Call flags.
	*	@param list List of arguments.
	*/
	bool operator()( asIScriptFunction& function, CASContext& context, CallFlags_t flags, va_list list )
	{
		CASFunction func( function, context );

		if( !func.IsValid() )
			return false;

		return func.VCall( flags, list );
	}

	/**
	*	Calls a function using an argument list.
	*	@param function Function to call.
	*	@param context Context to use.
	*	@param flags Call flags.
	*	@param args List of arguments.
	*/
	bool operator()( asIScriptFunction& function, CASContext& context, CallFlags_t flags, const CASArguments& args )
	{
		CASFunction func( function, context );

		if( !func.IsValid() )
			return false;

		return func.CallArgs( flags, args );
	}
};

/**
*	Internal helper functor used for calls.
*/
struct CASMethodFunctor final
{
	void* const pThis;

	/**
	*	Constructor.
	*	@param pThis This pointer.
	*/
	CASMethodFunctor( void* pThis )
		: pThis( pThis )
	{
	}

	CASMethodFunctor( const CASMethodFunctor& other ) = default;

	/**
	*	Calls an object method using varargs.
	*	@param function Function to call.
	*	@param context Context to use.
	*	@param flags Call flags.
	*	@param list List of arguments.
	*/
	bool operator()( asIScriptFunction& function, CASContext& context, CallFlags_t flags, va_list list )
	{
		CASMethod method( function, context, pThis );

		if( !method.IsValid() )
			return false;

		return method.VCall( flags, list );
	}

	/**
	*	Calls an object method using an argument list.
	*	@param function Function to call.
	*	@param context Context to use.
	*	@param flags Call flags.
	*	@param args List of arguments.
	*/
	bool operator()( asIScriptFunction& function, CASContext& context, CallFlags_t flags, const CASArguments& args )
	{
		CASMethod method( function, context, pThis );

		if( !method.IsValid() )
			return false;

		return method.CallArgs( flags, args );
	}

private:
	CASMethodFunctor& operator=( const CASMethodFunctor& ) = delete;
};

/**
*	Overload that calls global functions.
*	Do not use directly.
*	@param pContext Script context. If null, acquires a context using asIScriptEngine::RequestContext.
*	@param flags Call flags.
*	@param pFunction Function to call.
*	@param args Argument list to use for the call.
*	@tparam ARGS Argument list type.
*	@return true on success, false otherwise.
*/
template<typename ARGS>
inline bool VCallFunc( asIScriptContext* pContext, CallFlags_t flags, asIScriptFunction* pFunction, ARGS& args )
{
	return VCall( CASFunctionFunctor(), pContext, flags, pFunction, args );
}

/**
*	Overload that calls object methods.
*	Do not use directly.
*	@param pThis This pointer.
*	@param pContext Script context. If null, acquires a context using asIScriptEngine::RequestContext.
*	@param flags Call flags.
*	@param pFunction Function to call.
*	@param args Argument list to use for the call.
*	@tparam ARGS Argument list type.
*	@return true on success, false otherwise.
*/
template<typename ARGS>
inline bool VCallFunc( void* pThis, asIScriptContext* pContext, CallFlags_t flags, asIScriptFunction* pFunction, ARGS& args )
{
	return VCall( CASMethodFunctor( pThis ), pContext, flags, pFunction, args );
}

/**
*	Implements a single function call.
*/
#define __IMPLEMENT_SINGLE_CALL( call, preCall, postCall )	\
	preCall()												\
	const auto success = call;								\
	postCall()												\
	return success

/**
*	Implements function overloads for calling functions or object methods.
*	@param funcCallName Name of the functions to define for global function calls.
*	@param methodCallName Name of the functions to define for object method calls.
*	@param funcToCall Which function to call to perform the call.
*	@param argType Argument list argument type and name.
*	@param argName Argument list name.
*	@param preCall Function macro to use before a call occurs.
*	@param postCall Function macro to use after a call has occurred.
*/
#define __IMPLEMENT_CALL( funcCallName, methodCallName, funcToCall, argType, argName, preCall, postCall )						\
inline bool funcCallName( asIScriptContext* pContext, CallFlags_t flags, asIScriptFunction* pFunction, argType )				\
{																																\
	__IMPLEMENT_SINGLE_CALL( ( funcToCall( pContext, flags, pFunction, argName ) ), preCall, postCall );						\
}																																\
																																\
inline bool funcCallName( CallFlags_t flags, asIScriptFunction* pFunction, argType )											\
{																																\
	__IMPLEMENT_SINGLE_CALL( 																									\
		( funcToCall( static_cast<asIScriptContext*>( nullptr ), flags, pFunction, argName ) ), 								\
		preCall, postCall );																									\
}																																\
																																\
inline bool funcCallName( asIScriptContext* pContext, asIScriptFunction* pFunction, argType )									\
{																																\
	__IMPLEMENT_SINGLE_CALL( ( funcToCall( pContext, CallFlag::NONE, pFunction, argName ) ), preCall, postCall );				\
}																																\
																																\
inline bool funcCallName( asIScriptFunction* pFunction, argType )																\
{																																\
	__IMPLEMENT_SINGLE_CALL( 																									\
		( funcToCall( static_cast<asIScriptContext*>( nullptr ), CallFlag::NONE, pFunction, argName ) ), 						\
		preCall, postCall );																									\
}																																\
																																\
inline bool methodCallName( void* pThis, asIScriptContext* pContext, CallFlags_t flags, asIScriptFunction* pFunction, argType )	\
{																																\
	__IMPLEMENT_SINGLE_CALL( ( funcToCall( pThis, pContext, flags, pFunction, argName ) ), preCall, postCall );					\
}																																\
																																\
inline bool methodCallName( void* pThis, CallFlags_t flags, asIScriptFunction* pFunction, argType )								\
{																																\
	__IMPLEMENT_SINGLE_CALL( ( funcToCall( pThis, nullptr, flags, pFunction, argName ) ), preCall, postCall );					\
}																																\
																																\
inline bool methodCallName( void* pThis, asIScriptContext* pContext, asIScriptFunction* pFunction, argType )					\
{																																\
	__IMPLEMENT_SINGLE_CALL( ( funcToCall( pThis, pContext, CallFlag::NONE, pFunction, argName ) ), preCall, postCall );		\
}																																\
																																\
inline bool methodCallName( void* pThis, asIScriptFunction* pFunction, argType )												\
{																																\
	__IMPLEMENT_SINGLE_CALL( ( funcToCall( pThis, nullptr, CallFlag::NONE, pFunction, argName ) ), preCall, postCall );			\
}

/**
*	Simplified version of __IMPLEMENT_CALL that uses the same name for global function and object method call functions.
*/
#define __IMPLEMENT_CALL_SIMPLE( funcName, funcToCall, argType, argName, preCall, postCall )	\
__IMPLEMENT_CALL( funcName, funcName, funcToCall, argType, argName, preCall, postCall )

#define __DO_NOTHING()

/*
*	Implement calls for argument lists.
*/
__IMPLEMENT_CALL_SIMPLE( CallArgs, VCallFunc, const CASArguments& args, args, __DO_NOTHING, __DO_NOTHING )

/*
*	Implement calls for va_list types.
*/
__IMPLEMENT_CALL_SIMPLE( VCall, VCallFunc, va_list list, list, __DO_NOTHING, __DO_NOTHING )

#define __VA_ARG_PARAM ...

#define __BEGIN_VA_LIST()		\
 va_list list;					\
va_start( list, pFunction );

#define __END_VA_LIST()		\
va_end( list );

/*
*	Implement calls for varargs.
*/
__IMPLEMENT_CALL_SIMPLE( Call, VCallFunc, __VA_ARG_PARAM, list, __BEGIN_VA_LIST, __END_VA_LIST )

/*
*	Clean up macros
*/
#undef __IMPLEMENT_SINGLE_CALL
#undef __DO_NOTHING
#undef __VA_ARG_PARAM
#undef __BEGIN_VA_LIST
#undef __END_VA_LIST
}

/** @} */

#endif //WRAPPER_CASCALLABLE_H