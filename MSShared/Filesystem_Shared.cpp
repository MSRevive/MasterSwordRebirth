#include "MSDLLHeaders.h"

#ifdef ISCLIENT
#include "hud.h"
#include "cl_util.h"
#endif

#include <cassert>

#include "FileSystem_Shared.h"
#include "interface.h"

CSysModule* g_pFileSystemModule = NULL;
IFileSystem* g_pFileSystem = NULL;

bool FileSystem_Init()
{
	// Determine which filesystem to use.
	const char* szFsModule = "filesystem_stdio" DEFAULT_SO_EXT;

	char szFSDir[MAX_PATH];
	szFSDir[0] = '\0';

#ifdef ISCLIENT
	if(gEngfuncs.COM_ExpandFilename(szFsModule, szFSDir, sizeof(szFSDir)) == FALSE)
	{
		return false;
	}
#else
	//Just use the filename for the server. No COM_ExpandFilename here.
	strcpy(szFSDir, szFsModule);
#endif

	// Get filesystem interface.
	g_pFileSystemModule = Sys_LoadModule( szFSDir );
	assert( g_pFileSystemModule );

	if( !g_pFileSystemModule )
	{
		return false;
	}

	CreateInterfaceFn fileSystemFactory = Sys_GetFactory( g_pFileSystemModule );
	if( !fileSystemFactory )
	{
		return false;
	}

	g_pFileSystem = ( IFileSystem* ) fileSystemFactory( FILESYSTEM_INTERFACE_VERSION, NULL );
	assert( g_pFileSystem );

	if( !g_pFileSystem )
	{
		return false;
	}

	return true;
}

void FileSystem_Shutdown()
{
	if( g_pFileSystem != NULL )
	{
		g_pFileSystem = NULL;
	}

	if( g_pFileSystemModule != NULL )
	{
		Sys_UnloadModule( g_pFileSystemModule );
		g_pFileSystemModule = NULL;
	}
}
