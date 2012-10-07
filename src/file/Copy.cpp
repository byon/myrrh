// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains implementation of class myrrh::file::Copy.
 *
 * $Id: Copy.cpp 356 2007-09-18 19:55:21Z byon $
 */

#include "myrrh/file/Copy.hpp"
#include "myrrh/file/PositionScanner.hpp"

#include "boost/scoped_array.hpp"

#include <fstream>

namespace myrrh
{
namespace file
{

namespace
{

/**
 * Checks the given boolean condition. If true, nothing is done. Otherwise an
 * exception of template type Exception is thrown.
 * @param toCheck The condition to be checked
 * @param errorString Tells details of the error given to possible exception
 * @throws type defined by Exception, if toCheck == false
 */
template <typename Exception>
void Check(bool toCheck, const std::string &errorString);

/**
 * Returns size of the given file stream
 */
std::streampos GetFileSize(std::ifstream &stream);

}

Copy::Copy(PositionScannerPtr startScanner, PositionScannerPtr endScanner) :
    startScanner_(startScanner),
    endScanner_(endScanner)
{
}

// Too big method
void Copy::operator( )(std::ifstream &input, std::ofstream &output) const
{
    // if input stream is not open scanners will throw
    const std::streampos START(startScanner_->Scan(input));
    const std::streampos END(endScanner_->Scan(input));

    Check<OutOfBounds>(END >= START, "start point after end");

    assert(START >= 0);
    assert(END >= 0);
    assert(GetFileSize(input) >= END);

    Check<StreamNotOpen>(output.is_open( ), "output stream not open");

    const std::streamsize SIZE = END - START;
    assert(SIZE >= 0);

    if (!SIZE)
    {
        return;
    }

    // What if the file is large? Would be better to copy in smaller fragments
    boost::scoped_array<char> buffer(new char[SIZE + 1]);

    input.seekg(START);
    input.read(buffer.get( ), SIZE);
    buffer[SIZE] = '\0';

    output << buffer.get( );

    output.flush( );
}

Copy::StreamNotOpen::StreamNotOpen(const std::string &what) :
    runtime_error(what)
{
}

Copy::OutOfBounds::OutOfBounds(const std::string &what) :
    runtime_error(what)
{
}

namespace
{

template <typename Exception>
inline void Check(bool toCheck, const std::string &errorString)
{
    if (!toCheck)
    {
        throw Exception("Copy failed: " + errorString);
    }
}

std::streampos GetFileSize(std::ifstream &stream)
{
    const std::streampos ORIG_POS(stream.tellg( ));
    stream.seekg(0, std::ios::end);
    const std::streampos RESULT(stream.tellg( ));
    stream.seekg(ORIG_POS);
    return RESULT;
}

}

}
}
