#include "strutil.h"
#include "mstypes.h"

bool TokenizeString(const char *pszString, msstringlist &Tokens, msstring_ref Separator)
{
	char cTemp[256 - 1];
	int i = 0;
	bool AnyFound = false;
	msstring ParseStr = "%[^";
	ParseStr += Separator;
	ParseStr += "]";
	while (sscanf(&pszString[i], ParseStr, cTemp) > 0)
	{
		i += strlen(cTemp);
		Tokens.add(cTemp);
		AnyFound = true;

		if (pszString[i])
			i++; //Hit a semi-colon, continue
	}
	return AnyFound;
}

void ReplaceChar(char *pString, char org, char dest)
{
	int i = 0;
	while (pString[i])
	{
		if (pString[i] == org)
			pString[i] = dest;
		i++;
	}
}

mslist<std::string> strutil::explode(std::string const &str, char delim)
{
	mslist<std::string> result;
	std::istringstream iss(str);

	for (std::string token; std::getline(iss, token, delim); )
	{	
		if (!token.empty())
			result.push_back(std::move(token));
	}

	return result;
}

std::string& strutil::implode(mslist<std::string> vec, int start)
{
	static std::string result;
	result.clear();

	for (int i = start; i < vec.size(); i++)
		result += vec[i];

	return result;
}

// std::string& strutil::removeWhiteSpace(std::string &str)
// {
// 	str.erase(std::unique(std::begin(str), std::end(str), [](unsigned char a, unsigned char b){
// 		return isSpace(a) && isSpace(b);
// 	}), std::end(str));

// 	return str;
// }

bool strutil::isSpace(const char &ch)
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

void strutil::tolower(char* str)
{
	char* target = str;
	while (*target != '\0')
	{
		(*target) = ::tolower(*target);
		target++;
	}
}

bool strutil::isBadChar(int c)
{
	return (c == '(' || c == ')' || c == '$' || c == '¯');
}

char* strutil::stripBadChars(char* data)
{
	int i = 0, x = 0;
	char c;
	char* cleanData = data;

	while ((c = data[i++]) != '\0')
	{
		if (!isBadChar(c))
			cleanData[x++] = c;
	}

	cleanData[x] = '\0';
	return cleanData;
}

bool strutil::isBadStr(char* str)
{
	char* target = str;
	while (*target != '\0')
	{
		if (isBadChar(*target))
			return true;
		target++;
	}
	return false;
}