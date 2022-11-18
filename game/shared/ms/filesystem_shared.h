#ifndef MSSHARED_FILESYSTEM_SHARED_H
#define MSSHARED_FILESYSTEM_SHARED_H

#include "FileSystem.h"

/**
*	@file
*
*	Engine filesystem shared game code
*/
extern IFileSystem* g_pFileSystem; //we should use inline, but we're using stdcpp14

/**
*	@brief Loads the filesystem library and gets the filesystem from it
*	@return Whether the filesystem was loaded and initialized successfully
*/
bool FileSystem_Init();

/**
*	@brief Shuts down and unloads the filesystem
*/
void FileSystem_Shutdown();

#endif //MSSHARED_FILESYSTEM_SHARED_H