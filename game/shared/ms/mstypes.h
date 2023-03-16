#pragma once

#include <sstream>
#include <map>
#include <set>
#include <list>
#include <iterator>
#include <alloc/alloc.h>
#include "platform.h"

#define clrmem(a) memset(&a, 0, sizeof(a));

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

class msstring
{
private:
	char m_cData[MSSTRING_SIZE];
	int error = ((size_t)-1);

public:
	msstring() = default;
	msstring(const msstring_ref a);
	msstring(const msstring_ref a, size_t length);
	msstring(const msstring &a);
	msstring &operator=(const msstring_ref a);
	msstring &operator=(int a);
	msstring &operator=(const msstring &a);
	msstring &operator+=(const msstring_ref a);
	msstring &operator+=(int a);
	msstring operator+(const msstring_ref a);
	msstring operator+(msstring &a);
	msstring operator+(int a);
	bool operator==(char *a) const;
	bool operator==(const char *a) const;
	bool operator!=(char *a) const { return !operator==(a); }
	bool operator!=(const char *a) const { return !operator==(a); }
	bool operator<(char *a) const { return strcmp(m_cData, a) < 0; }
	bool operator<(const char *a) const { return strcmp(m_cData, a) < 0; }
	bool operator<(msstring& a) const { return strcmp(m_cData, a.m_cData) < 0; }
	bool operator<(const msstring& a) const { return strcmp(m_cData, a.m_cData) < 0; }
	operator char* ();
	operator void* () { return operator char* (); }
	char *c_str();
	size_t len() const;
	void append(const msstring_ref a);
	void append(const msstring_ref a, size_t length);

	//Returns position of the string "a"
	size_t find(const msstring_ref a, size_t start = 0) const;

	//Returns a substring starting at "find(a,start)". Returns full string if "a" not found
	msstring_ref find_str(const msstring_ref a, size_t start = 0) const;

	//Returns position of the first char within "a"
	size_t findchar(const msstring_ref a, size_t start = 0) const;

	//Returns a substring starting at "findchar(a,start)". Returns full string if text didn't contain any chars from "a"
	msstring_ref findchar_str(const msstring_ref a, size_t start = 0) const;

	//Returns true if substring "a" is contained within the main string
	bool contains(const msstring_ref a) const;
	//Returns true if the main string starts with "a"
	bool starts_with(const msstring_ref a) const;

	//MIB FEB2008a returns true if last character is "a"
	bool ends_with(const msstring_ref a) const;
	msstring substr(size_t start, size_t length);
	msstring substr(size_t start);

	//Returns a substring spanning from "start" to "find(a,start)". Returns full string if "find(a,start)" not found
	msstring thru_substr(const msstring_ref a, size_t start = 0) const;

	//Returns a substring spanning from "start" to "findchar(a,start)". Returns full string if "findchar(a,start)" not found
	msstring thru_char(const msstring_ref a, size_t start = 0) const;

	//Returns a substring starting at the first char that isn't within "a"
	msstring skip(const msstring_ref a) const;

	msstring tolower(void) const;
};

//a fixed string with the same features as std::string
template <size_t sizeT>
class fixedstr : public std::basic_string<char, std::char_traits<char>, fixed_size_allocator<char, sizeT>>
{
public:
	typedef std::basic_string<char, std::char_traits<char>, fixed_size_allocator<char, sizeT>> basestr;

	fixedstr() : basestr() {  }
	fixedstr(const char *str) : basestr(str) {  }
	fixedstr(const char *str, size_t n) : basestr(str, n) {  }
	fixedstr(size_t n, char c) : basestr(n, c) {  }

	bool contains(const char *str)
	{
		if (find(str) != std::basic_string::npos)
			return true;

		return false;
	}

	fixedstr tolower()
	{
		fixedstr str;
		std::transform(begin(), end(), begin(), [](unsigned char c){ return std::tolower(c); });
		return str;
	}

	fixedstr toupper()
	{
		fixedstr str;
		std::transform(begin(), end(), begin(), [](unsigned char c){ return std::toupper(c); });
		return str;
	}

	// fixedstr &operator=(const char *str) { return fixedstr(str); }
	// fixedstr &operator+(const char *str) { append(str); *this; }
	// fixedstr &operator+(int n) { append(n); *this; }
	// fixedstr &operator+(char ch) { append(ch); *this; }
	// fixedstr &operator+=(const char *str) { append(str); *this; }
	// fixedstr &operator+=(int n) { append(n); *this; }
	// fixedstr &operator+=(char ch) { append(ch); *this; }
	operator const char* () { return c_str(); }
};

//It's an int, a float, and a string
class msvariant
{
public:
	fixedstr<256> m_String = "";
	int m_Int;
	float m_Float;
	enum type
	{
		INT,
		FLOAT,
		STRING
	} m_Type;

	msvariant();
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

	void SetFromString(msstring_ref a);
	void SetFromInt(int a);
	void SetFromFloat(float a);
	void SetType(type Type) { m_Type = Type; }

	operator int() { return m_Int; }
	operator float() { return m_Float; }
	operator msstring_ref() { return m_String.c_str(); }
};

//wrapper for linked list
//we're too lazy to replace mslist in the entire project.
// template <typename itemtype_y>
// class mslist
// {
// public:
// 	std::list<itemtype_y> m_List;

// public:
// 	mslist() = default;
// 	~mslist() = default;

// 	inline itemtype_y &push_back(const itemtype_y &Item)
// 	{
// 		m_List.push_back(Item);
// 		return m_List.back();
// 	}

// 	void insert(size_t idx, const itemtype_y &Item)
// 	{
// 		auto l_front = m_List.begin();
// 		auto nx = std::next(l_front, idx);
// 		m_List.insert(nx, Item);
// 	}

// 	itemtype_y &add(const itemtype_y &Item)
// 	{
// 		return push_back(Item);
// 	}

// 	itemtype_y &add_blank()
// 	{
// 		return push_back(itemtype_y());
// 	}

// 	auto operator[](const size_t idx) const
// 	{
// 		auto l_front = m_List.begin();
// 		auto nx = std::next(l_front, idx);
// 		return *nx;
// 	}

// 	auto operator=(const mslist &OtherList)
// 	{
// 		if (size() != OtherList.size())
// 		{
// 			clear();
// 			for (size_t i = 0; i < OtherList.size(); i++)
// 				add(OtherList[i]);
// 		}
// 		else //Don't re-allocate if I don't have to
// 		{
// 			for (size_t i = 0; i < OtherList.size(); i++)
// 			{
// 				insert(i, OtherList[i]);
// 			}
// 		}

// 		return *this;
// 	}

// 	void erase(const size_t idx)
// 	{
// 		auto l_front = m_List.begin();
// 		auto nx = std::next(l_front, idx);
// 		m_List.erase(nx);
// 	}

// 	void clear()
// 	{
// 		m_List.clear();
// 	}

// 	size_t size() const
// 	{
// 		return m_List.size();
// 	}

// 	itemtype_y *FirstItem()
// 	{
// 		return m_List.front();
// 	}

// 	void reserve(size_t Items) 
// 	{
// 		m_List.assign(Items, itemtype_y());
// 	}

// 	void reserve_once(size_t ReserveItems, size_t Items) 
// 	{
// 		reserve(ReserveItems);
// 	}

// 	//compat functions
// 	void clearitems() {  }
//  	void reserve() {  }
// 	void unalloc() {  }
// };

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
		itemtype_y *pNewItems = msnew itemtype_y[m_ItemsAllocated];
		for (size_t i = 0; i < m_Items; i++)
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
			for (size_t i = 0; i < OtherList.size(); i++)
				add(OtherList[i]);
		}
		else //Don't re-allocate if I don't have to
			for (size_t i = 0; i < OtherList.size(); i++)
				m_First[i] = OtherList[i];

		return *this;
	}

	itemtype_y *FirstItem()
	{
		return m_First;
	}
};

typedef mslist<msstring> msstringlist;
typedef std::map<msstring,msstring> msstringstringhash;