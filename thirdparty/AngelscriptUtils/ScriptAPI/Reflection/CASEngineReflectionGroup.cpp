#include <cassert>

#include "AngelscriptUtils/util/ASUtil.h"

#include "CASEngineReflectionGroup.h"

asIScriptFunction* CASEngineReflectionGroup::FindGlobalFunction( const std::string& szName, bool bSearchByDecl )
{
	assert( m_pEngine );

	asIScriptFunction* pFunction = nullptr;

	if( bSearchByDecl )
	{
		const std::string szOldNS = m_pEngine->GetDefaultNamespace();

		const std::string szNS = as::ExtractNamespaceFromDecl( szName );

		m_pEngine->SetDefaultNamespace( szNS.c_str() );

		pFunction = m_pEngine->GetGlobalFunctionByDecl( szName.c_str() );

		m_pEngine->SetDefaultNamespace( szOldNS.c_str() );
	}
	else
	{
		const std::string szNS = as::ExtractNamespaceFromName( szName );
		const std::string szActualName = as::ExtractNameFromName( szName );

		const asUINT uiCount = m_pEngine->GetGlobalFunctionCount();

		for( asUINT uiIndex = 0; uiIndex < uiCount; ++uiIndex )
		{
			auto pFunc = m_pEngine->GetGlobalFunctionByIndex( uiIndex );

			if( szActualName == pFunc->GetName() && szNS == pFunc->GetNamespace() )
			{
				pFunction = pFunc;
				break;
			}
		}
	}

	if( pFunction )
		pFunction->AddRef();

	return pFunction;
}

asUINT CASEngineReflectionGroup::GetGlobalFunctionCount() const
{
	assert( m_pEngine );

	return m_pEngine->GetGlobalFunctionCount();
}

asIScriptFunction* CASEngineReflectionGroup::GetGlobalFunctionByIndex( asUINT uiIndex )
{
	assert( m_pEngine );

	if( auto pFunction = m_pEngine->GetGlobalFunctionByIndex( uiIndex ) )
	{
		pFunction->AddRef();
		return pFunction;
	}

	return nullptr;
}

asITypeInfo* CASEngineReflectionGroup::FindTypeInfo( const std::string& szName, bool bSearchByDecl )
{
	assert( m_pEngine );

	const std::string szOldNS = m_pEngine->GetDefaultNamespace();

	asITypeInfo* pType;

	if( bSearchByDecl )
	{
		const std::string szNS = as::ExtractNamespaceFromDecl( szName, false );

		m_pEngine->SetDefaultNamespace( szNS.c_str() );

		pType = m_pEngine->GetTypeInfoByDecl( szName.c_str() );
	}
	else
	{
		const std::string szNS = as::ExtractNamespaceFromName( szName );
		const std::string szActualName = as::ExtractNameFromName( szName );

		m_pEngine->SetDefaultNamespace( szNS.c_str() );

		pType = m_pEngine->GetTypeInfoByName( szActualName.c_str() );
	}

	m_pEngine->SetDefaultNamespace( szOldNS.c_str() );

	if( pType )
		pType->AddRef();

	return pType;
}

asUINT CASEngineReflectionGroup::GetObjectTypeCount() const
{
	assert( m_pEngine );

	return m_pEngine->GetObjectTypeCount();
}

asITypeInfo* CASEngineReflectionGroup::GetObjectTypeByIndex( asUINT uiIndex )
{
	assert( m_pEngine );

	if( auto pType = m_pEngine->GetObjectTypeByIndex( uiIndex ) )
	{
		pType->AddRef();
		return pType;
	}

	return nullptr;
}