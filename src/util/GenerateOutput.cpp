// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * Contains implementation of myrrh::util::GenerateOutput function and
 * myrrh::util::WriteFailed exception class.
 *
 * $Id: GenerateOutput.cpp 354 2007-09-17 17:39:58Z byon $
 */

#include "myrrh/util/GenerateOutput.hpp"

#include <cassert>
#include <sstream>

namespace myrrh
{
namespace util
{

namespace
{

std::string GetBaseString(std::istream &input,
                          const std::streamsize outputSize);

}

void GenerateOutput(std::istream &input, std::ostream &output,
                    const std::streamsize outputSize)
{
    if (outputSize <= 0)
    {
        return;
    }

    const std::string BASE(GetBaseString(input, outputSize));
    const std::streamsize BASE_SIZE =
        static_cast<std::streamsize>(BASE.size( ));
    if (BASE_SIZE <= 0)
    {
        return;
    }

    const std::streamsize LAST_COMPLETE_POINT = outputSize - BASE_SIZE;

    const std::streampos ORIG_POS(output.tellp( ));

    while ((output.tellp( ) - ORIG_POS) <= LAST_COMPLETE_POINT)
    {
        output << BASE;

        if (!output.good( ))
        {
            throw WriteFailed("Failed to write all output");
        }
    }

    const std::streamsize INCOMPLETE_SIZE = outputSize % BASE_SIZE;
    output << BASE.substr(0, INCOMPLETE_SIZE);

    output.flush( );
}

namespace
{

std::string GetBaseString(std::istream &input,
                          const std::streamsize outputSize)
{
    std::ostringstream stream;
    stream << input.rdbuf( );

    return stream.str( ).substr(0, outputSize);
}

}

WriteFailed::WriteFailed(const std::string &what) :
    std::runtime_error(what)
{
}

}

}
