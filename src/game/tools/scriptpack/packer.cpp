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

Packer::Packer(const char* wDir, const char* rDir, const char* oDir)
{
	_snprintf(m_WorkDir, MAX_PATH, "%s", wDir);
	_snprintf(m_RootDir, MAX_PATH, "%s", rDir);
	_snprintf(m_OutDir, MAX_PATH, "%s", oDir);
	_snprintf(m_CookedDir, MAX_PATH, "%s\\cooked", rDir);

	std::cout << "Work directory set to: " << m_WorkDir << std::endl;
	std::cout << "Cooked directory set to: " << m_CookedDir << std::endl;

	if (g_Release)
	{
		std::filesystem::path fsPath = m_CookedDir;
		std::error_code ec;
		std::filesystem::create_directories(fsPath, ec);
		if (ec) {
			std::cout << "ERROR: creating directories for " << fsPath << ": " << ec.message() << std::endl;
			exit(-1);
		}
	}
}

void Packer::readDirectory(const char *pszName, bool cooked)
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

				if(cooked)
					m_CookedFiles.push_back(file.string());
				else
					m_StoredFiles.push_back(file.string());
			}
		}
	}
}

//checks the scripts for errors and cleans them for release.
void Packer::catalogScripts() 
{
	printf("size: %d\n", m_StoredFiles.size());

	if(m_StoredFiles.size() > 0)
	{
		for(std::string &file : m_StoredFiles)
		{
			auto contents = getFileContents(file);

			if(contents.size() > 0)
			{
				std::cout << file << std::endl;
				std::string relativePath = file.substr(strlen(m_WorkDir) + 1, file.length());
				std::cout << relativePath << std::endl;

				if (g_Verbose)
					std::cout << "Cataloging script: " << relativePath << std::endl;
				
				if (g_Release)
				{
					std::string cookedFile = m_CookedDir;
					cookedFile += "/";
					cookedFile += relativePath;

					// std::ofstream o;
					// o.open(cookedFile, std::ios::out | std::ios::binary);
					// o.write(reinterpret_cast<const char*>(contents.data()), contents.size());
					// o.close();

					//reinterpret_cast<const char*>(contents.data())

					std::thread parserThread(&Packer::processScript, this, contents, relativePath, cookedFile, false);
					parserThread.join();
				}
				else
				{
					std::thread parserThread(&Packer::processScript, this, contents, relativePath, file, false);
					parserThread.join();
				}
			}
		}

		if (g_Release)
		{
			readDirectory(m_CookedDir, true);
		}
	}
}

// packs the scripts.
// we have to use c strings here because MSR does.
void Packer::packScripts()
{
	char cWriteFile[MAX_PATH];
	_snprintf(cWriteFile, MAX_PATH, "%s\\scripts.pak", m_OutDir);

	if(std::filesystem::exists(cWriteFile))
		std::remove(cWriteFile);

	FILE* fp = fopen(cWriteFile, "wb+");

	if (fp == NULL)
	{
		printf("Failed to create %s\n", cWriteFile);
		exit(-1);
	}

	std::vector<std::string> files;

	if (g_Release)
	{
		files = m_CookedFiles;
	}
	else
	{
		files = m_StoredFiles;
	}

	size_t baseDirLen = strlen(m_WorkDir)-1;
	size_t listSize = files.size();

	pakHeader_t Header;
	Header.MagicNumber = 1262698832;
	Header.DirectoryOffset = sizeof(pakHeader_t);
	Header.DirectoryCount = listSize;

	pakDirectory_t dummy;
	strcpy(dummy.cFilename, "");
	dummy.FileSize = 0;
	dummy.FileOffset = 0;

	// write the file header
	fwrite(&Header, sizeof(pakHeader_t), 1, fp);

	// write out dummy data to occupy the file metadata position
	for (int i = 0; i < listSize; i++)
		fwrite(&dummy, sizeof(pakDirectory_t), 1, fp);

	size_t currentFileBytesWritten = ftell(fp);

	// jump back to just after the header
	fseek(fp, sizeof(pakHeader_t), SEEK_SET);
	
	if (files.size() > 0 && baseDirLen > 0)
	{
		for(std::string &file : files)
		{
			auto contents = getFileContents(file);

			if (contents.size() > 0)
			{
				std::cout << file << std::endl;
				std::string relativePath = file.substr(baseDirLen, file.length());
				std::cout << relativePath << std::endl;

				pakDirectory_t File;
				strncpy(File.cFilename, &(file.c_str()[baseDirLen]), sizeof(File.cFilename)); // was strlen(m_WorkDir)+1 but that's not the correct index anymore?
				File.FileOffset = 0;
				File.FileSize = contents.size();

				if (g_Verbose == true)
					printf("Packing file: %s\n", File.cFilename);

				// remember where this entry is
				size_t currentPosition = ftell(fp);

				// write this entry
				size_t ObjectsWritten = fwrite(&File, sizeof(pakDirectory_t), 1, fp);

				if (ObjectsWritten != 1)
					printf("Failed to write entry: %s\n", File.cFilename);

				// jump to where the file data should be
				fseek(fp, currentFileBytesWritten, SEEK_SET);

				// remember the current position as where this file starts
				File.FileOffset = ftell(fp);

				// write the file data
				ObjectsWritten = fwrite(reinterpret_cast<const char*>(contents.data()), contents.size(), 1, fp);

				if (ObjectsWritten != 1)
					printf("Failed to write file: %s\n", File.cFilename);

				// remember the current position as where to pick up from when writing the next file
				currentFileBytesWritten = ftell(fp);

				// jump back to the entry
				fseek(fp, currentPosition, SEEK_SET);

				// write the entry with the correct FileOffset
				ObjectsWritten = fwrite(&File, sizeof(pakDirectory_t), 1, fp);
		
				if (ObjectsWritten != 1)
					printf("Failed to write entry: %s\n", File.cFilename);
			}
		}
	}
	else
	{
		std::cout << "ERROR: No files in list!" << std::endl;
		exit(-1);
	}

	fflush(fp);
	fclose(fp);
}

void Packer::processScript(std::vector<std::byte> buffer, std::string relativeFile, std::string createFile, bool errOnly)
{
	if (relativeFile == "items.txt")
	{
		std::ofstream o;
		o.open(createFile, std::ios::out | std::ios::binary | std::ios::trunc);
		o.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
		o.close();
		return;
	}

	if (relativeFile.find(".as")) 
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

			if (moduleProcessor.PreprocessModuleSource(norm_str, processedSource, moduleName))
			{
				norm_str = processedSource;
				std::cout << "Preprocessed module: " << moduleName << " in " << relativeFile << std::endl;
			}
			else
			{
				std::cout << "Note: File contains 'module' keyword but is not a valid module: " << relativeFile << std::endl;
			}
		}
#endif
	}
	else 
	{
		stripComments(buffer);
		stripWhiteSpace(buffer);
		stripEmptyLines(buffer);
	}

	


	// if (relativeFile == "items.txt" && !errOnly)
	// {
	// 	Parser parser(buffer, relativeFile);
	// 	parser.saveResult(createFile);
	// }
	// else
	// {
	// 	//we create parser object.
	// 	Parser parser(buffer, relativeFile);
	// 	parser.saveResult(createFile);
	// 	//parser.stripComments();

	// 	// // Preprocess module syntax before error checking
	// 	// parser.preprocessModules();

	// 	// //we check for errors here because comments were already replaced.
	// 	// parser.checkQuotes(); //check for quote errors
	// 	// //parser.checkBrackets(); //check for closing errors

	// 	// //only run this stuff if we're doing full parser.
	// 	// if (!errOnly)
	// 	// {
	// 	// 	parser.stripWhiteSpace();
	// 	// 	parser.stripDebug();
	// 	// }

	// 	// //do error print at the end
	// 	// parser.printErrors();
	// 	// if (g_ErrFile)
	// 	// 	parser.saveErrors();

	// 	// if (!errOnly)
	// 	// 	parser.saveResult(createFile);

	// 	// if (g_FailOnErr && parser.errorCheck())
	// 	// {
	// 	// 	//delete[] ffile;
	// 	// 	exit(-1);
	// 	// }
	// }

	std::filesystem::path fsFile = createFile;
	std::filesystem::path fsPath = fsFile.parent_path();

	std::error_code ec;
	std::filesystem::create_directories(fsPath, ec);
	if (ec) {
		std::cout << "ERROR: creating directories for " << fsPath << ": " << ec.message() << std::endl;
	}

	std::ofstream o;
	o.open(createFile, std::ios::out | std::ios::binary | std::ios::trunc);
	o.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
	o.close();
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