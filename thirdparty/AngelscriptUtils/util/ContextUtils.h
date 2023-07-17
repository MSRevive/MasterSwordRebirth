#ifndef UTIL_CONTEXTUTILS_H
#define UTIL_CONTEXTUTILS_H

#include <angelscript.h>

#include "AngelscriptUtils/wrapper/CASArguments.h"

/**
*	@addtogroup ASContext
*
*	@{
*/

namespace ctx
{
/**
*	Wrapper type used to pass around va_list. On some platforms va_list isn't a simple pointer, so this solves type issues.
*/
struct VAList final
{
	va_list list;
};

/**
*	Sets arguments for a function call.
*	@param targetFunc Target function.
*	@param context Context.
*	@param arguments List of arguments.
*	@return true on success, false otherwise.
*/
bool SetArguments( const asIScriptFunction& targetFunc, asIScriptContext& context, const CASArguments& arguments );

/**
*	Sets arguments for a function call.
*	@param targetFunc Target function.
*	@param context Context.
*	@param list Pointer to the arguments.
*	@return true on success, false otherwise.
*/
bool SetArguments( const asIScriptFunction& targetFunc, asIScriptContext& context, va_list list );

/**
*	Sets an argument on arg. Determines whether it's a primitive or object argument.
*	@param engine Script engine.
*	@param pData Data to copy.
*	@param iTypeId Type Id.
*	@param arg Argument to set.
*	@return true on success, false otherwise.
*/
bool SetArgument( asIScriptEngine& engine, void* pData, int iTypeId, CASArgument& arg );

/**
*	Sets an argument on the context, taking the argument from a CASArgument.
*	@param engine Script engine.
*	@param targetFunc Target function.
*	@param context Context.
*	@param uiIndex Argument index.
*	@param arg The argument.
*	@return true on success, false otherwise.
*/
bool SetContextArgument( asIScriptEngine& engine, const asIScriptFunction& targetFunc, asIScriptContext& context, const asUINT uiIndex, const CASArgument& arg );

/**
*	Sets an argument on the context, taking the argument from varargs.
*	@param engine Script engine.
*	@param targetFunc Target function.
*	@param context Context.
*	@param uiIndex Argument index.
*	@param list Pointer to the argument.
*	@return true on success, false otherwise.
*/
bool SetContextArgument( asIScriptEngine& engine, const asIScriptFunction& targetFunc, asIScriptContext& context, const asUINT uiIndex, VAList& list );

/**
*	Sets an argument on the context, taking the argument from an ArgumentValue_t.
*	@param engine Script engine.
*	@param targetFunc Target function.
*	@param context Context.
*	@param uiIndex Argument index.
*	@param iSourceTypeId Type id of the argument.
*	@param value Value to set.
*	@param bAllowPrimitiveReferences Indicates whether primitive type arguments taken by reference use pValue or &qword.
*	@return true on success, false otherwise.
*/
bool SetContextArgument( asIScriptEngine& engine, const asIScriptFunction& targetFunc, asIScriptContext& context,
						 const asUINT uiIndex, int iSourceTypeId, const ArgumentValue& value, bool bAllowPrimitiveReferences );

/**
*	Gets the argument of type iTypeId from the stack, and stores it in value
*	@param value Value to store the result in.
*	@param iTypeId Type Id of the value.
*	@param uiTMFlags A combination of asETypeModifiers, or asTM_NONE.
*	@param list Pointer to the argument.
*	@param[ in ] puiObjFlags Optional. Object flags. Must be set if pOutArgType is set.
*	@param[ out ] pOutArgType Optional. Argument type.
*	@return true on success, false otherwise.
*/
bool GetArgumentFromVarargs( ArgumentValue& value, int iTypeId, asDWORD uiTMFlags, VAList& list, asDWORD* puiObjFlags = nullptr, ArgType::ArgType* pOutArgType = nullptr );

/**
*	Converts the input argument to either an asQWORD or double.
*	iType is either asTYPEID_UINT64 or asTYPEID_DOUBLE.
*	@param arg Argument to convert.
*	@param uiValue The converted value, as an integer.
*	@param flValue The converted value, as a float.
*	@return true on success, false if conversion failed.
*/
bool ConvertInputArgToLargest( const CASArgument& arg, asINT64& uiValue, double& flValue );

/**
*	@see ConvertInputArgToLargest( const CASArgument* const pArg, asINT64& uiValue, double& flValue )
*/
bool ConvertInputArgToLargest( int iTypeId, const ArgumentValue& value, asINT64& uiValue, double& flValue );

/**
*	Gets the argument type from a type id and object flags.
*	@param iTypeId Type id.
*	@param uiObjFlags Object flags.
*	@return Argument type.
*/
ArgType::ArgType ArgumentTypeFromTypeId( const int iTypeId, const asDWORD uiObjFlags );

/**
*	Converts an enum to a primitive type.
*	@param arg Argument to convert.
*	@param iTypeId Type Id.
*	@param outValue Converted value.
*	@return true on success, false otherwise.
*/
bool ConvertEnumToPrimitive( const CASArgument& arg, const int iTypeId, ArgumentValue& outValue );

/**
*	Converts a primitive type to an enum.
*	@param arg Argument to convert.
*	@param outValue Converted value.
*	@return true on success, false otherwise.
*/
bool ConvertPrimitiveToEnum( const CASArgument& arg, ArgumentValue& outValue );

/**
*	Tries to set the given data on the given value.
*	@param pData Data to copy.
*	@param iTypeId Type Id of the pData instance.
*	@param arg Argument to set.
*	@param[ out ] bOutWasPrimitive Whether the value was a primitive type or not.
*	@return true on success, false otherwise.
*/
bool SetPrimitiveArgument( void* pData, int iTypeId, CASArgument& arg, bool& bOutWasPrimitive );

/**
*	Tries to set the given data on the given value.
*	@param pData Data to copy.
*	@param iTypeId Type Id of the pData instance.
*	@param value Value to set.
*	@param[ out ] bOutWasPrimitive Whether the value was a primitive type or not.
*	@return true on success, false otherwise.
*/
bool SetPrimitiveArgument( void* pData, int iTypeId, ArgumentValue& value, bool& bOutWasPrimitive );

/**
*	Sets an object argument on the given argument.
*	@param engine Script engine.
*	@param pObject Object to copy or addref.
*	@param iTypeId Type Id.
*	@param arg Argument to set.
*	@return true on success, false otherwise.
*/
bool SetObjectArgument( asIScriptEngine& engine, void* pObject, int iTypeId, CASArgument& arg );

/**
*	Convenience method for when you don't want to get return type info yourself.
*	@param context Context.
*	@param func Function whose return type will be used.
*	@param retVal Return value.
*	@param[ out ] uiOutFlags Optional. Return type flags.
*	@return true on success, false otherwise.
*	@see GetReturnValue( asIScriptContext& context, int iTypeId, asDWORD uiFlags, CASArgument& retVal )
*/
bool GetReturnValue( asIScriptContext& context, const asIScriptFunction& func, CASArgument& retVal, asDWORD* uiOutFlags = nullptr );

/**
*	Gets the return value from the context according to the given return value type.
*	@param context Context.
*	@param iTypeId Type id of the return value.
*	@param uiFlags Type flags.
*	@param retVal Return value.
*	@return true on success, false otherwise.
*/
bool GetReturnValue( asIScriptContext& context, int iTypeId, asDWORD uiFlags, CASArgument& retVal );

/**
*	Gets the return value from the context according to the given return value type.
*	@param context Context.
*	@param iTypeId Type id of the return value.
*	@param uiFlags Type flags.
*	@param pReturnValue pointer to the variable that will receive the return value.
*	@return true on success, false otherwise.
*/
bool GetReturnValue( asIScriptContext& context, int iTypeId, asDWORD uiFlags, void* pReturnValue );
}

/** @} */

#endif //UTIL_CONTEXTUTILS_H