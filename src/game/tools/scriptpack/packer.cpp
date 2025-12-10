#include <iostream>
#include <fstream>
#include <thread>
#include <sys/stat.h>

#include "cbase.h"
#include "packer.h"
//#include "parser.h"
#ifdef _MSR_UTILS
#include "scriptmodule/scriptmodule.h"
#endif

extern bool g_Verbose;
extern bool g_Release;
extern bool g_ErrFile;
extern bool g_FailOnErr;

Packer::Packer(std::string wDir, std::string rDir, std::string oDir)
{
	m_WorkDir = wDir;
	m_RootDir = rDir;
	m_OutDir = oDir;

	std::cout << "Work directory set to: " << m_WorkDir << std::endl;
}

void Packer::readDirectory(std::string pszName)
{
	std::filesystem::path fsPath = pszName;

	for(const auto &entry : std::filesystem::recursive_directory_iterator(fsPath))
	{
		if (g_Release && entry.path().parent_path().filename() == "developer")
			continue;

		if (g_Verbose && entry.is_directory())
		{
			std::cout << "Reading Directory: " << entry.path().string() << std::endl;
		}

		if( entry.is_regular_file() )
		{
			auto file = entry.path();
			if(file.extension() == ".script" || file.extension() == ".as" || file.filename() == "items.txt")
			{
				if( g_Verbose )
					std::cout << file.string() << std::endl;

				m_StoredFiles.push_back(file.string());
			}
		}
	}
}

void Packer::packScripts()
{
	namespace fs = std::filesystem;
	fs::path writePath = fs::path(m_OutDir) / "scripts.pak";
	std::fstream stream(writePath, std::ios::binary | std::ios::out | std::ios::in | std::ios::trunc);

	if (!stream.is_open())
	{
		std::cerr << "Failed to create " << writePath << std::endl;
		exit(-1);
	}

	size_t baseDirLen = m_WorkDir.size() + 1;
	size_t listSize = m_StoredFiles.size();

	if (listSize == 0)
	{
		std::cerr << "ERROR: No files in list!" << std::endl;
		exit(-1);
	}

	pakHeader_t Header;
	Header.MagicNumber = 1262698832;
	Header.DirectoryOffset = sizeof(pakHeader_t);
	Header.DirectoryCount = listSize;

	pakDirectory_t dummy;
	std::memset(&dummy, 0, sizeof(pakDirectory_t)); 

	// write file header
	stream.write(reinterpret_cast<const char*>(&Header), sizeof(pakHeader_t));

	// write out dummy data to occupy the file metadata position
	for (size_t i = 0; i < listSize; i++)
	{
		stream.write(reinterpret_cast<const char*>(&dummy), sizeof(pakDirectory_t));
	}

	// end of the directory block
	std::streampos currentFileBytesWritten = stream.tellp();

	// jump back to just after the header to start writing real directory entries
	stream.seekp(sizeof(pakHeader_t));

	size_t count = 0;

	std::cout << listSize << std::endl;

	for (std::string& file : m_StoredFiles)
	{
		std::cout << "Script #" << count++ << std::endl;

		auto contents = getFileContents(file);
		if (contents.empty())
		{
			std::cerr << "ERROR: file: " << file << " is empty!" << std::endl;
			continue;
		}

		std::cout << file << std::endl;
		std::string relativePath = (file.length() > baseDirLen) ? file.substr(baseDirLen) : file;
		std::cout << relativePath << std::endl;

		if (g_Verbose)
			std::cout << "Processing " << relativePath << "..." << std::endl;

		processScript(contents, relativePath);

		std::string safeStr = buildString(contents);
		size_t strSize = safeStr.size();

		pakDirectory_t FileEntry;
		std::memset(&FileEntry, 0, sizeof(pakDirectory_t));
		
		strncpy(FileEntry.cFilename, relativePath.c_str(), sizeof(FileEntry.cFilename) - 1);
		FileEntry.FileOffset = 0; // placeholder
		FileEntry.FileSize = strSize;

		if (g_Verbose)
			std::cout << "Packing file: " << FileEntry.cFilename << std::endl;

		// remember where we are in the directory block
		std::streampos entryPosition = stream.tellp();

		// write the Directory entry (with incorrect offset for now)
		stream.write(reinterpret_cast<const char*>(&FileEntry), sizeof(pakDirectory_t));

		if (stream.fail()) 
			std::cerr << "Failed to write directory entry: " << FileEntry.cFilename << std::endl;

		// jump to the end of the file (where data lives)
		stream.seekp(currentFileBytesWritten);

		// update the FileOffset to the current position
		FileEntry.FileOffset = (unsigned int)stream.tellp();

		// write the actual file data
		stream.write(safeStr.c_str(), strSize);

		if (stream.fail())
			std::cerr << "Failed to write file data: " << FileEntry.cFilename << std::endl;

		// update our tracker for the end of the file
		currentFileBytesWritten = stream.tellp();

		// jump back to the directory entry position
		stream.seekp(entryPosition);

		// rewrite the directory entry with the correct FileOffset
		stream.write(reinterpret_cast<const char*>(&FileEntry), sizeof(pakDirectory_t));

		std::cout << std::endl;
	}

	std::cout << "Finished packing..." << std::endl;
}

void Packer::processScript(std::vector<std::byte> &buffer, std::string relativeFile)
{
	if (relativeFile == "items.txt")
	{
		return;
	}
	else if (relativeFile.find(".as") != std::string::npos)
	{
#ifdef _MSR_UTILS
		// Check if this file contains module syntax
		std::string norm_str(reinterpret_cast<const char*>(buffer.data()));
		if (norm_str.find("module ") != std::string::npos)
		{
			// Use the scriptmodule addon to preprocess the file
			CScriptModule moduleProcessor;
			std::string moduleName;
			std::string processedSource;
			std::string namespaceName = "GameMaster";

			if (moduleProcessor.PreprocessModuleSource(norm_str, processedSource, moduleName, namespaceName))
			{
				norm_str = processedSource;
				std::cout << "Preprocessed module: " << moduleName << " in " << relativeFile << std::endl;
			}
			else
			{
				std::cout << "Note: File contains 'module' keyword but is not a valid module: " << relativeFile << std::endl;
			}
		}
		return;
#endif
	}
	
	if (g_Release)
	{
		stripComments(buffer);
		stripEmptyLines(buffer);
		stripWhiteSpace(buffer);
	}
}

void Packer::stripComments(std::vector<std::byte>& data)
{
	if (data.empty()) {
		return;
	}

	size_t write_idx = 0;
	bool in_comment = false; // Are we inside a comment?
	bool in_string = false; // Are we inside a string?

	for (size_t read_idx = 0; read_idx < data.size(); ++read_idx) {
		
		const char c = static_cast<char>(data[read_idx]);
		
		char next_c = '\0'; // Look-ahead character
		if (read_idx + 1 < data.size()) {
			next_c = static_cast<char>(data[read_idx + 1]);
		}

		if (in_string) {
			if (c == '"' && (write_idx == 0 || static_cast<char>(data[write_idx - 1]) != '\\')) {
				in_string = false;
			}

			data[write_idx++] = data[read_idx];
		} else if (in_comment) {
			if (c == '\n') {
				in_comment = false;
				data[write_idx++] = data[read_idx];
			}
		} else {
			if (c == '"') {
				in_string = true;
				data[write_idx++] = data[read_idx];
			
			} else if (c == '/' && next_c == '/') {
				in_comment = true;
				read_idx++; 
			} else {
				data[write_idx++] = data[read_idx];
			}
		}
	}

	data.resize(write_idx);
}

void Packer::stripWhiteSpace(std::vector<std::byte>& data) {
	if (data.empty()) {
		return;
	}

	size_t write_idx = 0;
	bool in_string = false; // Are we inside a string?
	bool at_start_of_line = true; // Are we at the start of a (potential) line?
	bool last_char_was_space = false; // Was the last *written* char a space?

	for (size_t read_idx = 0; read_idx < data.size(); ++read_idx) {
		const char c = static_cast<char>(data[read_idx]);

		if (in_string) {
			data[write_idx++] = data[read_idx];

			if (c == '"' && (write_idx <= 1 || static_cast<char>(data[write_idx - 2]) != '\\')) {
				in_string = false;
			}

			at_start_of_line = false;
			last_char_was_space = false;
		} else {
			if (c == '"') {
				in_string = true;
				data[write_idx++] = data[read_idx];
				at_start_of_line = false;
				last_char_was_space = false;

			} else if (c == '\n') {
				if (!at_start_of_line) {
					data[write_idx++] = data[read_idx];
				}

				at_start_of_line = true; 
				last_char_was_space = true;

			} else if (c == ' ' || c == '\t') {
				if (at_start_of_line) {
					continue; // Skip
				}

				if (!last_char_was_space) {
					data[write_idx++] = static_cast<std::byte>(' '); 
					last_char_was_space = true;
				}

			} else {
				data[write_idx++] = data[read_idx];
				at_start_of_line = false;
				last_char_was_space = false;
			}
		}
	}

	while (write_idx > 0) {
		char c = static_cast<char>(data[write_idx - 1]);
		if (std::isspace(static_cast<unsigned char>(c))) {
			write_idx--;
		} else {
			break;
		}
	}

	data.resize(write_idx);

	if (static_cast<char>(data[write_idx - 1]) != '\n') {
		data.push_back(static_cast<std::byte>('\n'));
	}
}

void Packer::stripEmptyLines(std::vector<std::byte>& data) {
	if (data.empty()) {
		return;
	}

	const std::byte* begin = data.data();
	const std::byte* end = data.data() + data.size();
	const std::byte* current = begin;

	std::vector<std::byte> temp_result_bytes;

	bool input_ends_with_newline = !data.empty() && 
									(static_cast<char>(data.back()) == '\n' || 
									static_cast<char>(data.back()) == '\r');

	bool first_line = true;

	while (current < end) {
		const std::byte* line_end = current;
		while (line_end < end && static_cast<char>(*line_end) != '\n') {
			line_end++;
		}
		
		const std::byte* line_content_end = line_end;
		if (line_content_end > current && static_cast<char>(*(line_content_end - 1)) == '\r') {
			line_content_end--;
		}

		size_t line_length = line_content_end - current;
		
		std::string current_line(
			reinterpret_cast<const char*>(current), 
			line_length
		);

		if (!trim(current_line).empty()) {
			if (!first_line) {
				temp_result_bytes.push_back(static_cast<std::byte>('\n'));
			}

			temp_result_bytes.insert(temp_result_bytes.end(), current, line_content_end);
			first_line = false;
		}

		if (line_end < end) {
			current = line_end + 1;
		} else {
			current = end;
		}
	}

	if (!temp_result_bytes.empty() && input_ends_with_newline && static_cast<char>(temp_result_bytes.back()) != '\n') {
		temp_result_bytes.push_back(static_cast<std::byte>('\n'));
	}

	data.swap(temp_result_bytes);
}