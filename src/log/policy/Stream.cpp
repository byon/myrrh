// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file includes implementation of classes myrrh::log::policy::Buffer and
 * myrrh::log::policy::Stream
 *
 * $Id: Stream.cpp 369 2007-09-20 20:08:51Z byon $
 */

#include "myrrh/log/policy/Stream.hpp"
#include "myrrh/log/policy/Policy.hpp"

namespace myrrh
{

namespace log
{

namespace policy
{

// Buffer class implementations

Buffer::Buffer(PolicyPtr policy) :
    policy_(policy)
{
}

int Buffer::SyncImpl( )
{
    const std::string& BUFFER = GetBuffer( );
    if (BUFFER.size( ) == static_cast<std::size_t>(policy_->Write(BUFFER)))
    {
        return 0;
    }

    return -1;
}

// Stream class implementations

Stream::Stream(PolicyPtr policy) :
    std::ostream(0),
    buffer_(policy)
{
    rdbuf(&buffer_);
}

}
}
}
