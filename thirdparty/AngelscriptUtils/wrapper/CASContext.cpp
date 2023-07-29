#include <cassert>

#include "AngelscriptUtils/IASContextResultHandler.h"

#include "CASContext.h"

void CASOwningContext::Release()
{
	if( m_pContext )
	{
		const auto result = m_pContext->Unprepare();

		auto pResultHandler = as::GetContextResultHandler( *m_pContext );

		if( pResultHandler )
			pResultHandler->ProcessUnprepareResult( *m_pContext, result );
	}

	if( m_pEngine )
	{
		m_pEngine->ReturnContext( m_pContext );
		m_pContext = nullptr;

		m_pEngine->Release();
		m_pEngine = nullptr;
	}
	else if( m_pContext )
	{
		m_pContext->Release();
		m_pContext = nullptr;
	}
}

void CASOwningContext::ReleaseOwnership()
{
	if( m_pContext )
	{
		m_pContext = nullptr;
	}

	if( m_pEngine )
	{
		m_pEngine = nullptr;
	}
}