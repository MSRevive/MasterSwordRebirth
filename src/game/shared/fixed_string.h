// Taken from https://github.com/FreeSlave/halflife-featureful, under the HL1 SDK license.
#pragma once

#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <type_traits>
#include "string_utils.h"

template<size_t N>
struct fixed_string
{
	typedef char value_type;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef char& reference;
	typedef const char& const_reference;
	typedef char* pointer;
	typedef const char* const_pointer;
	typedef const char* iterator;
	typedef const char* const_iterator;

	static constexpr size_t npos = (size_t)-1;

	fixed_string() {
		init();
	}
	fixed_string(const char* str) {
		setString(str);
	}
	fixed_string(const char* str, size_t length) {
		init();
		append(str, length);
	}
	template<size_t K>
	fixed_string(const fixed_string<K>& o) {
		static_assert(K <= N, "Can't convert string of bigger size to the string of smaller size");
		setString(o.c_str());
	}
	fixed_string<N>& operator=(const char* str) {
		setString(str);
		return *this;
	}
	template<size_t K>
	fixed_string<N>& operator=(const fixed_string<K>& o) {
		static_assert(K <= N, "Can't convert string of bigger size to the string of smaller size");
		setString(o.c_str());
		return *this;
	}
	template<typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
	fixed_string<N>& operator=(T a) {
		init();
		appendInt((int)a);
		return *this;
	}
	fixed_string<N>& operator+=(const char* str) {
		append(str);
		return *this;
	}
	template<size_t K>
	fixed_string<N>& operator+=(const fixed_string<K>& o) {
		append(o.c_str(), o.size());
		return *this;
	}
	template<typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
	fixed_string<N>& operator+=(T a) {
		appendInt((int)a);
		return *this;
	}
	fixed_string<N> operator+(const char* str) const {
		return fixed_string<N>(*this) += str;
	}
	template<size_t K>
	fixed_string<N> operator+(const fixed_string<K>& o) const {
		return fixed_string<N>(*this) += o;
	}
	template<typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
	fixed_string<N> operator+(T a) const {
		return fixed_string<N>(*this) += a;
	}
	bool operator==(const char* str) const {
		return strcmp(_a, str) == 0;
	}
	bool operator!=(const char* str) const {
		return strcmp(_a, str) != 0;
	}
	bool operator<=(const char* str) const {
		return strcmp(_a, str) <= 0;
	}
	bool operator<(const char* str) const {
		return strcmp(_a, str) < 0;
	}
	bool operator>=(const char* str) const {
		return strcmp(_a, str) >= 0;
	}
	bool operator>(const char* str) const {
		return strcmp(_a, str) > 0;
	}
	template<size_t K>
	bool operator==(const fixed_string<K>& s) const {
		return strcmp(c_str(), s.c_str()) == 0;
	}
	template<size_t K>
	bool operator!=(const fixed_string<K>& s) const {
		return strcmp(c_str(), s.c_str()) != 0;
	}
	template<size_t K>
	bool operator<=(const fixed_string<K>& s) const {
		return strcmp(c_str(), s.c_str()) <= 0;
	}
	template<size_t K>
	bool operator<(const fixed_string<K>& s) const {
		return strcmp(c_str(), s.c_str()) < 0;
	}
	template<size_t K>
	bool operator>=(const fixed_string<K>& s) const {
		return strcmp(c_str(), s.c_str()) >= 0;
	}
	template<size_t K>
	bool operator>(const fixed_string<K>& s) const {
		return strcmp(c_str(), s.c_str()) > 0;
	}
	char operator[](size_t i) const {
		return _a[i];
	}
	explicit operator bool() const {
		return _size != 0;
	}
	size_t size() const {
		return _size;
	}
	size_t len() const {
		return _size;
	}
	bool empty() const {
		return _size == 0;
	}
	void clear() {
		init();
	}
	void append(const char* str) {
		if (str != nullptr)
			append(str, strlen(str));
	}
	void append(const char* str, size_t length) {
		if (str == nullptr)
			return;
		const size_t room = N - 1 - _size;
		size_t count = 0;
		while (count < length && count < room && str[count] != '\0')
			count++;
		memcpy(_a + _size, str, count);
		_size += count;
		_a[_size] = '\0';
	}
	size_t find(const char* str, size_t start = 0) const {
		if (str == nullptr || start > _size)
			return npos;
		const char* found = strstr(_a + start, str);
		return found ? (size_t)(found - (_a + start)) : npos;
	}
	const char* find_str(const char* str, size_t start = 0) const {
		start = clampPos(start);
		const size_t pos = find(str, start);
		return _a + start + (pos != npos ? pos : 0);
	}
	size_t findchar(const char* chars, size_t start = 0) const {
		if (chars == nullptr)
			return npos;
		for (size_t i = start; i < _size; i++) {
			if (strchr(chars, _a[i]))
				return i - start;
		}
		return npos;
	}
	const char* findchar_str(const char* chars, size_t start = 0) const {
		start = clampPos(start);
		const size_t pos = findchar(chars, start);
		return _a + start + (pos != npos ? pos : 0);
	}
	bool contains(const char* str) const {
		return find(str) != npos;
	}
	bool starts_with(const char* str) const {
		return str != nullptr && strncmp(_a, str, strlen(str)) == 0;
	}
	bool ends_with(const char* str) const {
		if (str == nullptr)
			return false;
		const size_t length = strlen(str);
		return length <= _size && memcmp(_a + _size - length, str, length) == 0;
	}
	fixed_string<N> substr(size_t start, size_t length) const {
		return fixed_string<N>(_a + clampPos(start), length);
	}
	fixed_string<N> substr(size_t start) const {
		return fixed_string<N>(_a + clampPos(start));
	}
	fixed_string<N> thru_substr(const char* str, size_t start = 0) const {
		const size_t pos = find(str, start);
		return pos != npos ? substr(start, pos) : substr(start);
	}
	fixed_string<N> thru_char(const char* chars, size_t start = 0) const {
		const size_t pos = findchar(chars, start);
		return pos != npos ? substr(start, pos) : substr(start);
	}
	fixed_string<N> skip(const char* chars) const {
		size_t i = 0;
		while (chars != nullptr && i < _size && strchr(chars, _a[i]))
			i++;
		return substr(i);
	}
	fixed_string<N> tolower() const {
		fixed_string<N> ret(*this);
		for (size_t i = 0; i < ret._size; i++)
			ret._a[i] = (char)::tolower((unsigned char)ret._a[i]);
		return ret;
	}
	iterator begin() const {
		return cbegin();
	}
	iterator end() const {
		return cend();
	}
	const_iterator cbegin() const {
		return _a;
	}
	const_iterator cend() const {
		return _a + _size;
	}
	const char* c_str() const {
		return _a;
	}
private:
	void init()
	{
		_size = 0;
		_a[0] = '\0';
	}
	void setString(const char* str)
	{
		if (str == nullptr)
		{
			init();
		}
		else
		{
			strncpyEnsureTermination(_a, str);
			_size = strlen(_a);
		}
	}
	void appendInt(int a)
	{
		char buf[16];
		snprintf(buf, sizeof(buf), "%i", a);
		append(buf);
	}
	size_t clampPos(size_t pos) const
	{
		return pos < _size ? pos : _size;
	}
	char _a[N];
	size_t _size;
};