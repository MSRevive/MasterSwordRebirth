#ifndef COMMON_CSTRINGPOOL_H
#define COMMON_CSTRINGPOOL_H

#include <cstring>
#include <map>

/**
*	@brief A generic string pool for use as a replacement for the engine's string pool
*/
class CStringPool
{
private:
	struct LessFunc_t
	{
		bool operator()( const char* pszLhs, const char* pszRhs ) const
		{
			return strcmp( pszLhs, pszRhs ) < 0;
		}
	};

	typedef std::map<const char*, char*, LessFunc_t> StringMap_t;
public:
	CStringPool();
	~CStringPool();

	const char* Find( const char* pszString ) const;

	const char* FindOrAllocate( const char* pszString );

	void Clear();

private:
	StringMap_t m_StringMap;

private:
	CStringPool( const CStringPool& );
	CStringPool& operator=( const CStringPool& );
};

#endif //COMMON_CSTRINGPOOL_H
