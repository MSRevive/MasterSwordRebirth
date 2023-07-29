#ifndef ANGELSCRIPT_UTIL_STRINGUTILS_H
#define ANGELSCRIPT_UTIL_STRINGUTILS_H

#include <algorithm>
#include <cctype>
#include <cstring>
#include <functional>
#include <string>

//Implemented in the as namespace to prevent collisions with other implementations.
namespace as
{
/**
*	Taken from MSVC string hash.
*/
inline size_t StringHash( const char* const pszString )
{
#if defined( _WIN64 ) || ( defined( __GNUC__ ) && ( __x86_64__ || __ppc64__ ) )
	static_assert( sizeof( size_t ) == 8, "This code is for 64-bit size_t." );
	const size_t _FNV_offset_basis = 14695981039346656037ULL;
	const size_t _FNV_prime = 1099511628211ULL;

#else /* defined(_WIN64) */
	static_assert( sizeof( size_t ) == 4, "This code is for 32-bit size_t." );
	const size_t _FNV_offset_basis = 2166136261U;
	const size_t _FNV_prime = 16777619U;
#endif /* defined(_WIN64) */

	const size_t _Count = strlen( pszString );

	size_t _Val = _FNV_offset_basis;
	for( size_t _Next = 0; _Next < _Count; ++_Next )
	{	// fold in another byte
		_Val ^= ( size_t ) pszString[ _Next ];
		_Val *= _FNV_prime;
	}
	return ( _Val );
}

template<typename STR>
struct Hash_C_String final : public std::unary_function<STR*, size_t>
{
	std::size_t operator()( STR pszStr ) const
	{
		return StringHash( pszStr );
	}
};

template<typename STR, int( *COMPARE )( STR lhs, STR rhs ) = strcmp>
struct EqualTo_C_String final
{
	bool operator()( STR lhs, STR rhs ) const
	{
		return COMPARE( lhs, rhs ) == 0;
	}
};

/*
*	This code is based on the following Stack Overflow answer: http://stackoverflow.com/a/217605
*/
// trim from start
inline std::string& LTrim( std::string& s )
{
	s.erase( s.begin(), std::find_if( s.begin(), s.end(),
									  std::not1( std::ptr_fun<int, int>( std::isspace ) ) ) );
	return s;
}

// trim from end
inline std::string& RTrim( std::string& s )
{
	s.erase( std::find_if( s.rbegin(), s.rend(),
						   std::not1( std::ptr_fun<int, int>( std::isspace ) ) ).base(), s.end() );
	return s;
}

// trim from both ends
inline std::string& Trim( std::string& s )
{
	return LTrim( RTrim( s ) );
}

/*
*	End based on code.
*/

/**
*	Checks if a printf operation was successful
*/
inline bool PrintfSuccess( const int iRet, const size_t uiBufferSize )
{
	return iRet >= 0 && static_cast<size_t>( iRet ) < uiBufferSize;
}
}

#endif //ANGELSCRIPT_UTIL_STRINGUTILS_H