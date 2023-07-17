/**
*	@file
*
*	Implementations for platform specific code
*/
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <direct.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "ASPlatform.h"

#ifdef WIN32
void MakeDirectory( const char* pszDirectoryName )
{
	CreateDirectoryA( pszDirectoryName, nullptr );
}

void ChangeDirectory( const char* pszDirectoryName )
{
	_chdir( pszDirectoryName );
}
#else
void MakeDirectory( const char* pszDirectoryName )
{
	mkdir( pszDirectoryName, S_IRWXU | S_IRWXG | S_IRWXO );
}

void ChangeDirectory( const char* pszDirectoryName )
{
	chdir( pszDirectoryName );
}
#endif
