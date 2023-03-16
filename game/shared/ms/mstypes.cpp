#include "mstypes.h"

#undef msstring

//msstring type
msstring::msstring()
{
	m_cData[0] = 0;
}

msstring::msstring(const msstring_ref a) { operator=(a); }
msstring::msstring(const msstring_ref a, size_t length)
{
	strncpy(m_cData, a, length);
	m_cData[length] = 0;
}

msstring::msstring(const msstring &a) { operator=(a); }
msstring &msstring::operator=(const msstring_ref a)
{
	if (a == m_cData)
		return *this;
	m_cData[0] = 0;
	append(a);
	return *this;
}

msstring &msstring::operator=(int a)
{
	_snprintf(m_cData, MSSTRING_SIZE, "%i", a);
	return *this;
}

msstring &msstring::operator=(const msstring &a) { return operator=(a.m_cData); }
msstring &msstring::operator+=(const msstring_ref a)
{
	append(a);
	return *this;
}

msstring &msstring::operator+=(int a)
{
	msstring tmp;
	tmp = a;
	return operator+=((const msstring_ref &)(tmp));
}

msstring msstring::operator+(const msstring_ref a) { return msstring(m_cData) += a; }
msstring msstring::operator+(msstring &a) { return msstring(m_cData) += (const char *)a; }
msstring msstring::operator+(int a) { return msstring(m_cData) += a; }
bool msstring::operator==(char *a) const { return !strcmp(m_cData, a); }
bool msstring::operator==(const char *a) const { return !strcmp(m_cData, (const char *)a); }
msstring::operator char *() { return m_cData; }
char *msstring::c_str() { return m_cData; }
void msstring::append(const msstring_ref a, size_t length)
{
	size_t my_sz = len();
	size_t capped_sz = V_min(length, MSSTRING_MAXLEN - my_sz);
	if (capped_sz <= 0)
		return;
	strncpy(&m_cData[my_sz], a, capped_sz);
	m_cData[my_sz + capped_sz] = 0;
}

void msstring::append(const msstring_ref a)
{
	size_t len = strlen(a);
	append(a, len);
}

size_t msstring::len() const { return strlen(m_cData); }
size_t msstring::find(const msstring_ref a, size_t start) const
{
	msstring_ref substring = strstr(&m_cData[start], a);
	return substring ? (substring - &m_cData[start]) : msstring_error;
}

msstring_ref msstring::find_str(const msstring_ref a, size_t start) const
{
	size_t ret = find(a, start);
	return (ret != msstring_error) ? &m_cData[ret] : &m_cData[start];
}

size_t msstring::findchar(const msstring_ref a, size_t start) const
{
	for (int i = start; i < (signed)len(); i++)
	{
		char datachar[2] = {m_cData[i], 0};
		if (strstr(a, datachar))
			return i - start;
	}
	return msstring_error;
}

msstring_ref msstring::findchar_str(const msstring_ref a, size_t start) const
{
	size_t ret = findchar(a, start);
	return (ret != msstring_error) ? &m_cData[ret] : &m_cData[start];
}

bool msstring::contains(const msstring_ref a) const { return find(a) != msstring_error; }
bool msstring::starts_with(const msstring_ref a) const { return find(a) == 0; }
bool msstring::ends_with(const msstring_ref a) const
{
	msstring temp = a;
	int loc = len() - temp.len();
	return loc == find(temp);
}

msstring msstring::substr(size_t start, size_t length) { return msstring(&m_cData[start], length); }
msstring msstring::substr(size_t start) { return msstring(&m_cData[start]); }
msstring msstring::thru_substr(const msstring_ref a, size_t start) const
{
	size_t ret = find(a, start);
	return (ret != msstring_error) ? msstring(&m_cData[start], ret) : msstring(&m_cData[start]);
}

msstring msstring::thru_char(const msstring_ref a, size_t start) const
{
	size_t ret = findchar(a, start);
	return (ret != msstring_error) ? msstring(&m_cData[start], ret) : msstring(&m_cData[start]);
}

msstring msstring::skip(const msstring_ref a) const
{
	size_t my_sz = len();
	for (int i = 0; i < my_sz; i++)
	{
		char datachar[2] = {m_cData[i], 0};
		if (!strstr(a, datachar))
			return msstring(&m_cData[i], my_sz - i);
	}
	return &m_cData[my_sz];
}

msstring msstring::tolower(void) const
{
	size_t my_sz = len();
	msstring ret;
	for (int i = 0; i < my_sz; i++)
	{
		char ch = m_cData[i];
		ret += ::tolower(ch);
	}

	return ret;
}

//msvariant type
msvariant::msvariant() { clrmem(*this); }

void msvariant::SetFromString(msstring_ref a)
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