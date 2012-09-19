// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * File: myrrh/utility/RandomString.hpp
 * Author: Marko Raatikainen
 *
 * This class contains helper classes for generating random strings for test
 * purposes.
 *
 * $Id: RandomString.hpp 286 2007-03-18 15:04:53Z Byon $
 */

#ifndef MYRRH_UTILITY_RANDOMSTRING_H_INCLUDED
#define MYRRH_UTILITY_RANDOMSTRING_H_INCLUDED

// Could hide the implementation into cpp
#include <algorithm>
#include <ctime>
#include <string>

namespace myrrh
{

namespace util
{

// Used for unit testing, but unit tests should be deterministic. Scrap.

/**
 * Returns a random character that can be anything from from ranges A-Z, a-z
 * or any of the following characters: '[', ']', '\', '^', '_', '`'.
 * @note Assumes that srand( ) has been called before first call
 * @return A random character
 */
char RandChar( );

/**
 * Replaces the given string with a random string of same size
 * @note Assumes that srand( ) has been called before first call
 * @param orig The original string
 */
void ReplaceWithRandom(std::string &orig);

/**
 * Creates a string with random content
 * @note Assumes that srand( ) has been called before first call
 * @param size The size of the created string
 * @return The newly created string
 */
std::string GetRandomString(std::string::size_type size);

// Inline implementations

inline char RandChar( )
{
    static const char MAX = 'z' - 'A';

    return static_cast<char>(rand( ) % MAX + 'A');
}

inline void ReplaceWithRandom(std::string &orig)
{
    std::generate(orig.begin( ), orig.end( ), RandChar);
}

inline std::string GetRandomString(std::string::size_type size)
{
    std::string tmp(size, ' ');
    ReplaceWithRandom(tmp);
    return tmp;
}

}

}

#endif
