#include "parser.h"

#include <sstream>
#include <regex>
#include <ctype.h>
#include <stack>
#include <sys/stat.h>
#include <fstream>
#include <iostream>

#include "cbase.h"

Parser::Parser(const char *data, std::string file) {
	m_FileName = file;
	//m_Result = m_Data;
}

void Parser::stripComments()
{
	std::istringstream ss(m_Result);
	std::string line;
	std::string newRes;

	//we have to build a new result.
	while(getline(ss, line))
	{
		std::string nLine = "";
		size_t lineLen = line.length();

		for (size_t i = 0; i < lineLen; i++)
		{
			const char ch = line[i];
			const char nextch = line[i+1]; //get next ch.

			//remove comments.
			if (ch == '/' && nextch == '/')
				break;

			//just remove return carriages here instead of doing erase.
			if (ch == '\r')
				break;

			nLine += ch;
		}

		newRes += nLine;
		newRes += "\n";
	}

	m_Result = newRes;
}

/*
//credits to https://codereview.stackexchange.com/a/215913
void Parser::stripComments()
{
	State cState = State::NotAComment;
	std::string res;

	for (size_t i = 0; i < m_Result.length(); i++)
	{
		const char ch = m_Result[i];
		switch (cState)
		{
		case State::SlashOC:
			if (ch == '/')
			cState = State::SingleLineComment;
			else
			{
			// ?????
			cState = State::NotAComment;
			res += ch;
			}
			break;
		case State::SingleLineComment:
			if (ch == '\n')
			{
			cState = State::NotAComment;
			res += '\n';
			}
			break;
		case State::NotAComment:
			if (ch == '/')
			cState = State::SlashOC;
			else
			res += ch;

			break;
		}
	}

	m_Result = res;
}*/


void Parser::stripWhiteSpace() {
	std::istringstream ss(m_Result);
	std::string line;
	std::string newRes;

	//we have to build a new result.
	while(getline(ss, line))
	{
		//remove tabs at beginning of line.
		line.erase(0, line.find_first_not_of(" \t\v"));

		//skip over empty lines.
		if (line.find_first_not_of(" \r\t") != std::string::npos)
		{
			//remove extra spaces
			//credit to https://stackoverflow.com/questions/35301432/remove-extra-white-spaces-in-c
			line.erase(std::unique(std::begin(line), std::end(line), [](unsigned char a, unsigned char b){
				return isSpace(a) && isSpace(b);
			}), std::end(line));

			newRes += line;
			newRes += "\n";
		}
	}

	m_Result = newRes;
}

void Parser::stripDebug() {
	std::istringstream ss(m_Result);
	std::string line;
	std::string newRes;

	while(getline(ss, line))
	{
		if (line.substr(0, 3) != "dbg")
		{
			newRes += line;
			newRes += "\n";
		}
	}

	m_Result = newRes;
}

void Parser::checkQuotes() {
	State cState = State::NoQuote;
	std::istringstream ss(m_Result);
	std::string line;
	size_t lineNum = 1;

	while (getline(ss, line)) 
	{	
		//don't process if line is empty
		size_t lineLen = line.length();
		if (!line.empty() && !onlySpace(line))
		{
			for (size_t pos = 0; pos < lineLen; pos++)
			{
				const char ch = line[pos];
				
				switch(cState)
				{
				case State::InDoubleQuote:
					if (ch == '"')
						cState = State::NoQuote;
					else if (pos == (line.length()))
					{
						cState = State::NoQuote;
						quoteError(lineNum, pos);
					}
					break;
				case State::InSingleQuote:
					if (ch == '\'')
						cState = State::NoQuote;
					else if (pos == (line.length()))
					{
						cState = State::NoQuote;
						quoteError(lineNum, pos);
					}
					break;
				case State::InPara:
					if (ch == ')')
						cState = State::NoQuote;
					else if (pos == (line.length()))
					{
						cState = State::NoQuote;
						addError("%s:%u.%u: expected closing parentheses", lineNum, pos);
					}
					break;
				case State::NoQuote:
					if (ch == '"')
						cState = State::InDoubleQuote;
					else if (ch == '\'')
						cState = State::InSingleQuote;
					else if (ch == '(')
						cState = State::InPara;
					break;
				}
			}
		}

		lineNum++;
	}
}

void Parser::preprocessModules() {
#ifdef _MSR_UTILS
	// Check if this file contains module syntax
	if (m_Result.find("module ") != std::string::npos)
	{
		// Use the scriptmodule addon to preprocess the file
		CScriptModule moduleProcessor;
		std::string moduleName;
		std::string processedSource;
		
		if (moduleProcessor.PreprocessModuleSource(m_Result, processedSource, moduleName))
		{
			m_Result = processedSource;
			std::cout << "Preprocessed module: " << moduleName << " in " << m_FileName << std::endl;
		}
		else
		{
			std::cout << "Note: File contains 'module' keyword but is not a valid module: " << m_FileName << std::endl;
		}
	}
#endif
}

void Parser::checkBrackets() {
	std::istringstream ss(m_Result);
	std::string line;
	std::vector<std::pair<size_t, size_t>> openBrace{};
	size_t lineNum = 1;

	while (getline(ss, line))
	{
		size_t lineLen = line.length();
		for(size_t pos = 0; pos < lineLen; pos++)
		{
			switch(line[pos])
			{
			case '{':
				openBrace.push_back(std::make_pair(lineNum, pos));
				break;
			case '}':
				if (openBrace.size() > 0) //don't try to pop empty vector, bad things happen...
					openBrace.pop_back();
				break;
			}
		}

		lineNum++;
	}

	for (std::pair<size_t, size_t> ob : openBrace)
		addError("%s:%u.%u: brace opened, but never closed", ob.first, ob.second);
}

bool Parser::errorCheck()
{
	if (m_ErrorList.size() > 0)
		return true;
	else
		return false;
}

std::string Parser::getResult()
{
	return m_Result;
}

std::vector<std::string> Parser::getErrorlist()
{
	return m_ErrorList;
}

void Parser::printErrors()
{
	if (m_ErrorList.size() == 0)
		return;

	std::cout << m_ErrorList.size() << " errors found" << std::endl;
	for (auto i: m_ErrorList)
		std::cout << '\t' << i << std::endl;
}

void Parser::saveErrors()
{
	std::ofstream o{};
	o.open("./errors.txt", std::ios_base::app | std::ios_base::out);

	for (auto i: m_ErrorList)
		o << i << std::endl;
		
	o.close();
}

void Parser::saveResult(std::string file)
{
	std::filesystem::path fsFile = file;
	std::filesystem::path fsPath = fsFile.parent_path();

	std::error_code ec;
	std::filesystem::create_directories(fsPath, ec);
	if (ec) {
		std::cout << "ERROR: creating directories for " << fsPath << ": " << ec.message() << std::endl;
	}

	std::ofstream o;
	o.open(file, std::ios_base::binary | std::ios_base::trunc);
	o << m_Result;
	o.close();
}

//we have to use our own getline because of the mixed line endings.
//credits to https://gist.github.com/josephwb/df09e3a71679461fc104
std::istream& Parser::getline(std::istream &is, std::string &t) { 
	t.clear();

	// The characters in the stream are read one-by-one using a std::streambuf.
	// That is faster than reading them one-by-one using the std::istream.
	// Code that uses streambuf this way must be guarded by a sentry object.
	// The sentry object performs various tasks,
	// such as thread synchronization and updating the stream state.

	std::istream::sentry se(is, true);
	std::streambuf* sb = is.rdbuf();

	for (;;) {
		int c = sb->sbumpc();
		switch (c) {
			case '\n':
				return is;
			case '\r':
				if (sb->sgetc() == '\n') {
					sb->sbumpc();
				}
				return is;
			case EOF:
				// Also handle the case when the last line has no line ending
				if (t.empty()) {
					is.setstate(std::ios::eofbit);
				}
				return is;
			default:
				t += (char)c;
		}
	}
}

bool Parser::onlySpace(const std::string &str)
{
	return std::all_of(str.begin(), str.end(), isspace);
}

bool Parser::isSpace(const char &ch)
{
	switch(ch)
	{
	case ' ':
		return true;
	case '\t':
		return true;
	case '\v':
		return true;
	default:
		return false;
	}
}

void Parser::addError(std::string_view fmt, size_t lineNum, size_t pos)
{
	std::string_view fmtt(fmt);
	std::string str = std::vformat(fmtt, std::make_format_args(m_FileName, lineNum, pos));
	m_ErrorList.push_back(str);
}

void Parser::quoteError(size_t line, size_t pos)
{
	addError("%s:%u.%u: unclosed quotation", line, pos);
}

//create directory recursively for scripts
//modified from this https://gist.github.com/danzek/d7192d250c951804dec05125f5223a30
/*
void Parser::createDirectoryRecursively(std::string &path)
{
	static const std::string separators("\\/");

	struct stat info;
	if(stat(path.c_str(), &info) != 0)
	{
		// Recursively do it all again for the parent directory, if any
		size_t slashIndex = path.find_last_of(separators);
		if(slashIndex != std::string::npos) 
		{
			std::string parentPath = path.substr(0, slashIndex);
			createDirectoryRecursively(parentPath);
		}

		// Create the last directory on the path (the recursive calls will have taken
		// care of the parent directories by now)
		int result = CreateDirectory(path.c_str(), NULL); // windows API use BOOL which is a int.
		if(result == 0)
		{
			std::cout << path.c_str() << std::endl;
			std::cout << "ERROR: Could not create directory!" << std::endl;
			exit(-1);
		}
	}
}*/