#ifndef MSSHARED_FILESYSTEM_SHARED_H
#define MSSHARED_FILESYSTEM_SHARED_H

#include "FileSystem.h"

/**
*	@file
*
*	Engine filesystem shared game code
*/

/**
*	@brief Loads the filesystem library and gets the filesystem from it
*	@return Whether the filesystem was loaded and initialized successfully
*/
bool FileSystem_Init();

/**
*	@brief Shuts down and unloads the filesystem
*/
void FileSystem_Shutdown();

/**
*	@brief The engine's filesystem
*/
extern IFileSystem* g_pFileSystem;

#endif //MSSHARED_FILESYSTEM_SHARED_H
