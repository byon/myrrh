// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains declarations of file names that can be used for unit
 * testing.
 *
 * $Id: Files.hpp 286 2007-03-18 15:04:53Z Byon $
 */

#ifndef MYRRH_DATA_TEST_FILES_HPP_INCLUDED
#define MYRRH_DATA_TEST_FILES_HPP_INCLUDED

#include <string>
#include <vector>

namespace myrrh
{

namespace data
{

namespace test
{

class Files
{
public:

    static const std::string ROOT;
    static const std::string NOT_EXISTING;
    static const std::string EMPTY;
    static const std::string ONE_CHAR;
    static const std::string ONE_LINE;
    static const std::string ONE_LONG_LINE;
    static const std::string SEVERAL_LINES;
    static const std::string SEVERAL_LINES_NOT_EQUAL_LENGTH;
    static const std::string LARGE_FILE;

    /**
     * @note First these were planned to be insided a namespace, but apparently
     *       you cannot have namespaces inside classes.
     */
    struct Index
    {
        /**
         * The enumeration allows to loop through the file names with numeric
         * ids. The only need for this comes when trying to pass the
         * information to a template construct. That needs to be compile-time
         * and strings cannot be non-type parameters.
         */
        enum
        {
            NOT_EXISTING = 0,
            EMPTY,
            ONE_CHAR,
            ONE_LINE,
            ONE_LONG_LINE,
            SEVERAL_LINES,
            SEVERAL_LINES_NOT_EQUAL_LENGTH,
            LARGE_FILE,
        };
    };

    /**
     * Returns all of the file names stored in class. This is usable, if one
     * wants to loop all of the files through some algorithm.
     */
    static std::vector<std::string> GetAll( );

};

}

}

}

#endif
