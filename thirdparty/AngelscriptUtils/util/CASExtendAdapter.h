#ifndef ANGELSCRIPT_UTIL_CASEXTENDADAPTER_H
#define ANGELSCRIPT_UTIL_CASEXTENDADAPTER_H

#include <cassert>

#include "IASExtendAdapter.h"

#include "CASObjPtr.h"

#include "AngelscriptUtils/wrapper/ASCallable.h"

/**
*	@addtogroup ASExtend
*
*	@{
*/

/**
*	Class used to adapt C++ classes for extension in Angelscript.
*	@tparam BASECLASS C++ class to extend. Should be a class with at least one virtual function.
*	@tparam DERIVEDCLASS C++ class that will be instantiated. Used to call object methods correctly.
*/
template<typename BASECLASS, typename DERIVEDCLASS>
class CASExtendAdapter : public BASECLASS, public IASExtendAdapter
{
public:
	typedef BASECLASS BaseClass;
	typedef CASExtendAdapter ThisClass;
	typedef DERIVEDCLASS DerivedClass;

public:
	/**
	*	Constructor.
	*	@param object Script object that extends this object.
	*/
	CASExtendAdapter( CASObjPtr object )
		: BASECLASS()
		, m_Object( object )
	{
		assert( object );
	}

	/**
	*	Constructor.
	*	@param object Script object that extends this object.
	*	@param args Arguments for the baseclass constructor.
	*	@tparam ARGS Argument types for the baseclass constructor.
	*/
	template<typename... ARGS>
	CASExtendAdapter( CASObjPtr object, ARGS&&... args )
		: BASECLASS( std::move( args )... )
		, m_Object( object )
	{
		assert( object );
	}

	/**
	*	@return The object instance.
	*/
	const CASObjPtr& GetObject() const override final
	{
		return m_Object;
	}

	/**
	*	@copydoc GetObject() const
	*/
	CASObjPtr GetObject() override final
	{
		return m_Object;
	}

private:
	CASObjPtr m_Object;

private:
	//TODO: consider adding support for copying.
	CASExtendAdapter( const CASExtendAdapter& ) = delete;
	CASExtendAdapter& operator=( const CASExtendAdapter& ) = delete;
};

/*
*	Helper macros.
*/

/**
*	Calls a method that has a different C++ method name, and returns non-void.
*	@param retType Return type.
*	@param methodName Script method name to call.
*	@param baseMethodName C++ method name to call.
*	@param pszParams Script method parameters.
*	@param ... Arguments to pass.
*/
#define CALL_EXTEND_FUNC_RET_DIFFFUNC( retType, methodName, baseMethodName, pszParams, ... )						\
retType result = retType();																							\
																													\
if( auto pFunction = GetObject().GetTypeInfo()->GetMethodByDecl( #retType " " #methodName pszParams ) )				\
{																													\
	CASOwningContext ctx( *pFunction->GetEngine() );																\
																													\
	CASMethod method( *pFunction, ctx, GetObject().Get() );															\
																													\
	if( method.Call( CallFlag::NONE, ##__VA_ARGS__ ) )																\
	{																												\
		method.GetReturnValue( &result );																			\
	}																												\
}																													\
else																												\
{																													\
	result = baseMethodName( __VA_ARGS__ );																			\
}																													\
																													\
return result

/**
*	Calls a method, and returns non-void.
*	@param retType Return type.
*	@param methodName Script or C++ method name to call.
*	@param pszParams Script method parameters.
*	@param ... Arguments to pass.
*/
#define CALL_EXTEND_FUNC_RET( retType, methodName, pszParams, ... )									\
CALL_EXTEND_FUNC_RET_DIFFFUNC( retType, methodName, BaseClass::methodName, pszParams, ##__VA_ARGS__ )

/**
*	Calls a method that has a different C++ method name.
*	@param methodName Script method name to call.
*	@param baseMethodName C++ method name to call.
*	@param pszParams Script method parameters.
*	@param ... Arguments to pass.
*/
#define CALL_EXTEND_FUNC_DIFFFUNC( methodName, baseMethodName, pszParams, ... )								\
if( auto pFunction = GetObject().GetTypeInfo()->GetMethodByDecl( "void " #methodName pszParams ) )			\
{																											\
	as::Call( GetObject().Get(), pFunction, ##__VA_ARGS__ );												\
}																											\
else																										\
{																											\
	baseMethodName( __VA_ARGS__ );																			\
}

/**
*	Calls a method.
*	@param methodName Script or C++ method name to call.
*	@param pszParams Script method parameters.
*	@param ... Arguments to pass.
*/
#define CALL_EXTEND_FUNC( methodName, pszParams, ... )									\
CALL_EXTEND_FUNC_DIFFFUNC( methodName, BaseClass::methodName, pszParams, ##__VA_ARGS__ )

/** @} */

#endif //ANGELSCRIPT_UTIL_CASEXTENDADAPTER_H