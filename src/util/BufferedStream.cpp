// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains declaration and implementation of class
 * myrrh::log::BufferedStream
 *
 * $Id: BufferedStream.hpp 365 2007-09-20 18:14:42Z byon $
 */

#include "myrrh/util/BufferedStream.hpp"

namespace myrrh
{
namespace util
{

std::streambuf::int_type
BufferedStream::overflow(std::streambuf::int_type character)
{
    if (EOF != character)
    {
        text_ += static_cast<char>(character);
    }

    return character;
}

std::streamsize BufferedStream::xsputn(const char *text, std::streamsize length)
{
    text_.append(text, length);

    return length;
}

int BufferedStream::sync( )
{
    if (!text_.length( ))
    {
        // There's nothing to do
        return 0;
    }

    if (!SyncImpl( ))
    {
        // Success, clear our buffer
        text_.clear( );
        return 0;
    }

    return -1;
}

const std::string &BufferedStream::GetBuffer( ) const
{
    return text_;
}

}
}
