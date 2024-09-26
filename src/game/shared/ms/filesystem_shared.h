/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#ifndef MSSHARED_FILESYSTEM_SHARED_H
#define MSSHARED_FILESYSTEM_SHARED_H

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include <vector>
#include "FileSystem.h"

/**
*	@file
*
*	Functions, types and globals to load and use the GoldSource engine filesystem interface to read and write files.
*	See the VDC for information on which search paths exist to be used as path IDs:
*	https://developer.valvesoftware.com/wiki/GoldSource_SteamPipe_Directories
*/
enum class FileContentFormat
{
	Binary = 0,
	Text = 1
};

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

/**
*	@brief Loads a file from disk into a buffer.
*
*	@details If the returned buffer contains text data and @p format is @c FileContentFormat::Text it is safe to cast the data pointer to char*:
*	@code{.cpp}
*	auto text = reinterpret_cast<char*>(buffer.data());
*	@endcode
*
*	@param fileName Name of the file to load.
*	@param format If @c FileContentFormat::Text, a null terminator will be appended.
*	@param pathID If not null, only looks for the file in this search path.
*	@return If the file was successfully loaded the contents of the buffer,
*		with a zero byte (null terminator) appended to it if @p format is @c FileContentFormat::Text.
*		If the file could not be loaded an empty buffer is returned.
*/
std::vector<byte> FileSystem_LoadFileIntoBuffer(const char* fileName, FileContentFormat format, const char* pathID = nullptr);

/**
*	@brief Writes a text file to disk.
*	@param fileName Name of the file to write to.
*	@param text Null-terminated text to write. The null terminator is not written to disk.
*	@param pathID If not null, writes to a writable location assigned to the given search path.
*		Otherwise the first writable location will be used (in practice this will be the mod directory).
*		If no writable location exists no file will be written to.
*	@return True if the file was written, false if an error occurred.
*/
bool FileSystem_WriteTextToFile(const char* fileName, const char* text, const char* pathID = nullptr);

/**
*	@brief Helper class to automatically close the file handle associated with a file.
*/
class CFile 
{
public:
	CFile() noexcept = default;
	CFile& operator=(CFile&& other) noexcept;
	CFile& operator=(const CFile&) = delete;
	operator bool() const { return IsOpen(); }

	CFile(const char* filename, const char* options, const char* pathID = nullptr)
	{
		Open(filename, options, pathID);
	}

	CFile(CFile&& other) noexcept : m_Handle(other.m_Handle) 
	{
		other.m_Handle = FILESYSTEM_INVALID_HANDLE;
	}

	~CFile()
	{
		Close();
	}

	CFile(const CFile&) = delete;

	bool IsOpen() const 
	{ 
		return m_Handle != FILESYSTEM_INVALID_HANDLE; 
	}

	size_t Size() const { 
		return static_cast<size_t>(g_pFileSystem->Size(m_Handle)); 
	}

	bool Open(const char* filename, const char* options, const char* pathID = nullptr)
	{
		Close();
		m_Handle = g_pFileSystem->Open(filename, options, pathID);
		return IsOpen();
	}

	void Close()
	{
		if (IsOpen())
		{
			g_pFileSystem->Close(m_Handle);
			m_Handle = FILESYSTEM_INVALID_HANDLE;
		}
	}

	void Seek(int pos, FileSystemSeek_t seekType)
	{
		if (IsOpen())
		{
			g_pFileSystem->Seek(m_Handle, pos, seekType);
		}
	}

	int Read(void* dest, int size)
	{
		return g_pFileSystem->Read(dest, size, m_Handle);
	}

	int Write(const void* input, int size)
	{
		return g_pFileSystem->Write(input, size, m_Handle);
	}

	bool FileExists(const char* filename)
	{
		return g_pFileSystem->FileExists(filename);
	}

	template <typename... Args>
	int Printf(char* format, Args&&... args)
	{
		return g_pFileSystem->FPrintf(m_Handle, format, std::forward<Args>(args)...);
	}

private:
	FileHandle_t m_Handle = FILESYSTEM_INVALID_HANDLE;
};

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a ,b) (((a) > (b)) ? (a) : (b))

#endif //MSSHARED_FILESYSTEM_SHARED_H