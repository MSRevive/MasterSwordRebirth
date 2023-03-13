#pragma once

#include <cstring>
#include <string.h>
#include <string>
#include <sstream>
#include <map>
#include <set>
#include "platform.h"

//Deuplicated from msdebug.h
#ifdef DEV_BUILD
void *operator new(size_t size, const char *pszSourceFile, int LineNum);
void operator delete(void *ptr, const char *pszSourceFile, int LineNum);
#define msnew new (__FILE__, __LINE__)
#else
#define msnew new
#endif

typedef const char *msstring_ref;
#define msstring_error ((size_t)-1)
#define MSSTRING_SIZE 256
#define MSSTRING_MAXLEN 255

template <size_t sizeT = MSSTRING_SIZE>
class fixedstr
{
private:
	char m_cData[sizeT];
	int error = ((size_t)-1);

public:
	fixedstr() { m_cData[0] = 0; }
	fixedstr(const msstring_ref a) { operator=(a); }
	fixedstr(const msstring_ref a, size_t length)
	{
		strncpy(m_cData, a, length);
		m_cData[length] = 0;
	}
	fixedstr(const fixedstr &a) { operator=(a); }
	fixedstr &operator=(const msstring_ref a)
	{
		if (a == m_cData)
			return *this;
		m_cData[0] = 0;
		append(a);
		return *this;
	}

	fixedstr &operator=(int a) 
	{ 
		_snprintf(m_cData, sizeT, "%i", a); 
		return *this;
	}

	fixedstr &operator=(const fixedstr &a) { return operator=(a.m_cData); }
	fixedstr &operator+=(const msstring_ref a)
	{ 
		append(a); 
		return *this;
	}

	fixedstr &operator+=(int a)
	{
		fixedstr tmp;
		tmp = a;
		return operator+=((const msstring_ref &)(tmp));
	}

	fixedstr operator+(const msstring_ref a) { return fixedstr(m_cData) += a; };
	fixedstr operator+(fixedstr &a) { return fixedstr(m_cData) += (const char *)a; };
	fixedstr operator+(int a) { return fixedstr(m_cData) += a; };
	bool operator==(char *a) const { return !strcmp(m_cData, a); };
	bool operator==(const char *a) const { return !strcmp(m_cData, (const char *)a); };
	bool operator!=(char *a) const { return !operator==(a); }
	bool operator!=(const char *a) const { return !operator==(a); }
	bool operator<(char *a) const { return strcmp(m_cData, a) < 0; }
	bool operator<(const char *a) const { return strcmp(m_cData, a) < 0; }
	bool operator<(fixedstr& a) const { return strcmp(m_cData, a.m_cData) < 0; }
	bool operator<(const fixedstr& a) const { return strcmp(m_cData, a.m_cData) < 0; }
	operator char* () { return m_cData; }
	operator void* () { return operator char* (); }
	char *c_str() { return m_cData; }
	size_t len() const { return strlen(m_cData); }
	void append(const msstring_ref a)
	{
		size_t len = strlen(a);
		append(a, len);
	}

	void append(const msstring_ref a, size_t length)
	{
		size_t my_sz = len();
		size_t capped_sz = V_min(length, sizeT - my_sz);
		if (capped_sz <= 0)
			return;
		strncpy(&m_cData[my_sz], a, capped_sz);
		m_cData[my_sz + capped_sz] = 0;
	}

	//Returns position of the string "a"
	size_t find(const msstring_ref a, size_t start = 0) const
	{
		msstring_ref substring = strstr(&m_cData[start], a);
		return substring ? (substring - &m_cData[start]) : error;
	}

	//Returns a substring starting at "find(a,start)". Returns full string if "a" not found
	msstring_ref find_str(const msstring_ref a, size_t start = 0) const
	{
		size_t ret = find(a, start);
		return (ret != error) ? &m_cData[ret] : &m_cData[start];
	}

	//Returns position of the first char within "a"
	size_t findchar(const msstring_ref a, size_t start = 0) const
	{
		for (int i = start; i < (signed)len(); i++)
		{
			char datachar[2] = {m_cData[i], 0};
			if (strstr(a, datachar))
				return i - start;
		}
		return error;
	}

	//Returns a substring starting at "findchar(a,start)". Returns full string if text didn't contain any chars from "a"
	msstring_ref findchar_str(const msstring_ref a, size_t start = 0) const
	{
		size_t ret = findchar(a, start);
		return (ret != error) ? &m_cData[ret] : &m_cData[start];
	}

	//Returns true if substring "a" is contained within the main string
	bool contains(const msstring_ref a) const { return find(a) != error; }
	//Returns true if the main string starts with "a"
	bool starts_with(const msstring_ref a) const { return find(a) == 0; }

	//MIB FEB2008a returns true if last character is "a"
	bool ends_with(const msstring_ref a) const
	{
		fixedstr<sizeT> temp = a;
		int loc = len() - temp.len();
		return loc == find(temp);
	}
	fixedstr substr(size_t start, size_t length) { return fixedstr<sizeT>(&m_cData[start], length); }
	fixedstr substr(size_t start) { return fixedstr<sizeT>(&m_cData[start]); }

	//Returns a substring spanning from "start" to "find(a,start)". Returns full string if "find(a,start)" not found
	fixedstr thru_substr(const msstring_ref a, size_t start = 0) const
	{
		size_t ret = find(a, start);
		return (ret != error) ? fixedstr<sizeT>(&m_cData[start], ret) : fixedstr<sizeT>(&m_cData[start]);
	}

	//Returns a substring spanning from "start" to "findchar(a,start)". Returns full string if "findchar(a,start)" not found
	fixedstr thru_char(const msstring_ref a, size_t start = 0) const
	{
		size_t ret = findchar(a, start);
		return (ret != error) ? fixedstr<sizeT>(&m_cData[start], ret) : fixedstr<sizeT>(&m_cData[start]);
	}

	//Returns a substring starting at the first char that isn't within "a"
	fixedstr skip(const msstring_ref a) const
	{
		size_t my_sz = len();
		for (int i = 0; i < my_sz; i++)
		{
			char datachar[2] = {m_cData[i], 0};
			if (!strstr(a, datachar))
				return fixedstr<sizeT>(&m_cData[i], my_sz - i);
		}
		return &m_cData[my_sz];
	}

	fixedstr tolower(void) const
	{
		size_t my_sz = len();
		fixedstr<sizeT> ret;
		for (int i = 0; i < my_sz; i++)
		{
			char ch = m_cData[i];
			ret += ::tolower(ch);
		}

		return ret;
	}
};

//It's an int, a float, and a string
class msvariant
{
public:
	fixedstr<256> m_String;
	int m_Int;
	float m_Float;
	enum type
	{
		INT,
		FLOAT,
		STRING
	} m_Type;

	msvariant() { clrmem(*this); }
	msvariant(int a) { operator=(a); }
	msvariant(float a) { operator=(a); }
	msvariant(msstring_ref a) { operator=(a); }
	msvariant &operator=(msstring_ref a)
	{
		SetFromString(a);
		m_Type = STRING;
		return *this;
	}
	msvariant &operator=(int a)
	{
		SetFromInt(a);
		m_Type = INT;
		return *this;
	}
	msvariant &operator=(float a)
	{
		SetFromFloat(a);
		m_Type = STRING;
		return *this;
	}

	void SetFromString(msstring_ref a)
	{
		m_String = a;
		m_Int = atoi(a);
		m_Float = (float)atof(a);
	}

	void SetFromInt(int a) 
	{
		m_String = "";
		m_String += a;
		m_Int = a;
		m_Float = (float)a;
	}
	void SetFromFloat(float a)
	{
		_snprintf(m_String.c_str(), 256, "%f", a);
		m_Int = (int)a;
		m_Float = a;
	}
	void SetType(type Type) { m_Type = Type; }

	operator int() { return m_Int; }
	operator float() { return m_Float; }
	operator msstring_ref() { return m_String.c_str(); }
};

//mslist - List of items of type itemtype_y
//-Dog
template <class itemtype_y>
class mslist
{
private:
	itemtype_y *m_First;
	size_t m_Items;
	size_t m_ItemsAllocated;

public:
	mslist()
	{
		m_Items = 0;
		m_ItemsAllocated = 0;
		m_First = (itemtype_y *)0;
	}

	mslist(bool NoInit) {} //Special case - no initialization

	~mslist()
	{
		clear();
	}

	inline itemtype_y &push_back(const itemtype_y &Item) { return add(Item); }

	itemtype_y &add(const itemtype_y &Item)
	{
		reserve(m_Items + 1);

		int idx = m_Items;
		m_First[idx] = Item;

		m_Items++;

		return m_First[idx];
	}

	itemtype_y &add_blank() //Add an entry that's completely zero'd out
	{
		reserve(m_Items + 1);

		int idx = m_Items;
		memset(&m_First[idx], 0, sizeof(itemtype_y));

		m_Items++;

		return m_First[idx];
	}

	itemtype_y &operator[](const int idx) const
	{
		return m_First[idx];
	}

	void erase(const size_t idx)
	{
		if (idx + 1 < m_Items)
			memmove(&m_First[idx], &m_First[idx + 1], (m_Items - (idx + 1)) * sizeof(itemtype_y));

		m_Items--;
	}

	void clear()
	{
		unalloc();
		m_Items = m_ItemsAllocated = 0;
		m_First = (itemtype_y *)0;
	}

	void clearitems() //No slow dealloc.  Just empty out the items
	{
		m_Items = 0;
	}

	size_t size() const
	{
		return m_Items;
	}

	void reserve()
	{
		if (!m_ItemsAllocated)
			m_ItemsAllocated++;
		else
			m_ItemsAllocated *= 2;
		itemtype_y *pNewItems = ::msnew itemtype_y[m_ItemsAllocated];
		for (unsigned int i = 0; i < m_Items; i++)
			pNewItems[i] = m_First[i];
		unalloc();
		m_First = pNewItems;
	}

	void unalloc()
	{
		if (m_First)
			delete[] m_First;
		m_First = (itemtype_y *)0;
	}

	void reserve(size_t Items)
	{
		while (m_ItemsAllocated < Items)
			reserve();
	}

	void reserve_once(size_t ReserveItems, size_t Items) //Special case - only use if you know what you're doing
	{
		unalloc();
		m_ItemsAllocated = ReserveItems;
		m_First = msnew itemtype_y[m_ItemsAllocated];
		m_Items = Items;
	}

	mslist &operator=(const mslist &OtherList)
	{
		if (size() != OtherList.size())
		{
			clear();
			for (unsigned int i = 0; i < OtherList.size(); i++)
				add(OtherList[i]);
		}
		else //Don't re-allocate if I don't have to
			for (unsigned int i = 0; i < OtherList.size(); i++)
				m_First[i] = OtherList[i];

		return *this;
	}

	itemtype_y *FirstItem()
	{
		return m_First;
	}
};

typedef mslist<fixedstr<>> msstringlist;
typedef std::map<fixedstr<>,fixedstr<>> msstringstringhash;
typedef fixedstr<> msstring;