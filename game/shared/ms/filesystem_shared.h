#ifndef MSSHARED_FILESYSTEM_SHARED_H
#define MSSHARED_FILESYSTEM_SHARED_H

#include "FileSystem.h"

/**
*	@file
*
*	Engine filesystem shared game code
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
::mslist<byte> FileSystem_LoadFileIntoBuffer(const char* fileName, FileContentFormat format, const char* pathID = nullptr);

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
	constexpr operator bool() const { return IsOpen(); }

	CFile(const char* filename, const char* options, const char* pathID = nullptr)
	{
		Open(filename, options, pathID);
	}

	CFile(CFile&& other) noexcept : _handle(other._handle) 
	{
		other._handle = FILESYSTEM_INVALID_HANDLE;
	}

	~CFile()
	{
		Close();
	}

	CFile(const CFile&) = delete;

	constexpr bool IsOpen() const 
	{ 
		return _handle != FILESYSTEM_INVALID_HANDLE; 
	}

	size_t Size() const { 
		return static_cast<size_t>(g_pFileSystem->Size(_handle)); 
	}

	bool Open(const char* filename, const char* options, const char* pathID = nullptr)
	{
		Close();
		_handle = g_pFileSystem->Open(filename, options, pathID);
		return IsOpen();
	}

	void Close()
	{
		if (IsOpen())
		{
			g_pFileSystem->Close(_handle);
			_handle = FILESYSTEM_INVALID_HANDLE;
		}
	}

	void Seek(int pos, FileSystemSeek_t seekType)
	{
		if (IsOpen())
		{
			g_pFileSystem->Seek(_handle, pos, seekType);
		}
	}

	int Read(void* dest, int size)
	{
		return g_pFileSystem->Read(dest, size, _handle);
	}

	int Write(const void* input, int size)
	{
		return g_pFileSystem->Write(input, size, _handle);
	}

	bool FileExists(const char* filename)
	{
		return g_pFileSystem->FileExists(filename);
	}

	template <typename... Args>
	int Printf(const char* format, Args&&... args)
	{
		return g_pFileSystem->FPrintf(_handle, format, std::forward<Args>(args)...);
	}

private:
	FileHandle_t _handle = FILESYSTEM_INVALID_HANDLE;
};

#endif //MSSHARED_FILESYSTEM_SHARED_H