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
#include "boost/date_time/posix_time/posix_time.hpp"
#include <ostream>

namespace myrrh
{

namespace log
{

// Class implementations

void TimestampHeader::Write(std::ostream &stream, char id)
{
    /// @todo Most likely too inefficient. Consider changing later.
    auto facet = new boost::posix_time::time_facet("%Y.%m.%d %H:%M:%s");
    stream.imbue(std::locale(stream.getloc(), facet));
    stream << boost::posix_time::microsec_clock::local_time( ) << " " << id
           << " ";
}

}

}
