#include <cstdint>
#include <memory>
#include <sstream>

#include "ASUtil.h"

namespace as
{
const char* PrimitiveTypeIdToString( int iTypeId )
{
	switch( iTypeId )
	{
	case asTYPEID_VOID:		return "void";
	case asTYPEID_BOOL:		return "bool";
	case asTYPEID_INT8:		return "int8";
	case asTYPEID_INT16:	return "int16";
	case asTYPEID_INT32:	return "int32";
	case asTYPEID_INT64:	return "int64";
	case asTYPEID_UINT8:	return "uint8";
	case asTYPEID_UINT16:	return "uint16";
	case asTYPEID_UINT32:	return "uint32";
	case asTYPEID_UINT64:	return "uint64";
	case asTYPEID_FLOAT:	return "float";
	case asTYPEID_DOUBLE:	return "double";
	default:				return nullptr;
	}
}

void ReleaseVarArg( asIScriptEngine& engine, void* pObject, const int iTypeId )
{
	if( !pObject )
		return;

	auto pTypeInfo = engine.GetTypeInfoById( iTypeId );

	//Handles are pointers to pointers, so dereference it.
	if( iTypeId & asTYPEID_OBJHANDLE )
	{
		pObject = *reinterpret_cast<void**>( pObject );
	}

	if( iTypeId & asTYPEID_MASK_OBJECT )
	{
		engine.ReleaseScriptObject( pObject, pTypeInfo );
	}
}

bool HasDefaultConstructor( const asITypeInfo& type )
{
	//Non-object types return 0 factories here.
	for( asUINT uiIndex = 0; uiIndex < type.GetFactoryCount(); ++uiIndex )
	{
		asIScriptFunction* pFactory = type.GetFactoryByIndex( uiIndex );

		//A default constructor has 0 parameters
		if( pFactory->GetParamCount() == 0 )
			return true;
	}

	return false;
}

void* CreateObjectInstance( asIScriptEngine& engine, const asITypeInfo& type )
{
	if( !HasDefaultConstructor( type ) )
		return nullptr;

	return engine.CreateScriptObject( &type );
}

std::string PODToString( const void* pObject, const int iTypeId )
{
	assert( pObject );

	switch( iTypeId )
	{
	case asTYPEID_VOID:
		{
			//Treat as null handle
			std::ostringstream stream;
			stream << "0x" << std::hex << static_cast<uintptr_t>( 0 );
			return stream.str();
		}

	case asTYPEID_BOOL:
		{
			return *reinterpret_cast<const bool*>( pObject ) != 0 ? "true" : "false";
		}

	case asTYPEID_INT8:
		{
			return std::to_string( *reinterpret_cast<const char*>( pObject ) );
		}

	case asTYPEID_INT16:
		{
			return std::to_string( *reinterpret_cast<const short*>( pObject ) );
		}

	case asTYPEID_INT32:
		{
			return std::to_string( *reinterpret_cast<const long*>( pObject ) );
		}

	case asTYPEID_INT64:
		{
			return std::to_string( *reinterpret_cast<const long long*>( pObject ) );
		}

	case asTYPEID_UINT8:
		{
			return std::to_string( *reinterpret_cast<const unsigned char*>( pObject ) );
		}

	case asTYPEID_UINT16:
		{
			return std::to_string( *reinterpret_cast<const unsigned short*>( pObject ) );
		}

	case asTYPEID_UINT32:
		{
			return std::to_string( *reinterpret_cast<const unsigned long*>( pObject ) );
		}

	case asTYPEID_UINT64:
		{
			return std::to_string( *reinterpret_cast<const unsigned long long*>( pObject ) );
		}

	case asTYPEID_FLOAT:
		{
			return std::to_string( *reinterpret_cast<const float*>( pObject ) );
		}

	case asTYPEID_DOUBLE:
		{
			return std::to_string( *reinterpret_cast<const double*>( pObject ) );
		}

	default:
		{
			assert( false );
			return {};
		}
	}
}

std::string SPrintf( const char* pszFormat, size_t uiFirstParamIndex, asIScriptGeneric& arguments )
{
	//TODO: use fmtlib syntax & backend - Solokiller
	if( !pszFormat || uiFirstParamIndex > static_cast<size_t>( arguments.GetArgCount() ) )
		return {};

	//Total number of arguments - offset
	const size_t uiArgCount = static_cast<size_t>( arguments.GetArgCount() ) - uiFirstParamIndex;

	std::ostringstream stream;

	auto pEngine = arguments.GetEngine();

	//TODO: this should be rewritten to allow for format specifiers.
	while( *pszFormat )
	{
		if( *pszFormat == '%' )
		{
			//We've encountered a format parameter
			++pszFormat;

			if( *pszFormat == '%' )
			{
				//Just insert a %
				stream << '%';
			}
			else
			{
				//Parse index
				char* pszEnd;
				const unsigned int uiIndex = strtoul( pszFormat, &pszEnd, 10 );

				const size_t uiIndexLength = pszEnd - pszFormat;

				//If the index is invalid, stop.
				if( uiIndex == 0 || uiIndex > uiArgCount )
				{
					as::log->critical( "as::SPrintf: format parameter index is out of range!" );
					
					return {};
				}
				else
				{
					//Index is 1 based: make 0 based and offset to first actual varargs parameter
					const asUINT uiArgIndex = ( uiIndex - 1 ) + (asUINT)uiFirstParamIndex;
					const int iTypeId = arguments.GetArgTypeId( uiArgIndex );

					void* pValue = arguments.GetArgAddress( uiArgIndex );

					if( as::IsPrimitive( iTypeId ) )
					{
						stream << PODToString( pValue, iTypeId );
					}
					else
					{
						auto pType = pEngine->GetTypeInfoById( iTypeId );

						if( pType )
						{
							//It's a string
							if( strcmp( pType->GetName(), "string" ) == 0 )
							{
								const auto& szString = *reinterpret_cast<const std::string*>( pValue );

								stream << szString;
							}
							else
							{
								//Pointer types
								stream << "0x" << std::hex << reinterpret_cast<uintptr_t>( pValue ) << std::dec;
							}
						}
						else
						{
							if( pValue ) //Treat as dword
								stream << *reinterpret_cast<long*>( pValue );
							else //Treat as pointer
								stream << "0x" << std::hex << reinterpret_cast<uintptr_t>( pValue ) << std::dec;
						}
					}

					pszFormat += uiIndexLength - 1; //Account for the ++ below
				}
			}
		}
		else
		{
			stream << *pszFormat;
		}

		++pszFormat;
	}

	return stream.str();
}

bool CreateFunctionSignature(
	asIScriptEngine& engine,
	std::stringstream& function, const char* const pszReturnType, const char* const pszFunctionName,
	const CASArguments& args,
	const asUINT uiStartIndex, asIScriptGeneric& arguments )
{
	assert( pszReturnType );
	assert( pszFunctionName );

	if( !pszReturnType ||!pszFunctionName )
		return false;

	bool bSuccess = true;

	function << pszReturnType << ' ' << pszFunctionName << '(';

	const size_t uiCount = args.GetArgumentCount();

	//Needs to match.
	if( uiCount != ( arguments.GetArgCount() - uiStartIndex ) )
	{
		return false;
	}

	for( asUINT uiIndex = 0; uiIndex < uiCount && bSuccess; ++uiIndex )
	{
		const auto pArg = args.GetArgument( uiIndex );

		const int iTypeId = arguments.GetArgTypeId( uiIndex + uiStartIndex );

		const auto pszTypeDecl = engine.GetTypeDeclaration( iTypeId, true );

		if( !pszTypeDecl )
		{
			bSuccess = false;
			break;
		}

		if( uiIndex > 0 )
		{
			//Get ready for the next argument to be appended.
			function << ", ";
		}

		function << pszTypeDecl;

		switch( pArg->GetArgumentType() )
		{
		default:
		case ArgType::NONE:
		case ArgType::VOID:
			{
				//This should never occur, unless the argument parser fails without returning false
				as::log->critical( "as::CreateFunctionSignature: invalid input argument!" );
				bSuccess = false;
				break;
			}

		case ArgType::PRIMITIVE:
		case ArgType::ENUM:			break;
		case ArgType::VALUE:		function << "& in"; break; //Value types are always passed by reference.
		case ArgType::REF:
			{
				//Always pass as handle
				if( pszTypeDecl[ strlen( pszTypeDecl ) - 1 ] != '@' )
					function << '@';

				break;
			}
		}
	}

	if( bSuccess )
	{
		function << ')';
	}

	return bSuccess;
}
}