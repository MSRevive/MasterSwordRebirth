#ifndef UTIL_ASUTIL_H
#define UTIL_ASUTIL_H

#include <cassert>
#include <cctype>
#include <cstring>
#include <sstream>
#include <string>
#include <utility>

#include <angelscript.h>

#include "AngelscriptUtils/wrapper/CASArguments.h"

#include "AngelscriptUtils/util/ContextUtils.h"

#include "ASLogging.h"

#include "ASPlatform.h"

#include "StringUtils.h"

/**
*	@defgroup ASUtil Angelscript Utility Code
*
*	@{
*/

namespace as
{
/**
*	Returns a string representation of a primitive type id.
*	@param iTypeId Type Id.
*	@return String representation, or null if the type id is not a primitive type.
*/
const char* PrimitiveTypeIdToString( const int iTypeId );

/**
*	Returns whether the given type id is that of void.
*	Exists mostly for completeness.
*	@param iTypeId Type Id.
*	@return true if it is void, false otherwise.
*/
inline bool IsVoid( const int iTypeId )
{
	return asTYPEID_VOID == iTypeId;
}

/**
*	Returns whether a given type id is that of a primitive type.
*	@param iTypeId Type Id.
*	@return true if it is a primitive type, false otherwise.
*/
inline bool IsPrimitive( const int iTypeId )
{
	return asTYPEID_BOOL <= iTypeId && iTypeId <= asTYPEID_DOUBLE;
}

/**
*	Returns whether a given type id is that of an enum type.
*	@param iTypeId Type Id.
*	@return true if it is an enum type, false otherwise.
*/
inline bool IsEnum( const int iTypeId )
{
	return ( iTypeId > asTYPEID_DOUBLE && ( iTypeId & asTYPEID_MASK_OBJECT ) == 0 );
}

/**
*	Returns whether a given type id is that of an integer type.
*	@param iTypeId Type Id.
*	@return true if it is an integer type, false otherwise.
*/
inline bool IsInteger( const int iTypeId )
{
	return ( ( iTypeId >= asTYPEID_INT8 ) && ( iTypeId <= asTYPEID_UINT64 ) );
}

/**
*	Returns whether a given type id is that of a float type.
*	@param iTypeId Type Id.
*	@return true if it is a float type, false otherwise.
*/
inline bool IsFloat( const int iTypeId )
{
	return ( ( iTypeId >= asTYPEID_FLOAT ) && ( iTypeId <= asTYPEID_DOUBLE ) );
}

/**
*	Returns whether a given type id is that of a primitive type.
*	@param iTypeId Type Id.
*	@return true if it is a primitive type, false otherwise.
*/
inline bool IsObject( const int iTypeId )
{
	return ( iTypeId & asTYPEID_MASK_OBJECT ) != 0;
}

/**
*	Formats a function name and returns it.
*	The format for global functions is \<namespace>::\<name>.
*	The format for member functions is \<namespace>::\<classname>::\<name>.
*	@param function Function whose name should be formatted
*	@return Formatted function name
*/
inline std::string FormatFunctionName( const asIScriptFunction& function )
{
	const asIScriptFunction* pFunction = &function;

	{
		//If this is a delegate, get the original function.
		auto pDelegate = pFunction->GetDelegateFunction();

		if( pDelegate )
			pFunction = pDelegate;
	}

	auto pszNamespace = pFunction->GetNamespace();
	auto pszObjName = pFunction->GetObjectName();
	auto pszName = pFunction->GetName();

	const char szNSSep[] = "::";

	std::string szName;

	//Can copy up to a certain amount of the namespace name.
	if( pszNamespace && *pszNamespace )
	{
		szName += pszNamespace;
		szName += szNSSep;
	}

	if( pszObjName )
	{
		szName += pszObjName;
		szName += szNSSep;
	}

	szName += pszName;

	return szName;
}

/**
*	Releases a vararg argument.
*	@param engine Script engine.
*	@param pObject Object pointer.
*	@param iTypeId Type Id.
*/
void ReleaseVarArg( asIScriptEngine& engine, void* pObject, const int iTypeId );

/**
*	@param type Object type.
*	@return Whether the given type has a default constructor.
*/
bool HasDefaultConstructor( const asITypeInfo& type );

/**
*	Creates an instance of an object using its default constructor.
*	@param engine Script engine.
*	@param type Object type.
*	@return Object instance, or null if the object could not be instantiated.
*/
void* CreateObjectInstance( asIScriptEngine& engine, const asITypeInfo& type );

/**
*	Sets a value on an any object, which is an instance of a class that has the interface defined by CScriptAny
*	Angelscript will select the int64 or double overload when any integer or float type is passed, so this is necessary.
*	@param any Script engine.
*	@param pObject Object to set in the any
*	@param iTypeId Type Id
*/
template<typename ANY>
void SetAny( ANY& any, void* pObject, int iTypeId )
{
	//Have to convert primitive types of the form signed int and float smaller than their largest so that any.retrieve works correctly.
	asINT64 iDest = 0;
	double flDest = 0.0;

	//TODO: do unsigned types work properly with any?

	switch( iTypeId )
	{
	case asTYPEID_INT8:
	case asTYPEID_UINT8:	iDest = *static_cast<int8_t*>( pObject ); break;
	case asTYPEID_INT16:
	case asTYPEID_UINT16:	iDest = *static_cast<int16_t*>( pObject ); break;
	case asTYPEID_INT32:
	case asTYPEID_UINT32:	iDest = *static_cast<int32_t*>( pObject ); break;

	case asTYPEID_FLOAT:	flDest = *static_cast<float*>( pObject ); break;

	default: break;
	}

	switch( iTypeId )
	{
	case asTYPEID_INT8:
	case asTYPEID_UINT8:
	case asTYPEID_INT16:
	case asTYPEID_UINT16:
	case asTYPEID_INT32:
	case asTYPEID_UINT32:
		pObject = &iDest;
		iTypeId = asTYPEID_INT64;
		break;

	case asTYPEID_FLOAT:
		pObject = &flDest;
		iTypeId = asTYPEID_DOUBLE;
		break;

	default: break;
	}

	any.Store( pObject, iTypeId );
}

/**
*	Converts a primitive type to its string representation
*	@param pObject pointer to primitive value
*	@param iTypeId Type Id
*/
std::string PODToString( const void* pObject, const int iTypeId );

/**
*	Printf function used by script functions
*	@param pszFormat Format string
*	@param uiFirstParamIndex Index of the first parameter to use
*	@param arguments Generic arguments instance
*	@return Formatted string
*/
std::string SPrintf( const char* pszFormat, size_t uiFirstParamIndex, asIScriptGeneric& arguments );

/**
*	Used to handle the AddRef and Release behaviors on an Angelscript object when using template functions.
*	Specialize it for classes that use different method names.
*/
template<typename T>
class CASReferenceAdapter
{
public:
	typedef T Type_t;

public:

	static inline void AddRef( const T* const pObj )
	{
		pObj->AddRef();
	}

	static inline void Release( const T* const pObj )
	{
		pObj->Release();
	}

private:
	//Disable this stuff
	CASReferenceAdapter() = delete;
	CASReferenceAdapter( const CASReferenceAdapter& ) = delete;
	CASReferenceAdapter& operator=( const CASReferenceAdapter& ) = delete;
};

/*
*	Handles the setting of a pointer to an Angelscript reference counted object.
*	@param pPointer The pointer to assign the object to.
*	@param pObj Pointer to the object being assigned. Can be null.
*	@param bTransferOwnership If false, The destination pointer and the source pointer will both maintain a reference to the object.
*			Otherwise, the source pointer will no longer maintain a reference to it.
*	@tparam T Object type.
*	@tparam ADAPTER Adapter that implements static AddRef and Release functions for the given object type.
*
*	@return Pointer to the object.
*/
template<typename T, typename ADAPTER = CASReferenceAdapter<T>>
inline T* SetRefPointer( T*& pPointer, T* const pObj, const bool bTransferOwnership = false )
{
	if( pPointer )
	{
		ADAPTER::Release( pPointer );
	}

	pPointer = pObj;

	if( pPointer && !bTransferOwnership )
	{
		ADAPTER::AddRef( pPointer );
	}

	return pPointer;
}

/*
*	Handles the setting of a pointer to an Angelscript reference counted script object.
*	@param pDestThis The pointer to assign the object to.
*	@param pSourceThis Pointer to the object being assigned. Can be null.
*	@param bTransferOwnership If false, The destination pointer and the source pointer will both maintain a reference to the object.
*			Otherwise, the source pointer will no longer maintain a reference to it.
*
*	@return Pointer to the object.
*/
inline void* SetObjPointer( void*& pDestThis, void* const pSourceThis, asITypeInfo& typeInfo, const bool bTransferOwnership = false )
{
	auto pEngine = typeInfo.GetEngine();

	if( pDestThis )
	{
		pEngine->ReleaseScriptObject( pDestThis, &typeInfo );
	}

	if( pSourceThis )
	{
		pDestThis = pSourceThis;

		if( !bTransferOwnership )
			pEngine->AddRefScriptObject( pDestThis, &typeInfo );
	}
	else
	{
		pDestThis = nullptr;
	}

	return pDestThis;
}

/**
*	Registers a varargs function.
*	@param engine Script Engine.
*	@param regFunctor Functor that handles function/method registration.
*	@param pszReturnType Return type.
*	@param pszName Function name.
*	@param pszArguments Mandatory parameters.
*	@param uiMinArgs Minimum number of varargs.
*	@param uiMaxArgs Maximum number of varargs.
*	@param funcPtr Function pointer. Use asFUNCTION or asMETHOD.
*	@param pAuxiliary Optional. Auxiliary pointer.
*/
template<typename REGFUNCTOR>
void RegisterVarArgs( asIScriptEngine& engine, REGFUNCTOR regFunctor,
					  const char* const pszReturnType, const char* const pszName, const char* const pszArguments,
					  size_t uiMinArgs, size_t uiMaxArgs,
					  const asSFuncPtr& funcPtr, void* pAuxiliary )
{
	assert( pszReturnType );
	assert( pszName );
	assert( pszArguments );

	std::stringstream stream;

	//Varargs has a predictable format, so just format the beginning once and reset the write position to just before the ')' every loop.
	stream << pszReturnType << ' ' << pszName << '(' << pszArguments;

	//Figure out if there are any arguments before the varargs.
	bool bHasOtherArgs = std::string( pszArguments ).find_first_not_of( " \t" ) != std::string::npos;

	auto pos = stream.tellp();

	stream << ')';

	if( uiMaxArgs < uiMinArgs )
		std::swap( uiMinArgs, uiMaxArgs );

	const size_t uiNumVariations = ( uiMaxArgs - uiMinArgs ) + 1;

	size_t uiNumVarArgs = uiMinArgs;

	for( size_t uiVariant = 0; uiVariant < uiNumVariations; ++uiVariant )
	{
		stream.seekp( pos );

		if( uiNumVarArgs > 0 )
		{
			if( bHasOtherArgs || uiNumVarArgs > 1 )
			{
				stream << ", ";
			}

			stream << "?& in";
		}

		pos = stream.tellp();

		stream << ')';

		std::string str = stream.str();

#ifndef NDEBUG
		const auto result =
#endif
			regFunctor( engine, str.c_str(), funcPtr, asCALL_GENERIC, pAuxiliary );

		assert( result >= 0 );

		++uiNumVarArgs;
	}
}

/**
*	Functor that can register global functions.
*/
struct CASRegisterGlobalFunction final
{
	int operator()( asIScriptEngine& engine, const char* const pszDeclaration,
					const asSFuncPtr& funcPtr, asDWORD callConv, void* pAuxiliary = nullptr )
	{
		return engine.RegisterGlobalFunction( pszDeclaration, funcPtr, callConv, pAuxiliary );
	}
};

/**
*	Registers a varargs function.
*	@param engine Script Engine.
*	@param pszReturnType Return type.
*	@param pszName Function name.
*	@param pszArguments Mandatory parameters.
*	@param uiMinArgs Minimum number of varargs.
*	@param uiMaxArgs Maximum number of varargs.
*	@param funcPtr Function pointer. Use asFUNCTION.
*	@param pAuxiliary Optional. Auxiliary pointer.
*/
inline void RegisterVarArgsFunction( asIScriptEngine& engine,
									 const char* const pszReturnType, const char* const pszName, const char* const pszArguments,
									 size_t uiMinArgs, size_t uiMaxArgs,
									 const asSFuncPtr& funcPtr, void* pAuxiliary = nullptr )
{
	RegisterVarArgs( engine, CASRegisterGlobalFunction(),
					 pszReturnType, pszName, pszArguments,
					 uiMinArgs, uiMaxArgs,
					 funcPtr, pAuxiliary );
}

/**
*	Functor that can register object methods.
*/
struct CASRegisterMethod final
{
	const char* const pszObjectName;

	CASRegisterMethod( const char* const pszObjectName )
		: pszObjectName( pszObjectName )
	{
	}

	CASRegisterMethod( const CASRegisterMethod& other ) = default;

	int operator()( asIScriptEngine& engine, const char* const pszDeclaration,
					const asSFuncPtr& funcPtr, asDWORD callConv, void* pAuxiliary = nullptr )
	{
		return engine.RegisterObjectMethod( pszObjectName, pszDeclaration, funcPtr, callConv, pAuxiliary );
	}

private:
	CASRegisterMethod& operator=( const CASRegisterMethod& ) = delete;
};

/**
*	Registers a varargs function.
*	@param engine Script Engine.
*	@param pszObjectName Object name.
*	@param pszReturnType Return type.
*	@param pszName Function name.
*	@param pszArguments Mandatory parameters.
*	@param uiMinArgs Minimum number of varargs.
*	@param uiMaxArgs Maximum number of varargs.
*	@param funcPtr Function pointer. Use asMETHOD.
*	@param pAuxiliary Optional. Auxiliary pointer.
*/
inline void RegisterVarArgsMethod( asIScriptEngine& engine,
								   const char* const pszObjectName,
								   const char* const pszReturnType, const char* const pszName, const char* const pszArguments,
								   size_t uiMinArgs, size_t uiMaxArgs,
								   const asSFuncPtr& funcPtr, void* pAuxiliary = nullptr )
{
	RegisterVarArgs( engine, CASRegisterMethod( pszObjectName ),
					 pszReturnType, pszName, pszArguments,
					 uiMinArgs, uiMaxArgs,
					 funcPtr, pAuxiliary );
}

/**
*	Iterates over a list of functions.
*/
struct CASFunctionIterator final
{
	const asIScriptModule& module;

	/**
	*	Constructor.
	*	@param module Script module.
	*/
	CASFunctionIterator( const asIScriptModule& module )
		: module( module )
	{
	}

	CASFunctionIterator( const CASFunctionIterator& other ) = default;

	/**
	*	@return Number of functions.
	*/
	asUINT GetCount() const { return module.GetFunctionCount(); }

	/**
	*	Gets a function by index.
	*	@param uiIndex Function index.
	*	@return Function.
	*/
	asIScriptFunction* GetByIndex( const asUINT uiIndex ) const
	{
		return module.GetFunctionByIndex( uiIndex );
	}

private:
	CASFunctionIterator& operator=( const CASFunctionIterator& ) = delete;
};

/**
*	Iterates over a list of object methods.
*/
struct CASMethodIterator final
{
	const asITypeInfo& objectType;

	/**
	*	Constructor.
	*	@param objectType Object type info.
	*/
	CASMethodIterator( const asITypeInfo& objectType )
		: objectType( objectType )
	{
		//Must be an object.
		//TODO: asOBJ_VALUE should also count - Solokiller
		assert( objectType.GetFlags() & ( asOBJ_REF ) );
	}

	CASMethodIterator( const CASMethodIterator& other ) = default;

	/**
	*	@return Number of methods.
	*/
	asUINT GetCount() const { return objectType.GetMethodCount(); }

	/**
	*	Gets a method by index.
	*	@param uiIndex Function index.
	*	@return Method.
	*/
	asIScriptFunction* GetByIndex( const asUINT uiIndex ) const
	{
		return objectType.GetMethodByIndex( uiIndex );
	}

private:
	CASMethodIterator& operator=( const CASMethodIterator& ) = delete;
};

/**
*	Finds a function by name and argument types.
*	@param engine Script engine.
*	@param funcIterator Function iterator.
*	@param szFunctionName Name of the function.
*	@param arguments Function arguments.
*	@param bExplicitReturnType Whether the return type should be checked. Compared against iReturnTypeId.
*	@param iReturnTypeId The return type id to match against if bExplicitReturnType is true.
*	@return Function, or null if no function could be found.
*/
template<typename FUNCITERATOR>
inline asIScriptFunction* FindFunction(
	asIScriptEngine& engine,
	const FUNCITERATOR& funcIterator,
	const std::string& szFunctionName,
	CASArguments& arguments,
	const bool bExplicitReturnType = true,
	const int iReturnTypeId = asTYPEID_VOID )
{
	const auto& argList = arguments.GetArgumentList();

	for( asUINT uiIndex = 0; uiIndex < funcIterator.GetCount(); ++uiIndex )
	{
		asIScriptFunction* const pFunction = funcIterator.GetByIndex( uiIndex );

		//Name must match
		if( strcmp( pFunction->GetName(), szFunctionName.c_str() ) != 0 )
			continue;

		if( bExplicitReturnType )
		{
			if( pFunction->GetReturnTypeId() != iReturnTypeId )
				continue;
		}

		//Must match parameter count
		if( pFunction->GetParamCount() != arguments.GetArgumentCount() )
			continue;

		asUINT uiParamIndex;

		const asUINT uiParamCount = pFunction->GetParamCount();

		//Each parameter must be the correct type
		for( uiParamIndex = 0; uiParamIndex < uiParamCount; ++uiParamIndex )
		{
			int iTypeId;
			asDWORD uiFlags;

			if( pFunction->GetParam( uiParamIndex, &iTypeId, &uiFlags ) < 0 )
			{
				const auto szCandidateFuncName = as::FormatFunctionName( *pFunction );
				as::log->critical( "as::FindFunction: Failed to retrieve parameter {} for function {}!", uiParamIndex, szCandidateFuncName );
				break;
			}

			//Can be null in case of primitive types or enums
			auto pType = engine.GetTypeInfoById( iTypeId );

			const auto& arg = argList[ uiParamIndex ];

			const ArgType::ArgType argType = ctx::ArgumentTypeFromTypeId( iTypeId, pType ? pType->GetFlags() : 0 );

			if( arg.GetArgumentType() != argType )
			{
				//Enum to primitive conversion is handled by the next check
				if( arg.GetArgumentType() != ArgType::ENUM || argType != ArgType::PRIMITIVE )
					break;
			}

			//Make sure only to consider the base id + object flag
			if( ( arg.GetTypeId() & ( asTYPEID_MASK_OBJECT | asTYPEID_MASK_SEQNBR ) ) != ( iTypeId & ( asTYPEID_MASK_OBJECT | asTYPEID_MASK_SEQNBR ) ) )
			{
				ArgumentValue value;

				//Warning: either of these can modify value
				if( ctx::ConvertEnumToPrimitive( arg, iTypeId, value ) )
				{
					//TODO: handle conversion during call.
					/*
					//Change the type to match
					if( !arg.Set( iTypeId, arg.GetArgumentType(), value, false ) )
					{
						as::log->critical( "as::FindFunction: Failed to convert enum value!" );
						break;
					}
					*/
				}
				else
					break;
			}
		}

		//Validation passed
		if( uiParamIndex == uiParamCount )
		{
			return pFunction;
		}
	}

	return nullptr;
}

/**
*	Sets a global variable by name.
*	@param module Module that contains the global variable.
*	@param pszGlobalName Name of the global variable to set.
*	@param value Value to set.
*	@return true on success, false otherwise.
*/
template<typename T>
inline bool SetGlobalByName( asIScriptModule& module, const char* const pszGlobalName, T value )
{
	int iIndex = module.GetGlobalVarIndexByName( pszGlobalName );

	if( iIndex >= 0 )
	{
		void* pGlobal = module.GetAddressOfGlobalVar( iIndex );

		*( reinterpret_cast<T*>( pGlobal ) ) = value;

		return true;
	}

	return false;
}

/**
*	Sets a global variable by name.
*	@param module Module that contains the global variable.
*	@param pszDecl Declaration of the global variable to set.
*	@param value Value to set.
*	@return true on success, false otherwise.
*/
template<typename T>
inline bool SetGlobalByDecl( asIScriptModule& module, const char* const pszDecl, T value )
{
	int iIndex = module.GetGlobalVarIndexByDecl( pszDecl );

	if( iIndex >= 0 )
	{
		void* pGlobal = module.GetAddressOfGlobalVar( iIndex );

		*( reinterpret_cast<T*>( pGlobal ) ) = value;

		return true;
	}

	return false;
}

/*
*	Casting functions used for casting between classes.
*/
template<typename SOURCE_TYPE, typename DEST_TYPE>
DEST_TYPE* Cast_UpCast( SOURCE_TYPE* pThis )
{
	return static_cast<DEST_TYPE*>( pThis );
}

template<typename SOURCE_TYPE, typename DEST_TYPE>
DEST_TYPE* Cast_DownCast( SOURCE_TYPE* pThis )
{
	return dynamic_cast<DEST_TYPE*>( pThis );
}

/*
*	Casting functions used for casting between classes. Handles reference counting.
*/
template<typename SOURCE_TYPE, typename DEST_TYPE>
DEST_TYPE* Cast_UpCast_Ref( SOURCE_TYPE* pThis )
{
	if( pThis )
		pThis->AddRef();

	return static_cast<DEST_TYPE*>( pThis );
}

template<typename SOURCE_TYPE, typename DEST_TYPE>
DEST_TYPE* Cast_DownCast_Ref( SOURCE_TYPE* pThis )
{
	if( pThis )
		pThis->AddRef();

	return dynamic_cast<DEST_TYPE*>( pThis );
}

/**
*	Registers casts between a base and derived type.
*	The casting functions will trigger compile time errors if they are not compatible or don't have virtual functions (needs typeinfo, generated by virtual functions only).
*	@param engine Script engine.
*	@param pszBaseType Baseclass type.
*	@param pszSubType Subclass type.
*	@param upcast Function to use for casting from derived to base.
*	@param downcast Function to use for casting from base to derived.
*	@tparam BASE_TYPE Base class type.
*	@tparam SUB_TYPE Derived class type.
*	@return true on success, false otherwise.
*/
template<typename BASE_TYPE, typename SUB_TYPE>
bool RegisterCasts( asIScriptEngine& engine, const char* const pszBaseType, const char* const pszSubType, 
					SUB_TYPE* ( *upcast )( BASE_TYPE* ) = Cast_UpCast_Ref, BASE_TYPE* ( *downcast )( SUB_TYPE* ) = Cast_DownCast_Ref )
{
	assert( pszBaseType );
	assert( pszSubType );

	//Only register casts if the type being registered is not this type.
	if( strcmp( pszSubType, pszBaseType ) != 0 )
	{
		std::string szName = std::string( pszBaseType ) + "@ opImplCast()";

#ifndef NDEBUG
		auto retCode =
#endif
			engine.RegisterObjectMethod(
				pszSubType, szName.c_str(), asFUNCTION( upcast ), asCALL_CDECL_OBJFIRST );

		assert( retCode >= 0 );

		szName = std::string( pszSubType ) + "@ opCast()";

#ifndef NDEBUG
		retCode =
#endif
			engine.RegisterObjectMethod(
				pszBaseType, szName.c_str(), asFUNCTION( downcast ), asCALL_CDECL_OBJFIRST );

		assert( retCode >= 0 );
	}

	return true;
}

/**
*	Creates a function signature that can be called with the given arguments.
*	@param engine Script engine.
*	@param function Stream that will receive the signature.
*	@param pszReturnType Return type.
*	@param pszFunctionName Function name.
*	@param args Argument list.
*	@param uiStartIndex First argument in the generic call instance to check.
*	@param arguments Generic call instance whose arguments will be used for type information.
*	@return true on success, false otherwise.
*/
bool CreateFunctionSignature(
	asIScriptEngine& engine,
	std::stringstream& function, const char* const pszReturnType, const char* const pszFunctionName,
	const CASArguments& args,
	const asUINT uiStartIndex, asIScriptGeneric& arguments );

/**
*	Extracts a namespace from a name.
*	Namespaces are denoted by double colons. For example, "String::EMPTY_STRING"
*	@param szName Name.
*	@return If a namespace is contained in the name, returns a string containing that namespace. Otherwise, returns an empty string.
*/
inline std::string ExtractNamespaceFromName( const std::string& szName )
{
	if( szName.empty() )
		return "";

	size_t uiIndex = szName.rfind( "::" );

	if( uiIndex == std::string::npos )
		return "";

	return szName.substr( 0, uiIndex );
}

/**
*	Extracts a name from a name that may contain a namespace.
*	Namespaces are denoted by double colons. For example, "String::EMPTY_STRING"
*	@param szName Name.
*	@return If a name is contained in the name, returns a string containing that name. Otherwise, returns an empty string.
*/
inline std::string ExtractNameFromName( const std::string& szName )
{
	if( szName.empty() )
		return "";

	size_t uiIndex = szName.rfind( "::" );

	if( uiIndex == std::string::npos )
		return szName;

	return szName.substr( uiIndex + 2 );
}

/**
*	Extracts a namespace from a declaration.
*	Namespaces are denoted by double colons. For example, "void String::Compare(const string& in lhs, const string& in rhs)"
*	@param szDecl Name.
*	@param bIsFunctionDecl Whether this is a function or a class declaration.
*	@return If a namespace is contained in the declaration, returns a string containing that namespace. Otherwise, returns an empty string.
*/
inline std::string ExtractNamespaceFromDecl( const std::string& szDecl, const bool bIsFunctionDecl = true )
{
	if( szDecl.empty() )
		return "";

	size_t uiStart;

	bool bFoundWhitespace = false;

	for( uiStart = 0; uiStart < szDecl.length(); ++uiStart )
	{
		if( !bFoundWhitespace )
		{
			if( isspace( szDecl[ uiStart ] ) )
			{
				bFoundWhitespace = true;
			}
		}
		else
		{
			if( !isspace( szDecl[ uiStart ] ) )
			{
				break;
			}
		}
	}

	if( uiStart >= szDecl.length() )
		return "";

	size_t uiEnd;
	
	if( bIsFunctionDecl )
	{
		uiEnd = szDecl.find( '(', uiStart + 1 );

		if( uiEnd == std::string::npos )
			return "";
	}
	else
	{
		uiEnd = std::string::npos;
	}

	size_t uiNSEnd = szDecl.rfind( "::", uiEnd );

	if( uiNSEnd == std::string::npos || uiNSEnd <= uiStart )
		return "";

	return szDecl.substr( uiStart, uiNSEnd - uiStart );
}

/**
*	Contains caller info.
*	@see GetCallerInfo
*/
struct CASCallerInfo
{
	const char* pszSection = nullptr;
	int iLine = 0;
	int iColumn = 0;
};

/**
*	Gets information about the calling script.
*	@param[ out ] pszSection Name of the section, or "Unknown" if it couldn't be retrieved.
*	@param[ out ] iLine Line number, or 0 if it couldn't be retrieved.
*	@param[ out ] iColumn Column, or 0 if it couldn't be retrieved.
*	@param pContext Optional. Context to retrieve the info from. If null, calls asGetActiveContext.
*	@return Whether the context was valid.
*/
inline bool GetCallerInfo( const char*& pszSection, int& iLine, int& iColumn, asIScriptContext* pContext = nullptr )
{
	if( !pContext )
		pContext = asGetActiveContext();

	pszSection = nullptr;
	iColumn = 0;
	iLine = 0;

	if( pContext )
		iLine = pContext->GetLineNumber( 0, &iColumn, &pszSection );

	if( !pszSection )
		pszSection = "Unknown";

	return pContext != nullptr;
}

/**
*	Gets information about the calling script.
*	@param[ out ] info Caller info.
*	@param pContext Optional. Context to retrieve the info from. If null, calls asGetActiveContext.
*	@return Whether the context was valid.
*/
inline bool GetCallerInfo( CASCallerInfo& info, asIScriptContext* pContext = nullptr )
{
	return GetCallerInfo( info.pszSection, info.iLine, info.iColumn, pContext );
}
}

/** @} */

#endif //UTIL_ASUTIL_H