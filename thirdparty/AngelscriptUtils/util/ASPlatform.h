#ifndef ANGELSCRIPT_UTIL_PLATFORM_H
#define ANGELSCRIPT_UTIL_PLATFORM_H

#include <cstdio>

#ifdef WIN32
	#if _MSC_VER < 1900
		#define snprintf _snprintf
	#endif
#endif

/**
*	Creates a directory
*/
void MakeDirectory( const char* pszDirectoryName );

/**
*	Changes the working directory
*/
void ChangeDirectory( const char* pszDirectoryName );

template<typename T, size_t SIZE>
inline size_t _ASArraySizeof( const T( & )[ SIZE ] )
{
	return SIZE;
}

#define ASARRAYSIZE( p )	_ASArraySizeof( p )

#endif //ANGELSCRIPT_UTIL_PLATFORM_H
