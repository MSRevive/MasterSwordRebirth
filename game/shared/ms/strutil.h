#pragma once

#include <string>
#include "mstypes.h"

bool TokenizeString(msstring_ref pszString, msstringlist &Tokens, msstring_ref Separator);
inline bool TokenizeString(msstring_ref pszString, msstringlist &Tokens) { return TokenizeString(pszString, Tokens, ";"); }
void ReplaceChar(char *pString, char org, char dest);

namespace strutil {
	//this type has to be a mslist 'cause thanks msc.
	mslist<std::string> explode(std::string const &str, char delim);
	std::string& implode(mslist<std::string> vec, int start = 0);
	//std::string& removeWhiteSpace(std::string &str);
	bool isSpace(const char &ch);
	void tolower(char *str);
	bool isBadChar(int c);
	char* stripBadChars(char *data);
	bool isBadStr(char *str);
}