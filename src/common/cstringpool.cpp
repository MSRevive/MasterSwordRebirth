#include "cstringpool.h"

CStringPool::CStringPool()
{
}

CStringPool::~CStringPool()
{
	Clear();
}

const char* CStringPool::Find( const char* pszString ) const
{
	StringMap_t::const_iterator it = m_StringMap.find( pszString );

	if( it != m_StringMap.end() )
		return it->second;

	return NULL;
}

const char* CStringPool::FindOrAllocate( const char* pszString )
{
	{
		const char* pszResult = Find( pszString );

		if( pszResult != NULL )
			return pszResult;
	}

	const size_t l = strlen( pszString ) + 1;

	//Copy the key. Needed so Find can compare with the original string
	//If you get rid of the Quake newline stuff, you can use this for both key and value (half the allocations)
	char* pszKey = new char[ l ];

	strcpy( pszKey, pszString );

	char* pszResult = new char[ l ];

	char* pszNew = pszResult;

	//This code is from Quake, also used by the engine
	//Strings are copied and modified so "\n" becomes '\n' (single character), '\' followed by anything else is changed to just '\'
	for( size_t i = 0; i < l; ++i )
	{
		if( pszString[ i ] == '\\' && i < l - 1 )
		{
			++i;

			if( pszString[ i ] == 'n' )
				*pszNew++ = '\n';
			else
				*pszNew++ = '\\';
		}
		else
			*pszNew++ = pszString[ i ];
	}

	m_StringMap.insert( std::make_pair( pszKey, pszResult ) );
	
	return pszResult;
}

void CStringPool::Clear()
{
	for( StringMap_t::const_iterator it = m_StringMap.begin(), end = m_StringMap.end(); it != end; ++it )
	{
		delete[] it->first;
		delete[] it->second;
	}

	m_StringMap.clear();
}
