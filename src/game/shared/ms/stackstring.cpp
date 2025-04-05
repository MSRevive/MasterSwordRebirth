#include "Platform.h"
#include "stackstring.h"
#include "msdebug.h"

#undef msstring

msstring::msstring()
{
	data[0] = 0;
}
msstring::msstring(const char* a) { operator=(a); }
msstring::msstring(const char* a, size_t length)
{
	strncpy(data, a, length);
	data[length] = 0;
}

msstring::msstring(const msstring& a) { operator=(a); }

msstring &msstring::operator=(const char* a)
{
	if (a == data)
		return *this;
	data[0] = 0;
	append(a);
	return *this;
}
msstring &msstring::operator=(int a)
{
	_snprintf(data, MSSTRING_SIZE, "%i", a);
	return *this;
}
msstring &msstring::operator=(const msstring &a) { return operator=(a.data); }
msstring &msstring::operator+=(const char* a)
{
	append(a);
	return *this;
}
msstring &msstring::operator+=(msstring a)
{
	append(a.data);
	return *this;
}
msstring &msstring::operator+=(int a)
{
	msstring tmp;
	tmp = a;
	return operator+=(tmp);
}
msstring msstring::operator+(const char* a) { return msstring(data) += a; }
msstring msstring::operator+(const msstring &a) { return msstring(data) += a.data; }
msstring msstring::operator+(int a) { return msstring(data) += a; }
bool msstring::operator==(char *a) const { return !strcmp(data, a); }
bool msstring::operator==(const char *a) const { return !strcmp(data, a); }
bool msstring::operator==(msstring a) const { return !strcmp(data, a.data); }
msstring::operator char *() { return data; }
msstring::operator const char *() { return data; }
char *msstring::c_str() { return data; }
void msstring::append(const char* a, size_t length)
{
	size_t my_sz = len();
	size_t capped_sz = V_min(length, MSSTRING_MAXLEN - my_sz);
	if (capped_sz <= 0)
		return;
	strncpy(&data[my_sz], a, capped_sz);
	data[my_sz + capped_sz] = 0;
}
void msstring::append(const char* a)
{
	size_t len = strlen(a);
	append(a, len);
}
size_t msstring::len() const { return strlen(data); }
size_t msstring::find(const char* a, size_t start) const
{
	const char* substring = strstr(&data[start], a);
	return substring ? (substring - &data[start]) : msstring_error;
}
const char* msstring::find_str(const char* a, size_t start) const
{
	size_t ret = find(a, start);
	return (ret != msstring_error) ? &data[ret] : &data[start];
}
size_t msstring::findchar(const char* a, size_t start) const
{
	for (int i = start; i < (signed)len(); i++)
	{
		char datachar[2] = {data[i], 0};
		if (strstr(a, datachar))
			return i - start;
	}
	return msstring_error;
}
const char* msstring::findchar_str(const char* a, size_t start) const
{
	size_t ret = findchar(a, start);
	return (ret != msstring_error) ? &data[ret] : &data[start];
}
bool msstring::contains(const char* a) const { return find(a) != msstring_error; }
bool msstring::starts_with(const char* a) const { return find(a) == 0; }
bool msstring::ends_with(const char* a) const
{
	msstring temp = a;
	int loc = len() - temp.len();
	return loc == find(temp);
}
msstring msstring::substr(size_t start, size_t length) { return msstring(&data[start], length); }
msstring msstring::substr(size_t start) { return msstring(&data[start]); }
msstring msstring::thru_substr(const char* a, size_t start) const
{
	size_t ret = find(a, start);
	return (ret != msstring_error) ? msstring(&data[start], ret) : msstring(&data[start]);
}
msstring msstring::thru_char(const char* a, size_t start) const
{
	size_t ret = findchar(a, start);
	return (ret != msstring_error) ? msstring(&data[start], ret) : msstring(&data[start]);
}
msstring msstring::skip(const char* a) const
{
	size_t my_sz = len();
	for (int i = 0; i < my_sz; i++)
	{
		char datachar[2] = {data[i], 0};
		if (!strstr(a, datachar))
			return msstring(&data[i], my_sz - i);
	}
	return &data[my_sz];
}
msstring msstring::tolower(void) const
{
	size_t my_sz = len();
	msstring ret;
	for (int i = 0; i < my_sz; i++)
	{
		char ch = data[i];
		ret += ::tolower(ch);
	}

	return ret;
}

bool TokenizeString(const char *pszString, msstringlist &Tokens, const char* Separator)
{
	char cTemp[MSSTRING_SIZE - 1];
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

//
//	msvariant
//

msvariant::msvariant() { clrmem(*this); }

void msvariant::SetFromString(const char* a)
{
	m_String = a;
	m_Int = atoi(a);
	m_Float = (float)atof(a);
}

void msvariant::SetFromInt(int a)
{
	m_String = "";
	m_String += a;
	m_Int = a;
	m_Float = (float)a;
}

void msvariant::SetFromFloat(float a)
{
	_snprintf(m_String.c_str(), MSSTRING_SIZE, "%f", a);
	m_Int = (int)a;
	m_Float = a;
}

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
