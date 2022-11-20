#include "msdllheaders.h"

#ifdef ISCLIENT
#include "hud.h"
#include "cl_util.h"
#endif

#include <cassert>
#include <limits.h>

#include "filesystem_shared.h"
#include "interface.h"

CSysModule* g_pFileSystemModule = nullptr;
IFileSystem* g_pFileSystem = nullptr;

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
	strncpy(szFSDir, szFsModule, sizeof(szFSDir));
#endif

	// Get filesystem interface.
	g_pFileSystemModule = Sys_LoadModule(szFSDir);
	assert(g_pFileSystemModule != nullptr);

	if(g_pFileSystemModule == nullptr)
	{
		return false;
	}

	CreateInterfaceFn fileSystemFactory = Sys_GetFactory(g_pFileSystemModule);
	if(!fileSystemFactory)
	{
		return false;
	}

	g_pFileSystem = (IFileSystem*) fileSystemFactory(FILESYSTEM_INTERFACE_VERSION, NULL);
	assert(g_pFileSystem != nullptr);

	if(g_pFileSystem == nullptr)
	{
		return false;
	}

	return true;
}

void FileSystem_Shutdown()
{
	if( g_pFileSystem != nullptr )
	{
		g_pFileSystem = nullptr;
	}

	if( g_pFileSystemModule != nullptr )
	{
		Sys_UnloadModule(g_pFileSystemModule);
		g_pFileSystemModule = nullptr;
	}
}

std::vector<byte> FileSystem_LoadFileIntoBuffer(const char* fileName, FileContentFormat format, const char* pathID)
{
	assert(g_pFileSystem != nullptr);

	if (fileName == nullptr)
	{
		return {};
	}

	CFile file{fileName, "rb", pathID};
	if (file)
	{
		size_t size = file.Size();
		std::vector<byte> buffer;
		buffer.resize(size + (format == FileContentFormat::Text ? 1 : 0));
		file.Read(buffer.data(), size);

		if (format == FileContentFormat::Text)
		{
			//Null terminate it in case it's actually text.
			buffer[size] = byte{'\0'};
		}

		return buffer;
	}

	ALERT(at_console, "FileSystem_LoadFileIntoBuffer: couldn't open file \"%s\" for reading\n", fileName);
	return {};
}

bool FileSystem_WriteTextToFile(const char* fileName, const char* text, const char* pathID)
{
	assert(g_pFileSystem != nullptr);

	if (fileName == nullptr || text == nullptr)
	{
		return false;
	}

	const size_t length = strlen(text);

	//std::numeric_limits<int>::max() doesn't work cause of macro for max/min
	//if (length > static_cast<std::size_t>(std::numeric_limits<int>::max()))
	if (length > INT_MAX)
	{
		ALERT(at_console, "FileSystem_WriteTextToFile: text too long\n");
		return false;
	}

	CFile file{fileName, "rb", pathID};
	if (file)
	{
		file.Write(text, length);
		return true;
	}

	ALERT(at_console, "FileSystem_WriteTextToFile: couldn't open file \"%s\" for writing\n", fileName);
	return false;
}