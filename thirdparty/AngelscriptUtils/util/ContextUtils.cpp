#include <cassert>
#include <cstdarg>

#include "AngelscriptUtils/util/ASLogging.h"
#include "AngelscriptUtils/util/ASUtil.h"

#include "AngelscriptUtils/wrapper/CASContext.h"
#include "AngelscriptUtils/wrapper/ASCallable.h"

#include "ContextUtils.h"

namespace ctx
{
bool SetArguments( const asIScriptFunction& targetFunc, asIScriptContext& context, const CASArguments& arguments )
{
	const asUINT uiArgCount = targetFunc.GetParamCount();

	if( uiArgCount != arguments.GetArgumentCount() )
	{
		const auto szFunctionName = as::FormatFunctionName( targetFunc );

		as::log->critical( "ctx::SetArguments: argument count for function '{}' is incorrect: expected {}, got {}!",
						   szFunctionName, uiArgCount, arguments.GetArgumentCount() );

		return false;
	}

	bool bSuccess = true;

	asIScriptEngine& engine = *context.GetEngine();

	const auto& args = arguments.GetArgumentList();

	for( asUINT uiIndex = 0; uiIndex < uiArgCount && bSuccess; ++uiIndex )
	{
		bSuccess = SetContextArgument( engine, targetFunc, context, uiIndex, args[ uiIndex ] );
	}

	return bSuccess;
}

bool SetArguments( const asIScriptFunction& targetFunc, asIScriptContext& context, va_list list )
{
	assert( list );

	if( !list )
		return false;

	const asUINT uiArgCount = targetFunc.GetParamCount();

	bool bSuccess = true;

	asIScriptEngine& engine = *targetFunc.GetEngine();

	VAList vaList;
	
	//GCC fails to copy the list if it's done any other way.
	va_copy( vaList.list, list );

	for( asUINT uiIndex = 0; uiIndex < uiArgCount && bSuccess; ++uiIndex )
	{
		bSuccess = SetContextArgument( engine, targetFunc, context, uiIndex, vaList );
	}

	return bSuccess;
}

bool SetArgument( asIScriptEngine& engine, void* pData, int iTypeId, CASArgument& arg )
{
	bool bSuccess = true;

	bool bWasPrimitive = false;

	bSuccess = SetPrimitiveArgument( pData, iTypeId, arg, bWasPrimitive );

	if( bSuccess && !bWasPrimitive )
		bSuccess = SetObjectArgument( engine, pData, iTypeId, arg );

	return bSuccess;
}

bool SetContextArgument( asIScriptEngine& engine, const asIScriptFunction& targetFunc, asIScriptContext& context,
						 const asUINT uiIndex, const CASArgument& arg )
{
	return SetContextArgument( engine, targetFunc, context, uiIndex, arg.GetTypeId(), arg.GetArgumentValue(), false );
}

bool SetContextArgument( asIScriptEngine& engine, const asIScriptFunction& targetFunc, asIScriptContext& context, const asUINT uiIndex, VAList& list )
{
	int iTypeId;
	asDWORD uiFlags;

	if( targetFunc.GetParam( uiIndex, &iTypeId, &uiFlags ) < 0 )
	{
		as::log->critical( "ctx::SetContextArgument: An error occurred while getting function parameter information, aborting!" );
		return false;
	}

	bool bSuccess = true;

	ArgumentValue value;

	if( ( bSuccess = GetArgumentFromVarargs( value, iTypeId, uiFlags, list ) ) != false )
	{
		//Remove the handle flag from the typeid.
		//Input should never be dereferenced
		bSuccess = SetContextArgument( engine, targetFunc, context, uiIndex, iTypeId & ~asTYPEID_OBJHANDLE, value, true );
	}

	return bSuccess;
}

bool SetContextArgument( asIScriptEngine& engine, const asIScriptFunction& targetFunc, asIScriptContext& context,
										const asUINT uiIndex, int iSourceTypeId, const ArgumentValue& value, bool bAllowPrimitiveReferences )
{
	int iTypeId;
	asDWORD uiFlags;

	if( targetFunc.GetParam( uiIndex, &iTypeId, &uiFlags ) < 0 )
	{
		as::log->critical( "ctx::SetContextArgument: An error occurred while getting function parameter information, aborting!" );
		return false;
	}

	bool bSuccess = true;

	//It's an object handle (ref type)
	if( iTypeId & asTYPEID_OBJHANDLE )
	{
		void* pSourceObj = ( iSourceTypeId & asTYPEID_OBJHANDLE ) ? *reinterpret_cast<void**>( value.pValue ) : value.pValue;

		if( ( iSourceTypeId & asTYPEID_MASK_OBJECT ) )
		{
			asITypeInfo* pType = engine.GetTypeInfoById( iTypeId );

			if( pType )
			{
				bool bCanSet = true;

				//Types are functions
				if( pType->GetFlags() & asOBJ_FUNCDEF )
				{
					asIScriptFunction* pSourceFunc = reinterpret_cast<asIScriptFunction*>( pSourceObj );

					//Functions are incompatible, can't set
					if( !pSourceFunc->IsCompatibleWithTypeId( iTypeId ) )
					{
						as::log->critical( "ctx::SetContextArgument: Could not set argument {}, argument function signatures are different, aborting!", uiIndex );
						bCanSet = false;
					}
				}
				else
				{
					void* pObject = nullptr;

					if( engine.RefCastObject( pSourceObj, engine.GetTypeInfoById( iSourceTypeId ), pType, &pObject ) >= 0 )
						engine.ReleaseScriptObject( pObject, pType );
					else
					{
						as::log->critical( "ctx::SetContextArgument: Source argument is incompatible with target, aborting!" );
						bCanSet = false;
					}
				}

				if( bCanSet )
					bSuccess = context.SetArgObject( uiIndex, pSourceObj ) >= 0;
				else
					bSuccess = false;
			}
			else
			{
				as::log->critical( "ctx::SetContextArgument: Could not get object type for argument {}, aborting!", uiIndex );
				bSuccess = false;
			}
		}
		else
		{
			as::log->critical( "ctx::SetContextArgument: Source argument is incompatible with target, aborting!\n" );
			bSuccess = false;
		}
	}
	else if( iTypeId & asTYPEID_MASK_OBJECT ) //Object value (ref or value type)
	{
		bool bIsCompatible = false;

		if( iTypeId == iSourceTypeId )
			bIsCompatible = true;

		if( bIsCompatible )
			bSuccess = context.SetArgObject( uiIndex, value.pValue ) >= 0;
	}
	else //Primitive type (including enum)
	{
		//Primitive type taken by reference
		if( ( as::IsPrimitive( iTypeId ) ) && ( uiFlags & ( asTM_INREF | asTM_OUTREF ) ) )
		{
			const void* pAddress = bAllowPrimitiveReferences ? value.pValue : &value.qword;
			bSuccess = context.SetArgAddress( uiIndex, const_cast<void*>( pAddress ) ) >= 0;
		}
		else
		{
			bool bWasPrimitive = true;

			//Needs a little conversion magic
			asINT64 uiValue;
			double dValue;

			//Type was a primitive type or enum
			bSuccess = ConvertInputArgToLargest( iSourceTypeId, value, uiValue, dValue );

			if( bSuccess )
			{
				switch( iTypeId )
				{
				case asTYPEID_VOID:
					{
						//Impossible
						as::log->critical( "ctx::SetContextArgument: the impossible happened: a void argument, aborting!" );
						bSuccess = false;
						break;
					}

				case asTYPEID_BOOL:
				case asTYPEID_INT8:
				case asTYPEID_UINT8:	bSuccess = context.SetArgByte( uiIndex, static_cast<asBYTE>( uiValue ) ) >= 0; break;
				case asTYPEID_INT16:
				case asTYPEID_UINT16:	bSuccess = context.SetArgWord( uiIndex, static_cast<asWORD>( uiValue ) ) >= 0; break;
				case asTYPEID_INT32:
				case asTYPEID_UINT32:	bSuccess = context.SetArgDWord( uiIndex, static_cast<asDWORD>( uiValue ) ) >= 0; break;
				case asTYPEID_INT64:
				case asTYPEID_UINT64:	bSuccess = context.SetArgQWord( uiIndex, uiValue ) >= 0; break;

				case asTYPEID_FLOAT:	bSuccess = context.SetArgFloat( uiIndex, static_cast<float>( dValue ) ) >= 0; break;
				case asTYPEID_DOUBLE:	bSuccess = context.SetArgDouble( uiIndex, dValue ) >= 0; break;

				default:

					bWasPrimitive = false;

					//It's an enum
					if( as::IsEnum( iTypeId & asTYPEID_MASK_SEQNBR ) )
					{
						if( uiFlags & ( asTM_INREF | asTM_OUTREF ) )
						{
							const void* pAddress = bAllowPrimitiveReferences ? value.pValue : &value.qword;
							bSuccess = context.SetArgAddress( uiIndex, const_cast<void*>( pAddress ) ) >= 0;
						}
						else
							bSuccess = context.SetArgDWord( uiIndex, value.dword ) >= 0;
					}
					else
					{
						as::log->critical( "ctx::SetContextArgument: Attempted to set parameter of unknown type, aborting!" );
						bSuccess = false;
					}
					break;
				}

				if( bWasPrimitive )
				{
					if( iTypeId != iSourceTypeId )
					{
						as::log->critical( "ctx::SetContextArgument: Attempted to set primitive value of type '{}' to value of type '{}', aborting!",
										 as::PrimitiveTypeIdToString( iTypeId ), as::PrimitiveTypeIdToString( iSourceTypeId ) );
						bSuccess = false;
					}
				}
			}
		}
	}

	return bSuccess;
}

bool GetArgumentFromVarargs( ArgumentValue& value, int iTypeId, asDWORD uiTMFlags, VAList& list, asDWORD* puiObjFlags, ArgType::ArgType* pOutArgType )
{
	if( pOutArgType && !puiObjFlags )
		return false;

	bool bSuccess = true;

	if( pOutArgType )
		*pOutArgType = ArgType::NONE;

	//Its an object (ref or value type), or handle to ref type
	//Handles reference parameters automatically, source pointer must be correct type
	if( iTypeId & ( asTYPEID_OBJHANDLE | asTYPEID_MASK_OBJECT ) )
	{
		value.pValue = va_arg( list.list, void* );

		//Set type if requested
		if( pOutArgType )
		{
			if( ( *puiObjFlags ) & asOBJ_REF )
				*pOutArgType = ArgType::REF;
			else if( ( *puiObjFlags ) & asOBJ_VALUE )
				*pOutArgType = ArgType::VALUE;
			else
			{
				as::log->critical( "ctx::GetArgumentFromVarargs: Unknown object type, cannot convert!" );
				bSuccess = false;
			}
		}
	}
	else //Primitive type (including enum)
	{
		bool bWasPrimitive = true;

		//In/Out reference
		if( as::IsPrimitive( iTypeId ) && ( uiTMFlags & ( asTM_INREF | asTM_OUTREF ) ) )
		{
			value.pValue = va_arg( list.list, void* );
		}
		else
		{
			switch( iTypeId )
			{
			case asTYPEID_VOID:
				{
					//Impossible
					as::log->critical( "ctx::SetContextArgument: The impossible happened: a void argument" );
					bSuccess = false;
					break;
				}

			case asTYPEID_BOOL:
			case asTYPEID_INT8:
			case asTYPEID_UINT8:	value.byte = static_cast<asBYTE>( va_arg( list.list, long ) ); break; //Promoted to int
			case asTYPEID_INT16:
			case asTYPEID_UINT16:	value.word = static_cast<asWORD>( va_arg( list.list, long ) ); break; //Promoted to int
			case asTYPEID_INT32:
			case asTYPEID_UINT32:	value.dword = static_cast<asDWORD>( va_arg( list.list, long ) ); break;
			case asTYPEID_INT64:
			case asTYPEID_UINT64:	value.qword = va_arg( list.list, long long ); break;

			case asTYPEID_FLOAT:	value.flValue = static_cast<float>( va_arg( list.list, double ) ); break; //Promoted to double
			case asTYPEID_DOUBLE:	value.dValue = va_arg( list.list, double ); break;

			default:

				bWasPrimitive = false;

				//It's an enum
				if( as::IsEnum( iTypeId ) )
				{
					if( pOutArgType )
						*pOutArgType = ArgType::ENUM;

					value.dword = va_arg( list.list, long );
				}
				else
				{
					as::log->critical( "ctx::SetContextArgument: Attempted to set parameter of unknown type, aborting!" );
					bSuccess = false;
				}
				break;
			}
		}

		if( pOutArgType && bWasPrimitive )
			*pOutArgType = ArgType::PRIMITIVE;
	}

	return bSuccess;
}

bool ConvertInputArgToLargest( const CASArgument& arg, asINT64& uiValue, double& flValue )
{
	return ConvertInputArgToLargest( arg.GetTypeId(), arg.GetArgumentValue(), uiValue, flValue );
}

bool ConvertInputArgToLargest( int iTypeId, const ArgumentValue& value, asINT64& uiValue, double& flValue )
{
	bool bSuccess = true;

	int iType = asTYPEID_UINT64;

	switch( iTypeId )
	{
	case asTYPEID_VOID:
		{
			//Impossible
			as::log->critical( "ctx::ConvertInputArgToLargest: The impossible happened: a void argument, aborting!" );
			bSuccess = false;
			break;
		}

	case asTYPEID_BOOL:		uiValue = value.byte; break;

	case asTYPEID_INT8:
	case asTYPEID_UINT8:	uiValue = value.byte; break; //Promoted to int
	case asTYPEID_INT16:
	case asTYPEID_UINT16:	uiValue = value.word; break; //Promoted to int
	case asTYPEID_INT32:
	case asTYPEID_UINT32:	uiValue = value.dword; break;
	case asTYPEID_INT64:
	case asTYPEID_UINT64:	uiValue = value.qword; break;

	case asTYPEID_FLOAT:	flValue = value.flValue; iType = asTYPEID_DOUBLE; break; //Promoted to double
	case asTYPEID_DOUBLE:	flValue = value.dValue; iType = asTYPEID_DOUBLE; break;

	default:

		//It's an enum
		if( as::IsEnum( iTypeId ) )
		{
			uiValue = value.dword;
		}
		else
		{
			//Type isn't a primitive type or enum
			bSuccess = false;
		}
		break;
	}

	if( bSuccess )
	{
		//Both values must contain the same value
		if( iType == asTYPEID_UINT64 )
			flValue = static_cast<double>( uiValue );
		else
			uiValue = static_cast<asINT64>( flValue );
	}

	return bSuccess;
}

ArgType::ArgType ArgumentTypeFromTypeId( const int iTypeId, const asDWORD uiObjFlags )
{
	ArgType::ArgType argType = ArgType::NONE;

	//Its an object (ref or value type), or handle to ref type
	if( iTypeId & ( asTYPEID_OBJHANDLE | asTYPEID_MASK_OBJECT ) )
	{
		if( uiObjFlags & asOBJ_REF )
			argType = ArgType::REF;
		else if( uiObjFlags & asOBJ_VALUE )
			argType = ArgType::VALUE;
	}
	else if( as::IsPrimitive( iTypeId ) )
	{
		argType = ArgType::PRIMITIVE;
	}
	else if( as::IsEnum( iTypeId ) )
	{
		argType = ArgType::ENUM;
	}

	return argType;
}

bool ConvertEnumToPrimitive( const CASArgument& arg, const int iTypeId, ArgumentValue& outValue )
{
	//Enums can convert to integral types
	//Enum have their type id set to int32, so handle both
	if( !as::IsEnum( arg.GetTypeId() ) && arg.GetTypeId() != asTYPEID_INT32 )
	{
		return false;
	}

	//Convert the enum value to the appropriate type
	if( asTYPEID_INT8 <= iTypeId && iTypeId <= asTYPEID_DOUBLE )
	{
		switch( iTypeId )
		{
		case asTYPEID_INT8:
		case asTYPEID_UINT8:
			{
				if( arg.GetArgumentValue().dword > UINT8_MAX )
				{
					as::log->critical( "Truncating enum value to 1 byte!" );
				}

				outValue.byte = static_cast<asBYTE>( arg.GetArgumentValue().dword );
				break;
			}

		case asTYPEID_INT16:
		case asTYPEID_UINT16:
			{
				if( arg.GetArgumentValue().dword > UINT16_MAX )
				{
					as::log->critical( "Truncating enum value to 2 bytes!" );
				}

				outValue.word = static_cast<asWORD>( arg.GetArgumentValue().dword );
				break;
			}

		case asTYPEID_INT32:
		case asTYPEID_UINT32:	outValue.dword = arg.GetArgumentValue().dword; break;

		case asTYPEID_INT64:
		case asTYPEID_UINT64:	outValue.qword = arg.GetArgumentValue().dword; break;

		case asTYPEID_FLOAT:	outValue.flValue = static_cast<float>( arg.GetArgumentValue().dword ); break;
		case asTYPEID_DOUBLE:	outValue.dValue = arg.GetArgumentValue().dword; break;
		}

		return true;
	}

	return false;
}

bool ConvertPrimitiveToEnum( const CASArgument& arg, ArgumentValue& outValue )
{
	if( !as::IsPrimitive( arg.GetTypeId() ) )
	{
		return false;
	}

	switch( arg.GetTypeId() )
	{
	case asTYPEID_INT8:
	case asTYPEID_UINT8:	outValue.dword = arg.GetArgumentValue().byte; break;

	case asTYPEID_INT16:
	case asTYPEID_UINT16:	outValue.dword = arg.GetArgumentValue().word; break;

	case asTYPEID_INT32:
	case asTYPEID_UINT32:	outValue.dword = arg.GetArgumentValue().dword; break;

	case asTYPEID_INT64:
	case asTYPEID_UINT64:
		{
			if( arg.GetArgumentValue().qword > UINT32_MAX )
			{
				as::log->critical( "Truncating 8 byte value to 4 bytes!" );
			}

			outValue.dword = static_cast<asDWORD>( arg.GetArgumentValue().qword );
			break;
		}

	case asTYPEID_FLOAT:	outValue.dword = static_cast<asDWORD>( arg.GetArgumentValue().flValue ); break;

	case asTYPEID_DOUBLE:
		{
			//TODO: is this correct?
			if( arg.GetArgumentValue().dValue > UINT32_MAX )
			{
				as::log->critical( "Truncating 8 byte value to 4 bytes!" );
			}

			outValue.dword = static_cast<asDWORD>( arg.GetArgumentValue().dValue );
			break;
		}
	}

	return true;
}

bool SetPrimitiveArgument( void* pData, int iTypeId, CASArgument& arg, bool& bOutWasPrimitive )
{
	bOutWasPrimitive = false;

	if( !pData )
		return false;

	ArgumentValue value;

	bool bSuccess = SetPrimitiveArgument( pData, iTypeId, value, bOutWasPrimitive );

	if( bSuccess && bOutWasPrimitive )
	{
		bSuccess = arg.Set( iTypeId, ArgType::PRIMITIVE, value );

		if( !bSuccess )
			bOutWasPrimitive = false;
	}

	return bSuccess;
}

bool SetPrimitiveArgument( void* pData, int iTypeId, ArgumentValue& value, bool& bOutWasPrimitive )
{
	if( !pData )
	{
		bOutWasPrimitive = false;
		return false;
	}

	bOutWasPrimitive = true;

	bool bSuccess = true;

	//We handle signed and unsigned the same way; they occupy the same amount of memory, and will be cast when the destination function receives them.
	switch( iTypeId )
	{
	case asTYPEID_VOID:
		{
			//What the hell?! how did somebody pass a void argument?
			bSuccess = false;
			break;
		}

	case asTYPEID_BOOL:		value.byte = *reinterpret_cast<bool*>( pData ); break;

	case asTYPEID_INT8:
	case asTYPEID_UINT8:	value.byte = *reinterpret_cast<asBYTE*>( pData ); break;

	case asTYPEID_INT16:
	case asTYPEID_UINT16:	value.word = *reinterpret_cast<asWORD*>( pData ); break;

	case asTYPEID_INT32:
	case asTYPEID_UINT32:	value.dword = *reinterpret_cast<asDWORD*>( pData ); break;

	case asTYPEID_INT64:
	case asTYPEID_UINT64:	value.qword = *reinterpret_cast<asQWORD*>( pData ); break;

	case asTYPEID_FLOAT:	value.flValue = *reinterpret_cast<float*>( pData ); break;
	case asTYPEID_DOUBLE:	value.dValue = *reinterpret_cast<double*>( pData ); break;

	default: //Not a primitive type.
		{
			bOutWasPrimitive = false;
			break;
		}
	}

	return bSuccess;
}

bool SetObjectArgument( asIScriptEngine& engine, void* pObject, int iTypeId, CASArgument& arg )
{
	bool bSuccess = true;

	ArgType::ArgType argType = ArgType::NONE;
	ArgumentValue value;

	if( asITypeInfo* pType = engine.GetTypeInfoById( iTypeId ) )
	{
		const asDWORD uiFlags = pType->GetFlags();

		//Is it a value type?
		if( uiFlags & asOBJ_VALUE )
		{
			argType = ArgType::VALUE;
			value.pValue = engine.CreateScriptObjectCopy( pObject, pType );
		}
		else if( uiFlags & asOBJ_REF ) //is it a reference type?
		{
			argType = ArgType::REF;

			//All types are interpreted as pointers
			value.pValue = pObject;

			if( iTypeId & asTYPEID_OBJHANDLE )
			{
				//Handles are pointer to pointer, dereference
				//If this is a handle to a function (not funcdef), will also be correctly dereferenced
				value.pValue = *reinterpret_cast<void**>( value.pValue );

				//Remove flag from typeid
				iTypeId &= ~asTYPEID_OBJHANDLE;
			}

			//Functions require a little more work
			if( uiFlags & asOBJ_FUNCDEF )
			{
				//Note: you can pass in a function pointer directly, without casting to a funcdef.
				//However, since a function name can't be used as a parameter, no functions could ever exist with that format.
				//This is impossible to detect and can't be guarded against.
				//So we allow it, but warn against it in the manual.
				asIScriptFunction* pFunc = reinterpret_cast<asIScriptFunction*>( value.pValue );

				//Delegates have to be handled as pointer to pointer
				if( pFunc->GetFuncType() == asFUNC_DELEGATE )
					value.pValue = *reinterpret_cast<void**>( value.pValue );
			}

			engine.AddRefScriptObject( value.pValue, pType );
		}
		else
		{
			//I don't know what this is.
			bSuccess = false;
			as::log->critical( "ctx::SetObjectArgument: unknown type '{}', aborting!", pType->GetName() );
		}
	}
	else
	{
		//Check if it's an enum.
		if( as::IsEnum( iTypeId ) )
		{
			argType = ArgType::ENUM;
			value.dword = *reinterpret_cast<asDWORD*>( pObject );
		}
		else //I don't know what this is.
		{
			bSuccess = false;
			as::log->critical( "ctx::SetObjectArgument: unknown type '{}'({}), aborting!", engine.GetTypeDeclaration( iTypeId, true ), iTypeId );
		}
	}

	if( bSuccess )
		arg.Set( iTypeId, argType, value );

	return bSuccess;
}

bool GetReturnValue( asIScriptContext& context, const asIScriptFunction& func, CASArgument& retVal, asDWORD* uiOutFlags )
{
	asDWORD uiFlags;
	const int iTypeId = func.GetReturnTypeId( &uiFlags );

	if( uiOutFlags )
		*uiOutFlags = uiFlags;

	return GetReturnValue( context, iTypeId, uiFlags, retVal );
}

bool GetReturnValue( asIScriptContext& context, int iTypeId, asDWORD uiFlags, CASArgument& retVal )
{
	bool bSuccess = true;

	ArgType::ArgType argType = ArgType::NONE;
	ArgumentValue value;

	auto pEngine = context.GetEngine();

	//Object handle type
	if( iTypeId & asTYPEID_OBJHANDLE )
	{
		argType = ArgType::REF;
		value.pValue = context.GetReturnObject();
	}
	else if( iTypeId & asTYPEID_MASK_OBJECT ) //Object type (ref or value type)
	{
		asITypeInfo* pType = pEngine->GetTypeInfoById( iTypeId );

		if( pType )
		{
			const asDWORD uiObjFlags = pType->GetFlags();

			value.pValue = context.GetReturnObject();

			if( uiObjFlags & asOBJ_REF )
			{
				argType = ArgType::REF;
			}
			else if( uiObjFlags & asOBJ_VALUE )
			{
				argType = ArgType::VALUE;

				//If the object is returned by value, a copy needs to be made
				//This is handled by CASArgument in the Set method below.
			}
			else
			{
				as::log->critical( "ctx::GetReturnValue: unknown object type, cannot convert!" );
				bSuccess = false;
			}
		}
		else
		{
			as::log->critical( "ctx::GetReturnValue: failed to get object type!" );
			bSuccess = false;
		}
	}
	else if( iTypeId == asTYPEID_VOID )
	{
		argType = ArgType::VOID;
	}
	else //Primitive type (including enum)
	{
		bool bWasPrimitive = true;

		//It's a reference
		if( uiFlags & asTM_INOUTREF )
		{
			if( !SetPrimitiveArgument( context.GetReturnAddress(), iTypeId, value, bWasPrimitive ) )
			{
				as::log->critical( "ctx::GetReturnValue: Something went wrong while trying to convert the return value to a usable type!" );
				bSuccess = false;
			}
		}
		else //It's returned by value
		{
			switch( iTypeId )
			{
			case asTYPEID_BOOL:
			case asTYPEID_INT8:
			case asTYPEID_UINT8:	value.byte = context.GetReturnByte(); break;

			case asTYPEID_INT16:
			case asTYPEID_UINT16:	value.word = context.GetReturnWord(); break;

			case asTYPEID_INT32:
			case asTYPEID_UINT32:	value.dword = context.GetReturnDWord(); break;

			case asTYPEID_INT64:
			case asTYPEID_UINT64:	value.qword = context.GetReturnQWord(); break;

			case asTYPEID_FLOAT:	value.flValue = context.GetReturnFloat(); break;
			case asTYPEID_DOUBLE:	value.dValue = context.GetReturnDouble(); break;

			default: //Not a primitive type.
				{
					bWasPrimitive = false;
					break;
				}
			}
		}

		if( bSuccess )
		{
			if( bWasPrimitive )
				argType = ArgType::PRIMITIVE;
			else
			{
				if( as::IsEnum( iTypeId & asTYPEID_MASK_SEQNBR ) )
				{
					argType = ArgType::ENUM;
					value.dword = context.GetReturnDWord();
				}
				else
				{
					as::log->critical( "ctx::GetReturnValue: Unknown primitive return type, cannot convert!" );
					bSuccess = false;
				}
			}
		}
	}

	if( !bSuccess )
		return false;

	//Let the argument class handle copying
	return retVal.Set( *pEngine, iTypeId, argType, value, true );
}

bool GetReturnValue( asIScriptContext& context, int iTypeId, asDWORD uiFlags, void* pReturnValue )
{
	assert( pReturnValue );

	if( !pReturnValue )
		return false;

	bool bSuccess = true;

	//Object handle type
	if( iTypeId & asTYPEID_OBJHANDLE )
	{
		*reinterpret_cast<void**>( pReturnValue ) = context.GetReturnObject();
	}
	else if( iTypeId & asTYPEID_MASK_OBJECT ) //Object type (ref or value type)
	{
		asIScriptEngine* pEngine = context.GetEngine();
		asITypeInfo* pType = pEngine->GetTypeInfoById( iTypeId );

		if( pType )
		{
			void* pObject = context.GetReturnObject();

			asIScriptFunction* pFunc;

			asUINT uiIndex;

			const asUINT uiCount = pType->GetMethodCount();

			//Find an assignment operator
			for( uiIndex = 0; uiIndex < uiCount; ++uiIndex )
			{
				pFunc = pType->GetMethodByIndex( uiIndex );

				if( pFunc->GetParamCount() == 1 )
				{
					int iFuncTypeId;
					asDWORD uiFuncFlags;

					if( pFunc->GetParam( 0, &iFuncTypeId, &uiFuncFlags ) >= 0 )
					{
						if( iFuncTypeId == pType->GetTypeId() && uiFuncFlags & asTM_INREF )
						{
							context.PushState();

							//Call the assignment operator on the object
							CASContext ctx( context );

							CASMethod func( *pFunc, ctx, pReturnValue );

							func( CallFlag::NONE, pObject );

							context.PopState();

							break;
						}
					}
				}
			}

			if( uiIndex == uiCount )
			{
				bSuccess = false;
				as::log->critical( "ctx::GetReturnValue: No assignment operator found for type '{}::{}', cannot convert!",
								 pType->GetNamespace(), pType->GetName() );
			}
		}
		else
		{
			as::log->critical( "ctx::GetReturnValue: Failed to get object type!" );
			bSuccess = false;
		}
	}
	else if( iTypeId == asTYPEID_VOID )
	{
		as::log->critical( "ctx::GetReturnValue: Type is null!" );
	}
	else //Primitive type (including enum)
	{
		bool bWasPrimitive = true;

		//It's a reference
		if( uiFlags & asTM_INOUTREF )
		{
			*reinterpret_cast<void**>( pReturnValue ) = context.GetReturnAddress();
		}
		else //It's returned by value
		{
			switch( iTypeId )
			{
			case asTYPEID_BOOL:		*reinterpret_cast<bool*>( pReturnValue ) = context.GetReturnByte() != 0; break;
			case asTYPEID_INT8:
			case asTYPEID_UINT8:	*reinterpret_cast<asBYTE*>( pReturnValue ) = context.GetReturnByte(); break;

			case asTYPEID_INT16:
			case asTYPEID_UINT16:	*reinterpret_cast<asWORD*>( pReturnValue ) = context.GetReturnWord(); break;

			case asTYPEID_INT32:
			case asTYPEID_UINT32:	*reinterpret_cast<asDWORD*>( pReturnValue ) = context.GetReturnDWord(); break;

			case asTYPEID_INT64:
			case asTYPEID_UINT64:	*reinterpret_cast<asQWORD*>( pReturnValue ) = context.GetReturnQWord(); break;

			case asTYPEID_FLOAT:	*reinterpret_cast<float*>( pReturnValue ) = context.GetReturnFloat(); break;
			case asTYPEID_DOUBLE:	*reinterpret_cast<double*>( pReturnValue ) = context.GetReturnDouble(); break;

			default: //Not a primitive type.
				{
					bWasPrimitive = false;
					break;
				}
			}
		}

		if( bSuccess )
		{
			if( !bWasPrimitive )
			{
				if( as::IsEnum( iTypeId & asTYPEID_MASK_SEQNBR ) )
				{
					*reinterpret_cast<asDWORD*>( pReturnValue ) = context.GetReturnDWord();
				}
				else
				{
					as::log->critical( "ctx::GetReturnValue: Unknown primitive return type, cannot convert!" );
					bSuccess = false;
				}
			}
		}
	}

	return bSuccess;
}
}
