// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * The file contains the non-inline implementation of class TimestampHeader.
 *
 * $Id: Header.cpp 286 2007-03-18 15:04:53Z Byon $
 */

#include "myrrh/log/Header.hpp"
#include "myrrh/util/Time.hpp"
#include <memory>
#include <ostream>
#include <cstdio>

#pragma warning(disable : 4996)

namespace myrrh
{

namespace log
{

// Class implementations

void TimestampHeader::Write(std::ostream &stream, char id)
{
    // Note that calling of TimeStampAsCString is not thread safe. However this
    // function should not be called without lock in Verbosity's constructor.
    static char header[64] = {0};
    sprintf(header, "%s %c ", myrrh::util::TimeStampAsCString( ), id);

    stream << header;
}

}

}
