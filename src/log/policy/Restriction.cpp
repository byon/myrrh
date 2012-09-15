// Copyright 2007 Marko Raatikainen.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
 * This file contains implementation of class myrrh::log::policy::Restriction
 *
 * $Id: Restriction.cpp 335 2007-09-09 16:11:55Z byon $
 */

#include "myrrh/log/policy/Restriction.hpp"
#include "myrrh/log/policy/Opener.hpp"

namespace myrrh
{

namespace log
{

namespace policy
{

// Restriction class implementations

Restriction::~Restriction( )
{
}

// SizeRestriction class implementations

SizeRestriction::SizeRestriction(std::size_t maxSize) :
    MAX_SIZE_(maxSize)
{
}

bool SizeRestriction::IsRestricted(const File &file, std::size_t toWrite) const
{
    return (file.WrittenSize( ) + toWrite > MAX_SIZE_);
}

}

}

}
