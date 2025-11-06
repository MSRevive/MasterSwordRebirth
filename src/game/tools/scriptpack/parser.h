#pragma once

// Include scriptmodule addon for module preprocessing
#ifdef _MSR_UTILS
#include "scriptmodule/scriptmodule.h"
#endif

class Parser
{
public:
	Parser(const char* data, const char* file);
	void stripComments();
	void stripWhiteSpace();
	void stripDebug();
	void checkQuotes();
	// Preprocess module syntax using scriptmodule addon
	void preprocessModules();
	void checkBrackets();
	//false if script passes, true if script fails.
	bool errorCheck();
	std::string getResult();
	std::vector<std::string> getErrorlist();
	void printErrors();
	void saveErrors();
	void saveResult(const char* create);

private:
	enum class State : char
	{
		InDoubleQuote,
		InSingleQuote,
		InPara,
		NoQuote,
	};

	//we have to use our own getline because of the mixed line endings.
	//credits to https://gist.github.com/josephwb/df09e3a71679461fc104
	std::istream& getline(std::istream& is, std::string &t);
	bool onlySpace(const std::string& str);
	static bool isSpace(const char& ch);
	void addError(const char* fmt, size_t lineNum, size_t pos);
	void quoteError(size_t line, size_t pos);

	//create directory recursively for scripts
	//modified from this https://gist.github.com/danzek/d7192d250c951804dec05125f5223a30
	//void createDirectoryRecursively(std::string& path);

	std::string m_Result{};
	std::string m_Data;
	const char* m_FileName;
	std::vector<std::string> m_ErrorList{};
};