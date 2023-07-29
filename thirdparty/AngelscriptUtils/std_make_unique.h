#ifndef STD_MAKE_UNIQUE_H
#define STD_MAKE_UNIQUE_H

// Shepard - This header file fixes missing "std::make_unique" for C++11
// compilers, taken from this StackOverflow post:
// https://stackoverflow.com/questions/17902405/how-to-implement-make-unique-function-in-c11

#ifndef WIN32
#if (__cplusplus >= 201103L && __cplusplus < 201402L)
namespace std
{
	template<typename T, typename... Args>
	unique_ptr<T> make_unique(Args&&... args)
	{
		return unique_ptr<T>(new T(forward<Args>(args)...));
	}
}
#endif // (__cplusplus >= 201103L && __cplusplus < 201402L)
#endif // WIN32

#endif // STD_MAKE_UNIQUE_H
