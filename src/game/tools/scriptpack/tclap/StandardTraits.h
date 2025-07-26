// -*- Mode: c++; c-basic-offset: 4; tab-width: 4; -*-

/******************************************************************************
 *
 *  file:  StandardTraits.h
 *
 *  Copyright (c) 2007, Daniel Aarno, Michael E. Smoot .
 *  Copyright (c) 2017, Google LLC
 *  All rights reserved.
 *
 *  See the file COPYING in the top directory of this distribution for
 *  more information.
 *
 *  THE SOFTWARE IS PROVIDED _AS IS_, WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

// This is an internal tclap file, you should probably not have to
// include this directly

#ifndef TCLAP_STANDARD_TRAITS_H
#define TCLAP_STANDARD_TRAITS_H

#include <string>
#include <cstring>  // For strdup
#include <cstdlib>  // For _strdup on MSVC

// If Microsoft has already typedef'd wchar_t as an unsigned
// short, then compiles will break because it's as if we're
// creating ArgTraits twice for unsigned short. Thus...
#ifdef _MSC_VER
#ifndef _NATIVE_WCHAR_T_DEFINED
#define TCLAP_DONT_DECLARE_WCHAR_T_ARGTRAITS
#endif
#endif

namespace TCLAP {

// Integer types (signed, unsigned and bool) and floating point types all
// have value-like semantics.

// Strings have string like argument traits.
template <>
struct ArgTraits<std::string> {
    typedef StringLike ValueCategory;
};

// const char* also has string like argument traits.
template <>
struct ArgTraits<const char*> {
    typedef StringLike ValueCategory;
};

template <typename T>
void SetString(T &dst, const std::string &src) {
    dst = src;
}

// Specialization for const char* - uses strdup to allocate persistent memory
template <>
void SetString(const char* &dst, const std::string &src) {
    // Note: This allocates memory that should be freed with free()
    // In practice, for command line argument parsing, this memory
    // persists for the lifetime of the program, so it's usually ok
#ifdef _MSC_VER
    dst = _strdup(src.c_str());
#else
    dst = strdup(src.c_str());
#endif
}

}  // namespace TCLAP

#endif  // TCLAP_STANDARD_TRAITS_H
