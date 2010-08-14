// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains implementations of file names that can be used for unit
 * testing.
 *
 * $Id: Files.cpp 286 2007-03-18 15:04:53Z Byon $
 */

#include "myrrh/data/test/Files.hpp"

namespace myrrh
{

namespace data
{

namespace test
{

const std::string Files::ROOT("/data/test/");
const std::string Files::NOT_EXISTING(ROOT + "NotExisting.txt");
const std::string Files::EMPTY(ROOT + "Empty.txt");
const std::string Files::ONE_CHAR(ROOT + "OneChar.txt");
const std::string Files::ONE_LINE(ROOT + "OneLine.txt");
const std::string Files::ONE_LONG_LINE(ROOT + "OneLongLine.txt");
const std::string Files::SEVERAL_LINES(ROOT + "SeveralLines.txt");
const std::string Files::SEVERAL_LINES_NOT_EQUAL_LENGTH(
    ROOT + "SeveralLinesNotEqual.txt");
const std::string Files::LARGE_FILE(ROOT + "LargeFile.txt");

std::vector<std::string> Files::GetAll( )
{
    std::vector<std::string> result;
    result.push_back(NOT_EXISTING);
    result.push_back(EMPTY);
    result.push_back(ONE_CHAR);
    result.push_back(ONE_LINE);
    result.push_back(ONE_LONG_LINE);
    result.push_back(SEVERAL_LINES);
    result.push_back(SEVERAL_LINES_NOT_EQUAL_LENGTH);
    result.push_back(LARGE_FILE);

    return result;
}

}

}

}
