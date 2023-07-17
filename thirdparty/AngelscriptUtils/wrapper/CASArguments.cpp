#include <cassert>
#include <cstdarg>

#include <angelscript.h>

#include "AngelscriptUtils/CASManager.h"
#include "AngelscriptUtils/util/ASLogging.h"
#include "AngelscriptUtils/util/ASUtil.h"
#include "AngelscriptUtils/util/ContextUtils.h"

#include "CASArguments.h"

CASArgument::~CASArgument()
{
	Reset();
}

CASArgument::CASArgument( const CASArgument& other )
{
	Set( other );
}

CASArgument& CASArgument::operator=( const CASArgument& other )
{
	Set( other );

	return *this;
}

void* CASArgument::GetArgumentAsPointer() const
{
	if( !HasValue() )
		return nullptr;

	switch( m_ArgType )
	{
	case ArgType::PRIMITIVE:
	case ArgType::ENUM:		return const_cast<asQWORD*>( &m_Value.qword );

	case ArgType::REF:
	case ArgType::VALUE:	return m_Value.pValue;

	default:				return nullptr;
	}
}

bool CASArgument::Set( asIScriptEngine& engine, const int iTypeId, const ArgType::ArgType type, const ArgumentValue& value, const bool bCopy )
{
	Reset();

	if( type == ArgType::NONE )
		return true;

	bool bSuccess = true;

	m_iTypeId = iTypeId;
	m_ArgType = type;

	if( bCopy )
	{
		if( type == ArgType::VALUE || type == ArgType::REF )
		{
			if( asITypeInfo* pType = engine.GetTypeInfoById( iTypeId ) )
			{
				//Need to copy value
				if( type == ArgType::VALUE )
				{
					m_Value.pValue = engine.CreateScriptObjectCopy( value.pValue, pType );
				}
				else
				{
					//Need to addref
					engine.AddRefScriptObject( value.pValue, pType );
					m_Value = value;
				}
			}
			else
			{
				as::log->critical( "CASArgument::Set: Failed to get object type!" );
				bSuccess = false;
			}
		}
		else if( type == ArgType::VOID )
		{
			//Do nothing
		}
		else
		{
			//Primitive type or enum, just copy
			m_Value = value;
		}
	}
	else if( type != ArgType::VOID )
	{
		m_Value = value;
	}

	return bSuccess;
}

bool CASArgument::Set( const int iTypeId, const ArgType::ArgType type, const ArgumentValue& value, const bool bCopy )
{
	return Set( *CASManager::GetActiveManager()->GetEngine(), iTypeId, type, value, bCopy );
}

bool CASArgument::Set( const CASArgument& other )
{
	if( this == &other )
		return true;

	return Set( other.GetTypeId(), other.GetArgumentType(), other.GetArgumentValue(), true );
}

void CASArgument::Reset()
{
	if( HasValue() )
	{
		//Release reference if needed
		if( !as::IsPrimitive( m_iTypeId ) && !as::IsEnum( m_iTypeId ) )
		{
			auto pEngine = CASManager::GetActiveManager()->GetEngine();

			asITypeInfo* pType = pEngine->GetTypeInfoById( m_iTypeId );

			if( pType )
				pEngine->ReleaseScriptObject( m_Value.pValue, pType );
			else
			{
				as::log->critical( "CASArgument::Reset: Failed to get object type!" );
			}
		}

		m_iTypeId = -1;
		m_ArgType = ArgType::NONE;
		m_Value = ArgumentValue();
	}
}

CASArguments::CASArguments( asIScriptGeneric& arguments, size_t uiStartIndex )
{
	SetArguments( arguments, uiStartIndex );
}

CASArguments::CASArguments( asIScriptFunction& targetFunc, va_list list )
{
	SetArguments( targetFunc, list );
}

CASArguments::~CASArguments()
{
	Clear();
}

void CASArguments::Release() const
{
	if( InternalRelease() )
		delete this;
}

CASArguments::CASArguments( const CASArguments& other )
{
	Assign( other );
}

CASArguments& CASArguments::operator=( const CASArguments& other )
{
	Assign( other );

	return *this;
}

void CASArguments::Assign( const CASArguments& other )
{
	if( this != &other )
	{
		Clear();

		m_Arguments.resize( other.GetArgumentCount() );

		if( other.HasArguments() )
		{
			const auto& sourceArgs = other.GetArgumentList();

			//Failure is unlikely, but there might be issues copying between lists that don't occur in init from scripts
			bool bSuccess = true;

			auto pEngine = CASManager::GetActiveManager()->GetEngine();

			for( size_t uiIndex = 0; uiIndex < m_Arguments.size() && bSuccess; ++uiIndex )
			{
				const CASArgument& sourceArg = sourceArgs[ uiIndex ];

				bSuccess = m_Arguments[ uiIndex ].Set( *pEngine, sourceArg.GetTypeId(), sourceArg.GetArgumentType(), sourceArg.GetArgumentValue(), true );
			}

			if( !bSuccess )
				Clear();
		}
	}
}

void CASArguments::Clear()
{
	m_Arguments.clear();
	m_Arguments.shrink_to_fit();
}

const CASArgument* CASArguments::GetArgument( const size_t uiIndex ) const
{
	assert( uiIndex < m_Arguments.size() );

	if(  uiIndex >= m_Arguments.size() )
		return nullptr;

	return &m_Arguments[ uiIndex ];
}

bool CASArguments::SetArguments( asIScriptGeneric& arguments, size_t uiStartIndex )
{
	const size_t uiArgCount = static_cast<size_t>( arguments.GetArgCount() );

	//If true, an internal error occured
	if( uiStartIndex > uiArgCount )
	{
		as::log->critical( "CASArguments::SetArguments: Start index is greater than argument count!" );
		return false;
	}

	const asUINT uiTargetArgs = (asUINT)uiArgCount - (asUINT)uiStartIndex;

	bool bSuccess = true;

	Arguments_t args( uiTargetArgs );

	auto pEngine = CASManager::GetActiveManager()->GetEngine();

	for( asUINT uiIndex = 0; uiIndex < uiTargetArgs && bSuccess; ++uiIndex )
	{
		void* pData = arguments.GetArgAddress( uiIndex + (asUINT)uiStartIndex );
		int iTypeId = arguments.GetArgTypeId( uiIndex + (asUINT)uiStartIndex );

		bSuccess = ctx::SetArgument( *pEngine, pData, iTypeId, args[ uiIndex ] );
	}

	if( bSuccess )
	{
		m_Arguments = std::move( args );
	}

	return bSuccess;
}

//Store arguments in this object
bool CASArguments::SetArguments( asIScriptFunction& targetFunc, va_list list )
{
	if( !list )
		return false;

	Clear();

	const asUINT uiArgCount = targetFunc.GetParamCount();

	Arguments_t args( uiArgCount );

	bool bSuccess = true;

	auto pEngine = CASManager::GetActiveManager()->GetEngine();

	int iTypeId;
	asDWORD uiFlags;

	ArgumentValue value;

	ctx::VAList vaList;
	
	//GCC fails to copy the list if it's done any other way.
	va_copy( vaList.list, list );

	for( asUINT uiIndex = 0; uiIndex < uiArgCount && bSuccess; ++uiIndex )
	{
		if( targetFunc.GetParam( uiIndex, &iTypeId, &uiFlags ) < 0 )
		{
			as::log->critical( "CASArguments::SetArguments(va_list): An error occurred while getting function parameter information, aborting!" );
			bSuccess = false;
			break;
		}

		asITypeInfo* pType = pEngine->GetTypeInfoById( iTypeId );

		asDWORD uiObjFlags = pType->GetFlags();
		ArgType::ArgType argType;

		if( ( bSuccess = ctx::GetArgumentFromVarargs( value, iTypeId, uiFlags, vaList, &uiObjFlags, &argType ) ) != false )
		{
			//Make copies of the input arguments; they won't exist anymore once this method has finished execution.
			args[ uiIndex ].Set( *pEngine, iTypeId, argType, value, true );
		}
		else
		{
			const auto szFunctionName = as::FormatFunctionName( targetFunc );

			as::log->critical( "CASArguments::SetArguments(va_list): Function '{}': failed to set argument {}, aborting!",
							   szFunctionName, uiIndex );
		}
	}

	if( bSuccess )
	{
		m_Arguments = std::move( args );
	}

	return bSuccess;
}